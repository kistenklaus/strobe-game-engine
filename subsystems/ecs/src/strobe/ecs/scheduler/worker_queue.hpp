#pragma once

#include "strobe/core/memory/AllocatorTraits.hpp"
#include "strobe/core/memory/cache_line.hpp"
#include "strobe/ecs/scheduler/allocator.hpp"
#include <atomic>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <new>
#include <type_traits>
namespace strobe::ecs {

class WorkerQueue {
public:
  static constexpr size_t ITEM_CHUNK_SIZE = 32;

public: // public types
  using allocator = strobe::ecs::scheduler::allocator_ref;
  using allocator_traits = AllocatorTraits<allocator>;
  using job = void;

  class Pool;

  struct item {
    friend class WorkerQueue;
    friend class Pool;
    // completely opaque handle.
  private:
    job *m_job;
    item *m_next;
    Pool *m_pool;
  };

  static_assert(std::is_trivially_constructible_v<item>);
  static_assert(std::is_trivially_destructible_v<item>);

  class Pool {
  public:
    Pool() noexcept = default;

    Pool(const Pool &) = delete;
    Pool &operator=(const Pool &) = delete;
    Pool(Pool &&) = delete;
    Pool &operator=(Pool &&) = delete;

    // Owner-only.
    item *allocate(allocator alloc) {
      item *head = m_freelist.load(std::memory_order_acquire);
      while (head != nullptr) {
        item *next = head->m_next;
        if (m_freelist.compare_exchange_weak(head, next,
                                             std::memory_order_acquire,
                                             std::memory_order_relaxed)) {
          head->m_next = nullptr;
          head->m_job = nullptr;
          return head;
        }
      }
      item *chunk =
          allocator_traits::template allocate<item>(alloc, ITEM_CHUNK_SIZE);

      assert(chunk != nullptr);
      for (uint32_t i = 0; i < ITEM_CHUNK_SIZE; ++i) {
        chunk[i].m_pool = this;
      }
      item *result = chunk;
      result->m_job = nullptr;
      result->m_next = nullptr;

      static_assert(ITEM_CHUNK_SIZE > 1);
      item *first_free = chunk + 1;
      for (uint32_t i = 1; i + 1 < ITEM_CHUNK_SIZE; ++i) {
        chunk[i].m_next = chunk + i + 1;
      }
      item *last = chunk + ITEM_CHUNK_SIZE - 1;
      head = m_freelist.load(std::memory_order_relaxed);
      do {
        last->m_next = head;
      } while (!m_freelist.compare_exchange_weak(head, first_free,
                                                 std::memory_order_release,
                                                 std::memory_order_relaxed));

      return result;
    }

  private:
    friend class WorkerQueue;

    // Thread-safe.
    void deallocate(item *node) noexcept {
      assert(node != nullptr);
      assert(node->m_pool == this);
      item *head = m_freelist.load(std::memory_order_relaxed);
      do {
        node->m_next = head;
      } while (!m_freelist.compare_exchange_weak(
          head, node, std::memory_order_release, std::memory_order_relaxed));
    }

  private:
    alignas(memory::cache_line)
        std::atomic<item *> m_freelist = nullptr;
  };

private: // <- private types
  struct MPSCInbox {
    // thread-safe
    void push(item *node) noexcept {
      assert(node != nullptr);

      item *head = m_head.load(std::memory_order_relaxed);
      do {
        node->m_next = head;
      } while (!m_head.compare_exchange_weak(
          head, node, std::memory_order_release, std::memory_order_relaxed));
    }

    // thread-safe
    void push_list(item *first, item *last) noexcept {
      assert(first != nullptr);
      assert(last != nullptr);
      assert(last->m_next == nullptr);

      item *head = m_head.load(std::memory_order_relaxed);
      do {
        last->m_next = head;
      } while (!m_head.compare_exchange_weak(
          head, first, std::memory_order_release, std::memory_order_relaxed));
    }

    // owner-only
    item *pop_all() noexcept {
      return m_head.exchange(nullptr, std::memory_order_acquire);
    }

  private:
    alignas(memory::cache_line)
        std::atomic<item *> m_head = nullptr;
  };

  struct SPMCDeque {
    explicit SPMCDeque(allocator alloc, uint32_t capacity) noexcept {
      assert(capacity > 0);
      capacity = std::bit_ceil(capacity);
      assert(capacity >= 2);
      m_capacity = capacity;
      m_mask = capacity - 1;
      m_ringbuffer =
          allocator_traits::template allocate<job *>(alloc, m_capacity);
      assert(m_ringbuffer != nullptr);
      static_assert(std::is_trivially_destructible_v<job *>);
      for (uint32_t i = 0; i < m_capacity; ++i) {
        std::construct_at(m_ringbuffer + i, nullptr);
      }
      m_top.store(0, std::memory_order_relaxed);
      m_bottom.store(0, std::memory_order_relaxed);
    }

    SPMCDeque(const SPMCDeque &) = delete;
    SPMCDeque &operator=(const SPMCDeque &) = delete;

