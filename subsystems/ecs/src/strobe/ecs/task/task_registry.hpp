#pragma once

#include "strobe/core/containers/vector.hpp"
#include "strobe/core/memory/AllocatorTraits.hpp"
#include "strobe/core/memory/monotonic_resource.hpp"
#include "strobe/ecs/object/object_function.hpp"
#include "strobe/ecs/task/task_traits.hpp"
#include <algorithm>
#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <type_traits>

namespace strobe::ecs {

struct Universe;

class TaskRegistry {
public:
  using allocator = strobe::ecs::allocator_ref;
  using allocator_traits = AllocatorTraits<allocator>;

  explicit TaskRegistry(Universe *universe, const allocator &alloc) noexcept
      : m_universe(universe), m_chunkPool(alloc), m_headers(alloc),
        m_recordAlloc(alloc) {
    m_first = m_chunkPool.acquire();
    m_last.store(m_first, std::memory_order_relaxed);
  }

  TaskRegistry(const TaskRegistry &) = delete;
  TaskRegistry &operator=(const TaskRegistry &) = delete;
  TaskRegistry(TaskRegistry &&) = delete;
  TaskRegistry &operator=(TaskRegistry &&) = delete;

  ~TaskRegistry() noexcept {
    // scheduler must have been joined.
    for (task_header *header : m_headers) {
      if (header != nullptr) {
        std::destroy_at(header);
      }
    }
    chunk *first = m_first;
    chunk *last = m_last.load(std::memory_order_relaxed);
    assert(first != nullptr);
    assert(first == last);
    assert(first->count.load(std::memory_order_relaxed) == 0);
    m_chunkPool.reclaim(first, last);
    m_first = nullptr;
    m_last.store(nullptr, std::memory_order_relaxed);
  }

  template <task_fn Fn> void cmd_submit() noexcept {
    using traits = task_traits<Fn>;
    using task_type = typename task_traits<Fn>::task_type;
    constexpr task_curry fn = [](TaskRegistry *treg) noexcept {
      task_header *header = treg->require_task_header<task_type>();
      // TODO: check somehow that all traits::lifetime_arguments
      // are constructed otherwise drop and and warn.
      header->submit(treg->scheduler_ptr());
    };
    chunk *current = m_last.load(std::memory_order_acquire);
    const std::uint32_t index =
        current->count.fetch_add(1, std::memory_order_relaxed);
    if (index < chunk_size) {
      current->tasks[index] = fn;
      return;
    }
    chunk *next = m_chunkPool.acquire();
    next->tasks[0] = fn;
    next->count.store(1, std::memory_order_relaxed);
    chunk *previous = m_last.exchange(next, std::memory_order_acq_rel);
    previous->next.store(next, std::memory_order_release);
  }

  void drain_cmds() noexcept {
    Scheduler *scheduler = scheduler_ptr();
    const location loc = tloc();
    chunk *retired_first = nullptr;
    chunk *retired_last = nullptr;

    while (true) {
      scheduler->fence(op_scope(acq_rel(loc)));
      if (retired_first != nullptr) {
        m_chunkPool.reclaim(retired_first, retired_last);
        retired_first = nullptr;
        retired_last = nullptr;
      }
      chunk *first = m_first;
      chunk *last = m_last.load(std::memory_order_relaxed);
      if (first == last && first->count.load(std::memory_order_relaxed) == 0) {
        break;
      }
      chunk *replacement = m_chunkPool.acquire();
      m_first = replacement;
      m_last.store(replacement, std::memory_order_release);
      chunk *current = first;

      while (true) {
        std::uint32_t count = current->count.load(std::memory_order_relaxed);
        if (count > chunk_size) {
          count = static_cast<std::uint32_t>(chunk_size);
        }
        for (std::uint32_t i = 0; i < count; ++i) {
          current->tasks[i](this);
        }
        if (current == last) {
          break;
        }
        current = current->next.load(std::memory_order_relaxed);
        assert(current != nullptr);
      }
      retired_first = first;
      retired_last = last;
    }
  }

private:
  using task_curry = void (*)(TaskRegistry *) noexcept;

  using task_id = uint32_t;
  static constexpr task_id invalid_task_id =
      std::numeric_limits<task_id>::max();

  static constexpr size_t chunk_size = 128;

  struct chunk {
    std::atomic<uint32_t> count = 0;
    std::atomic<chunk *> next = nullptr;
    task_curry tasks[chunk_size];
  };

