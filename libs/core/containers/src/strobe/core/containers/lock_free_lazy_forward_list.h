#pragma once

#include <atomic>

#include "strobe/core/memory/AllocatorTraits.hpp"
#include "strobe/core/memory/ReferenceCounter.hpp"
#include "strobe/core/memory/sync_monotonic_pool_allocator.hpp"
namespace strobe {

// TODO NOT DONE YET FIXIT FIRST!!!
template <typename T, Allocator A>
class LockFreeLazyForwardList {
 private:
  struct Node {
    T value;
    std::atomic<Node*> next;
    memory::ReferenceCounter<std::size_t> epoch;
  };
  using Pool = LockFreeMonotonicPoolResource<sizeof(Node), alignof(Node), A>;
  using PoolTraits = AllocatorTraits<Pool>;

 public:
  using value_type = T;
  using reference = T&;
  using const_reference = const T&;
  using allocator_type = A;

  class iterator {
    friend LockFreeLazyForwardList<T, A>;
    explicit iterator(Node* node) : m_node(node) {}

   public:
    iterator() : m_node(nullptr) {}

    T& operator*() const {
      assert(m_node != nullptr);
      // this is safe because we hold a ref count of node.
      return m_node->value;
    }
    T* operator->() const {
      assert(m_node != nullptr);
      // this is safe because we hold a ref count of node.
      return &m_node->value;
    }

    iterator& operator++() {
      assert(m_node != nullptr);
      assert(m_node != m_node->next);

      Node* next = m_node->next.load(std::memory_order_relaxed);
      while (true) {
        if (next == nullptr) {
          // we hit the end.
          m_node = nullptr;
          return *this;
        }
        if (next->epoch.inc()) {
          // successfully acquire node.
          m_node = next;
          return *this;
        }
        // increment failed => node's value was already destructured.

        // erase next as it is no longer a active node
        // lockfree
        Node* skip = next->next.load(std::memory_order_relaxed);
        if (m_node->next.compare_exchange_weak(next, skip,
                                               std::memory_order_acquire,
                                               std::memory_order_relaxed)) {
          // nothing should be pointing to it because:
          // - the count of next == 0 (inc failed)
          // - the previous node skips this node.
          PoolTraits::template deallocate<Node>(next);
        }
      }

      // NOTE: this probably has be after the increment of next!
      bool died = m_node->epoch.dec();
      if (died) {
        m_node->value.~T();
        // we cannot yet deallocate it because
        // iterators might still point to it
      }

      m_node = m_node->next;
      return *this;
    }
    iterator operator++(int) {
      iterator temp = *this;
      ++(*this);
      return temp;
    }

    bool operator==(const iterator& other) const {
      return m_node == other.m_node;
    }
    bool operator!=(const iterator& other) const {
      return m_node != other.m_node;
    }

   private:
    Node* m_node;
    Pool* m_pool;
  };
  using const_iterator = iterator; // TODO write proper one later.
 
  LockFreeLazyForwardList(const A& alloc) : m_pool(alloc) {}

  // O(1) time.
  template <typename U>
  void push(U&& value) {
    // TODO I feel like something was missing here.

    Node* node = PoolTraits::template allocate<Node>(m_pool);
    new (&node->value) T(std::forward<U>(value));
    node->epoch.reset();
    Node* head = m_head.load(std::memory_order_relaxed);
    do {
      node->next = head;
    } while (!m_head.compare_exchange_weak(
        head, node, std::memory_order_acquire, std::memory_order_relaxed));

  }

  void erase(const_iterator pos) {
    if (pos.m_node->epoch.dec()) {
      pos.m_node->value.~T();
      // deallocation is done lazily.
    }

    // TODO I feel like something was missing here.
  }

  iterator begin() { return iterator(m_head, &m_pool); }
  iterator end() { return iterator(nullptr, &m_pool); }
  iterator begin() const { return const_iterator(m_head, &m_pool); }
  iterator end() const { return const_iterator(nullptr, &m_pool); }
  iterator cbegin() const { return const_iterator(m_head, &m_pool); }
  iterator cend() const { return const_iterator(nullptr, &m_pool); }

 private:
  Pool m_pool;
  std::atomic<Node*> m_head;
};

}  // namespace strobe
