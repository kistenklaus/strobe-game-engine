#pragma once

#include "strobe/core/memory/AllocatorTraits.hpp"
#include "strobe/core/memory/cache_line.hpp"
#include "strobe/ecs/scheduler/allocator.hpp"
#include "strobe/ecs/scheduler/config.hpp"

#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <memory>
#include <new>
#include <type_traits>

namespace strobe::ecs {

class JobScoreboard {
public:
  using allocator = strobe::ecs::scheduler::allocator_ref;
  using allocator_traits = AllocatorTraits<allocator>;
  static constexpr uint32_t MAX_DEPENDENCIES = schedule::SLOT_COUNT - 1;
  static_assert(MAX_DEPENDENCIES != 0);

private:
  struct ConsumerNode {
    ConsumerNode *next;
    JobScoreboard *consumer;
  };

  static ConsumerNode *completed_sentinel() noexcept {
    return reinterpret_cast<ConsumerNode *>(uintptr_t{1});
  }

public:
  class ReadyConsumers {
    friend class JobScoreboard;

  public:
    class iterator {
    public:
      using iterator_category = std::input_iterator_tag;
      using value_type = JobScoreboard *;
      using difference_type = std::ptrdiff_t;
      using pointer = JobScoreboard **;
      using reference = JobScoreboard *;

      iterator() noexcept = default;

      explicit iterator(ConsumerNode *node) noexcept
          : m_current(node), m_next(node != nullptr ? node->next : nullptr) {}

      JobScoreboard *operator*() const noexcept {
        assert(m_current != nullptr);
        assert(m_current->consumer != nullptr);
        return m_current->consumer;
      }

      iterator &operator++() noexcept {
        m_current = m_next;
        m_next = m_current != nullptr ? m_current->next : nullptr;
        return *this;
      }

      iterator operator++(int) noexcept {
        iterator old = *this;
        ++(*this);
        return old;
      }

      friend bool operator==(const iterator &a, const iterator &b) noexcept {
        return a.m_current == b.m_current;
      }

      friend bool operator!=(const iterator &a, const iterator &b) noexcept {
        return !(a == b);
      }

    private:
      ConsumerNode *m_current = nullptr;
      ConsumerNode *m_next = nullptr;
    };

    iterator begin() const noexcept { return iterator(m_head); }
    iterator end() const noexcept { return iterator(nullptr); }

    bool empty() const noexcept { return m_head == nullptr; }

  private:
    explicit ReadyConsumers(ConsumerNode *head) noexcept : m_head(head) {}

    ConsumerNode *m_head = nullptr;
  };

public:
  JobScoreboard(allocator alloc) noexcept : m_top(nullptr), m_nodes(nullptr) {
    static_assert(std::is_trivial_v<ConsumerNode>);

    m_nodes = allocator_traits::template allocate<ConsumerNode>(
        alloc, MAX_DEPENDENCIES);

    assert(m_nodes != nullptr);

    for (uint32_t i = 0; i < MAX_DEPENDENCIES; ++i) {
      std::construct_at(m_nodes + i);
    }

    m_top = m_nodes;
  }

  JobScoreboard(const JobScoreboard &) = delete;
  JobScoreboard &operator=(const JobScoreboard &) = delete;
  JobScoreboard(JobScoreboard &&) = delete;
  JobScoreboard &operator=(JobScoreboard &&) = delete;

  // Owner-only.
  void reset() noexcept {
    m_pending.store(1, std::memory_order_relaxed);         // submit gate
    m_consumers.store(nullptr, std::memory_order_relaxed); // open list
    m_top = m_nodes;
  }

  // Owner-only.
  void addSource(JobScoreboard *source) noexcept {
    assert(source != nullptr);
    assert(m_nodes != nullptr);
    assert(m_top != nullptr);
    assert(m_top < (m_nodes + MAX_DEPENDENCIES));

    ConsumerNode *node = m_top++;
    node->next = nullptr;
    node->consumer = this;

    // Count this source before publishing the consumer node.
    m_pending.fetch_add(1, std::memory_order_relaxed);

    ConsumerNode *head = source->m_consumers.load(std::memory_order_acquire);

    do {
      if (head == completed_sentinel()) {
        // Source completed before registration succeeded.
        node->next = nullptr;
        node->consumer = nullptr;
        notifySourceReady();
        return;
      }

      node->next = head;
    } while (!source->m_consumers.compare_exchange_weak(
        head, node, std::memory_order_release, std::memory_order_acquire));
  }

  // Owner-only.
  //
  // Releases the submit gate.
  // Returns true if this scoreboard is now ready.
  bool commit() noexcept { return notifySourceReady(); }

  // Thread-safe.
  //
  // Marks this scoreboard as complete and returns all consumers that became
  // ready because of this completion.
  ReadyConsumers complete() noexcept {
    ConsumerNode *list =
        m_consumers.exchange(completed_sentinel(), std::memory_order_acq_rel);
    assert(list != completed_sentinel());
    ConsumerNode *readyHead = nullptr;
    while (list != nullptr) {
      ConsumerNode *node = list;
      list = list->next;
      JobScoreboard *consumer = node->consumer;
      assert(consumer !=
             nullptr); // TODO <- this failed once !! AHHH treiber fuckoff
      if (consumer->notifySourceReady()) {
        node->next = readyHead;
        readyHead = node;
      } else {
        node->next = nullptr;
        node->consumer = nullptr;
      }
    }

    return ReadyConsumers(readyHead);
  }

private:
  // Thread-safe.
  //
  // One source has completed.
  // Returns true if this scoreboard became ready.
  bool notifySourceReady() noexcept {
    const uint32_t old = m_pending.fetch_sub(1, std::memory_order_acq_rel);
    assert(old > 0);
    return old == 1;
  }

private:
  alignas(memory::cache_line) std::atomic<uint32_t> m_pending = 0;
  alignas(memory::cache_line) std::atomic<ConsumerNode *> m_consumers = nullptr;

  ConsumerNode *m_top = nullptr;
  ConsumerNode *m_nodes = nullptr;

public: // allocation info
  static constexpr size_t static_allocation_size() {
    return sizeof(ConsumerNode) * MAX_DEPENDENCIES;
  }

  static constexpr size_t static_allocation_alignment() {
    return alignof(ConsumerNode);
  }

  static constexpr size_t dynamic_allocation_size() { return 0; }

  static constexpr size_t dynamic_allocation_alignment() { return 1; }
};

} // namespace strobe::ecs