  struct chunk_pool {
  public:
    explicit chunk_pool(const allocator &alloc) noexcept : m_upstream(alloc) {}
    chunk_pool(const chunk_pool &) = delete;
    chunk_pool &operator=(const chunk_pool &) = delete;
    chunk_pool(chunk_pool &&) = delete;
    chunk_pool &operator=(chunk_pool &&) = delete;

    ~chunk_pool() noexcept {
      chunk *current = m_free.exchange(nullptr, std::memory_order_acquire);
      while (current != nullptr) {
        chunk *next = current->next.load(std::memory_order_relaxed);

        std::destroy_at(current);

        allocator_traits::template deallocate<chunk>(m_upstream, current, 1);

        current = next;
      }
    }

    // thread-safe
    // Time: O(1)
    chunk *acquire() noexcept {
      chunk *c = pop_free();
      if (c == nullptr) {
        c = allocator_traits::template allocate<chunk>(m_upstream, 1);
        std::construct_at(c);
      }
      c->count.store(0, std::memory_order_relaxed);
      c->next.store(nullptr, std::memory_order_relaxed);
      return c;
    }

    // exclusive
    // Time: O(1)
    void reclaim(chunk *first, chunk *last) noexcept {
      if (first == nullptr) {
        assert(last == nullptr);
        return;
      }
      assert(last != nullptr);
      chunk *old = m_free.load(std::memory_order_relaxed);
      last->next.store(old, std::memory_order_relaxed);
      m_free.store(first, std::memory_order_release);
    }

  private:
    chunk *pop_free() noexcept {
      chunk *head = m_free.load(std::memory_order_acquire);
      while (head != nullptr) {
        chunk *next = head->next.load(std::memory_order_acquire);
        if (m_free.compare_exchange_weak(head, next, std::memory_order_acquire,
                                         std::memory_order_acquire)) {
          return head;
        }
      }
      return nullptr;
    }

  private:
    allocator m_upstream;
    std::atomic<chunk *> m_free = nullptr;
  };

  struct task_header {
    virtual void submit(Scheduler *scheduler) noexcept = 0;
    virtual ~task_header() = default;
  };

  template <task_fn Fn> struct task_record final : task_header {
    task_record(Universe *universe) noexcept
        : task_header(), m_func(universe) {}

    using task_type = typename task_traits<Fn>::task_type;
    static constexpr auto call_operator = &task_type::operator();
    using function = stateless_object_function<call_operator>;
    void submit(Scheduler *scheduler) noexcept override {
      m_func.submit(scheduler);
    }
    function m_func;
  };

private:
  static uint32_t next_task_type_id() noexcept {
    static std::atomic<uint32_t> id_acc = 0;
    return id_acc.fetch_add(1, std::memory_order_relaxed);
  }

  // thread-safe!
  template <task_fn Fn>
    requires(!std::is_reference_v<Fn>)
  static task_id task_type_id() noexcept {
    static uint32_t id = next_task_type_id();
    return id;
  }

  // submitting thread-only
  template <task_fn Fn> task_header *require_task_header() noexcept {
    task_id id = task_type_id<Fn>();
    assert(id != invalid_task_id);
    size_t index = static_cast<size_t>(id);
    size_t sz = m_headers.size();
    if (sz <= index) {
      const std::size_t new_size = std::max(index + 1, sz * 3 / 2 + 1);
      m_headers.resize(new_size, nullptr);
    }
    task_header *header = m_headers[id];
    if (header != nullptr) {
      return header;
    }
    using traits = task_traits<Fn>;
    using task_type = typename traits::task_type;
    using record = task_record<task_type>;

    using alloc_traits = AllocatorTraits<decltype(m_recordAlloc)>;
    record *ptr = alloc_traits::template allocate<record>(m_recordAlloc);
    std::construct_at(ptr, m_universe);
    m_headers[id] = static_cast<task_header *>(ptr);
    return static_cast<task_header *>(ptr);
  }

  inline __attribute__((always_inline)) Scheduler *
  scheduler_ptr() const noexcept;

  inline __attribute__((always_inline)) location tloc() const noexcept;

private:
  Universe *m_universe;
  chunk_pool m_chunkPool;

  chunk *m_first = nullptr;
  std::atomic<chunk *> m_last = nullptr;

  Vector<task_header *, allocator> m_headers;
  MonotonicResource<allocator> m_recordAlloc;
};

} // namespace strobe::ecs
