#pragma once

#include <atomic>
#include <cassert>
#include <cstddef>
#include <limits>
#include <memory>
#include <optional>
#include <type_traits>

namespace strobe::sync {

namespace details {
#ifdef __APPLE__
// apple architecture is weird, they have conditional stores
// instead of CAS, and for some magical reason CAS2 is faster here.
static constexpr bool LockFreeFlexibleMSQueueUseCAS2Default = true;
#else
static constexpr bool LockFreeFlexibleMSQueueUseCAS2Default = false;
#endif
} // namespace details

/// Michael & Scott (MPSC) lock-free bounded queue (single consumer).
/// IMPORTANT: No safe deferred reclamation (single consumer only).
/// Based on: https://www.cs.rochester.edu/~scott/papers/1996_PODC_queues.pdf
template <typename T,
          bool UseCAS2 = details::LockFreeFlexibleMSQueueUseCAS2Default>
class LockFreeFlexibleMSQueue {
private:
  using counter_type =
      std::conditional_t<UseCAS2, std::uint64_t, std::uint32_t>;
  using pointer_offset =
      std::conditional_t<UseCAS2, std::uint64_t, std::uint32_t>;
  static constexpr std::size_t NULL_POINTER_OFFSET =
      std::numeric_limits<pointer_offset>::max();

public:
  struct Pointer {
    pointer_offset ptrOffset;
    counter_type count;

    Pointer() : ptrOffset(NULL_POINTER_OFFSET), count(0) {}
    Pointer(pointer_offset offset, counter_type count)
        : ptrOffset(offset), count(count) {}

    friend bool operator==(const Pointer &a, const Pointer &b) {
      return a.ptrOffset == b.ptrOffset && a.count == b.count;
    }

    operator bool() const { return ptrOffset != NULL_POINTER_OFFSET; }
  };
  static_assert(std::atomic<Pointer>::is_always_lock_free);

  struct Node {
    T value;
    std::atomic<Pointer> next;
  };

  union FreelistNode {
    struct {
      Pointer next;
    } free;
    struct {
      Node node;
    } value;

    FreelistNode() : free{} {}
    ~FreelistNode() {}
  };

  LockFreeFlexibleMSQueue(std::size_t flexibleBufferSize, FreelistNode *buffer)
      : m_buffer(buffer) {
    assert((flexibleBufferSize + 1) <
           std::numeric_limits<pointer_offset>::max());

    // Initialize freelist
    for (pointer_offset i = 0; i < flexibleBufferSize - 1; ++i) {
      m_buffer[i].free.next = Pointer(i + 1, 0);
    }
    m_buffer[flexibleBufferSize - 1].free.next =
        Pointer(NULL_POINTER_OFFSET, 0); // End of freelist
    m_freelistPtr.store(Pointer(0, 0), std::memory_order_release);

    // Create dummy node
    Pointer dummyPtr = allocateNode();
    Node *dummyNode = derefNode(dummyPtr);
    dummyNode->next.store(Pointer(), std::memory_order_relaxed);

    m_head.store(dummyPtr, std::memory_order_release);
    m_tail.store(dummyPtr, std::memory_order_release);
  }

  ~LockFreeFlexibleMSQueue() {
    if (!std::is_trivially_destructible_v<T>) {
      while (true) {
        std::optional<T> v(dequeue());
        if (!v.has_value()) {
          break;
        }
      }
    }
  }

  template <typename... Args> bool enqueue(Args &&...args) {
    Pointer newPtr = allocateNode();
    if (!newPtr) {
      return false; // Queue full
    }

    Node *newNode = derefNode(newPtr);
    new (&newNode->value) T(std::forward<Args>(args)...);
    newNode->next.store(Pointer(), std::memory_order_relaxed);

    while (true) {
      Pointer tail = m_tail.load(std::memory_order_acquire);
      Node *tailNode = derefNode(tail);

      Pointer next = tailNode->next.load(std::memory_order_acquire);
      if (tail == m_tail.load(std::memory_order_acquire)) {
        if (!next) {
          if (tailNode->next.compare_exchange_weak(next, newPtr,
                                                   std::memory_order_release,
                                                   std::memory_order_relaxed)) {
            // Try to advance tail
            Pointer desired(tailNode->next.load().ptrOffset, tail.count + 1);
            m_tail.compare_exchange_weak(tail, desired,
                                         std::memory_order_release,
                                         std::memory_order_relaxed);
            return true;
          }
        } else {
          // Tail lagging behind, advance it
          Pointer desired(next.ptrOffset, tail.count + 1);
          m_tail.compare_exchange_weak(tail, desired, std::memory_order_release,
                                       std::memory_order_relaxed);
        }
      }
    }
  }

  std::optional<T> dequeue() {
    while (true) {
      Pointer head = m_head.load(std::memory_order_acquire);
      Pointer tail = m_tail.load(std::memory_order_acquire);
      Node *headNode = derefNode(head);

      Pointer next = headNode->next.load(std::memory_order_acquire);
      if (head == m_head.load(std::memory_order_acquire)) {
        if (!next) {
          return std::nullopt; // Queue empty
        }

        if (head.ptrOffset == tail.ptrOffset) {
          // Tail might be lagging; help advance
          Pointer desired(next.ptrOffset, tail.count + 1);
          m_tail.compare_exchange_weak(tail, desired, std::memory_order_release,
                                       std::memory_order_relaxed);
        }

        Node *nextNode = derefNode(next);
        T value = std::move(nextNode->value);
        std::destroy_at(&nextNode->value);

        Pointer desired(next.ptrOffset, head.count + 1);
        // Since only one consumer: no need for CAS
        m_head.store(desired, std::memory_order_release);

        deallocateNode(head);
        return value;
      }
    }
  }

private:
  Pointer allocateNode() {
    Pointer freelistHead = m_freelistPtr.load(std::memory_order_relaxed);
    while (freelistHead) {
      FreelistNode *node = derefFreelist(freelistHead);
      Pointer next = Pointer(node->free.next.ptrOffset, freelistHead.count + 1);
      if (m_freelistPtr.compare_exchange_weak(freelistHead, next,
                                              std::memory_order_acquire,
                                              std::memory_order_relaxed)) {
        return freelistHead;
      }
    }
    return {}; // Queue full
  }

  void deallocateNode(Pointer ptr) {
    assert(ptr);
    FreelistNode *node = derefFreelist(ptr);
    Pointer currentHead = m_freelistPtr.load(std::memory_order_relaxed);
    do {
      node->free.next = currentHead;
    } while (!m_freelistPtr.compare_exchange_weak(
        currentHead, Pointer(ptr.ptrOffset, ptr.count + 1),
        std::memory_order_release, std::memory_order_relaxed));
  }

  inline FreelistNode *derefFreelist(Pointer p) {
    return &m_buffer[p.ptrOffset];
  }

  inline Node *derefNode(Pointer p) {
    return &m_buffer[p.ptrOffset].value.node;
  }
  std::atomic<Pointer> m_head;
  std::atomic<Pointer> m_freelistPtr;
  std::atomic<Pointer> m_tail;
  FreelistNode *m_buffer; // Flexible array layout (hacky)
};

} // namespace strobe::sync