    SPMCDeque(SPMCDeque &&) = delete;
    SPMCDeque &operator=(SPMCDeque &&) = delete;

    // owner-only.
    bool push(job *job) noexcept {
      assert(job != nullptr);
      const uint64_t bottom = m_bottom.load(std::memory_order_relaxed);
      const uint64_t top = m_top.load(std::memory_order_acquire);
      if (bottom - top >= m_capacity) {
        return false;
      }
      m_ringbuffer[bottom & m_mask] = job;
      m_bottom.store(bottom + 1, std::memory_order_release);
      return true;
    }

    // owner-only.
    job *pop() noexcept {
      uint64_t bottom = m_bottom.load(std::memory_order_relaxed);
      uint64_t top = m_top.load(std::memory_order_relaxed);
      if (bottom <= top) {
        return nullptr;
      }
      bottom -= 1;
      m_bottom.store(bottom, std::memory_order_relaxed);
      std::atomic_thread_fence(std::memory_order_seq_cst);
      top = m_top.load(std::memory_order_relaxed);
      const int64_t size =
          static_cast<int64_t>(bottom) - static_cast<int64_t>(top);
      if (size < 0) {
        m_bottom.store(top, std::memory_order_relaxed);
        return nullptr;
      }
      job *result = m_ringbuffer[bottom & m_mask];
      if (size > 0) {
        return result;
      }
      // Last item: owner races with thieves.
      const uint64_t restoredBottom = bottom + 1;
      if (!m_top.compare_exchange_strong(top, top + 1,
                                         std::memory_order_seq_cst,
                                         std::memory_order_relaxed)) {
        m_bottom.store(restoredBottom, std::memory_order_relaxed);
        return nullptr;
      }
      m_bottom.store(restoredBottom, std::memory_order_relaxed);
      return result;
    }

    // thief-safe.
    job *steal() noexcept {
      uint64_t top = m_top.load(std::memory_order_acquire);
      std::atomic_thread_fence(std::memory_order_seq_cst);
      const uint64_t bottom = m_bottom.load(std::memory_order_acquire);
      const int64_t size =
          static_cast<int64_t>(bottom) - static_cast<int64_t>(top);

      if (size <= 0) {
        return nullptr;
      }
      job *job = m_ringbuffer[top & m_mask];
      if (!m_top.compare_exchange_strong(top, top + 1,
                                         std::memory_order_seq_cst,
                                         std::memory_order_relaxed)) {
        return nullptr;
      }
      return job;
    }

    uint32_t capacity() const noexcept { return m_capacity; }

  private:
    uint32_t m_capacity = 0;
    uint32_t m_mask = 0;

    alignas(memory::cache_line)
        std::atomic<uint64_t> m_top{0};

    alignas(memory::cache_line)
        std::atomic<uint64_t> m_bottom{0};

    job **m_ringbuffer = nullptr;
  };

public:
  WorkerQueue(allocator alloc, uint32_t dequeCapacity)
      : m_inbox(), m_pending(nullptr), m_deque(alloc, dequeCapacity) {}

  bool submit_local(job *job) noexcept {
    assert(job != nullptr);
    return m_deque.push(job);
  }

  // owner-only.
  void submit_local(item *node, job *job) noexcept {
    assert(node != nullptr);
    assert(job != nullptr);
    assert(node->m_pool != nullptr);

    node->m_job = job;
    node->m_next = m_pending;
    m_pending = node;
  }

  // thread-safe.
  void submit(item *node, job *job) noexcept {
    assert(node != nullptr);
    assert(job != nullptr);
    assert(node->m_pool != nullptr);
    node->m_job = job;
    node->m_next = nullptr;
    m_inbox.push(node);
  }

  // owner-only
  job *pop() noexcept {
    if (job *job = m_deque.pop()) {
      return job;
    }
    if (m_pending == nullptr) {
      m_pending = m_inbox.pop_all();
    }
    item *node = m_pending;
    while (node != nullptr) {
      item *next = node->m_next;
      job *job = node->m_job;
      assert(job != nullptr);
      if (!m_deque.push(job)) {
        m_pending = node;
        return m_deque.pop();
      }
      node->m_job = nullptr;
      node->m_next = nullptr;
      node->m_pool->deallocate(node);
      node = next;
    }
    m_pending = nullptr;
    return m_deque.pop();
  }

  // thief-safe.
  job *steal() noexcept { return m_deque.steal(); }

private:
  MPSCInbox m_inbox;
  item *m_pending;
  SPMCDeque m_deque;

public: // allocation info.
  static constexpr size_t static_allocation_size(uint32_t dequeCapacity) {
    assert(dequeCapacity > 0);

    const uint32_t capacity = std::bit_ceil(dequeCapacity);
    assert(capacity >= 2);

    return sizeof(job *) * capacity;
  }

  static constexpr size_t static_allocation_alignment() {
    return alignof(job *);
  }

  static constexpr size_t dynamic_allocation_size() {
    return sizeof(item) * ITEM_CHUNK_SIZE;
  }
  static constexpr size_t dynamic_allocation_alignment() {
    return alignof(item *);
  }
};

} // namespace strobe::ecs
