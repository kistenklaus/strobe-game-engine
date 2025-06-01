#pragma once

#include <atomic>

#include "strobe/core/memory/AllocatorTraits.hpp"
#include "strobe/core/memory/monotonic_pool_allocator.hpp"
namespace strobe {

template <typename T, Allocator A>
class LockFreeMonotonicForwardList {
 private:
  struct Node {
    T m_value;
    std::atomic<Node*> m_next;
  };
  using Pool = MonotonicPoolResource<sizeof(Node), alignof(Node), A>;
  using PoolTraits = AllocatorTraits<Pool>;

 public:
  LockFreeMonotonicForwardList(const A& alloc)
      : m_pool(alloc), m_head(nullptr) {}

  ~LockFreeMonotonicForwardList() {
    Node* head = m_head.load(std::memory_order_acquire);
    while (head != nullptr) {
      std::destroy_at(&head->m_value);
      head->m_next = head;
    }
    m_head = nullptr;
  }
  // Both are in principle possible.
  LockFreeMonotonicForwardList(const LockFreeMonotonicForwardList&) = delete;
  LockFreeMonotonicForwardList& operator=(const LockFreeMonotonicForwardList&) =
      delete;
  LockFreeMonotonicForwardList(LockFreeMonotonicForwardList&&) = delete;
  LockFreeMonotonicForwardList& operator=(LockFreeMonotonicForwardList&&) =
      delete;

  class iterator {
   public:
    friend LockFreeMonotonicForwardList;
    iterator() : m_node(static_cast<Node*>(-1)) {}
    T& operator*() const {
      assert(m_node != nullptr);
      return m_node->value;
    }
    T* operator->() const {
      assert(m_node != nullptr);
      return &m_node->value;
    }

    iterator& operator++() {
      m_node = m_node->m_next.load(std::memory_order_relaxed);
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
    explicit iterator(Node* node) : m_node(node) {}
    Node* m_node;
  };

  class const_iterator {
   public:
    friend LockFreeMonotonicForwardList;
    const_iterator() : m_node(static_cast<Node*>(-1)) {}

    T& operator*() const {
      assert(m_node != nullptr);
      return m_node->value;
    }
    T* operator->() const {
      assert(m_node != nullptr);
      return &m_node->value;
    }

    const_iterator& operator++() {
      m_node = m_node->m_next.load(std::memory_order_relaxed);
    }

    const_iterator operator++(int) {
      iterator temp = *this;
      ++(*this);
      return temp;
    }

    bool operator==(const const_iterator& other) const {
      return m_node == other.m_node;
    }
    bool operator!=(const const_iterator& other) const {
      return m_node != other.m_node;
    }

   private:
    explicit const_iterator(Node* node) : m_node(node) {}
    Node* m_node;
  };

  template <typename U>
  void push(U&& value) {
    Node* node = PoolTraits::template allocate<Node>(m_pool);
    new (&node->m_value) T(std::forward<U>(value));

    Node* head = m_head.load(std::memory_order_relaxed);
    do {
      node->m_next = head;
    } while (m_head.compare_exchange_weak(head, node, std::memory_order_release,
                                          std::memory_order_relaxed));
  }

  iterator begin() { return iterator(m_head.load(std::memory_order_relaxed)); }
  iterator end() { return iterator(nullptr); }

  const_iterator begin() const {
    return const_iterator(m_head.load(std::memory_order_relaxed));
  }
  const_iterator end() const { return const_iterator(nullptr); }

  const_iterator cbegin() const {
    return const_iterator(m_head.load(std::memory_order_relaxed));
  }
  const_iterator cend() const { return const_iterator(nullptr); }

 private:
  Pool m_pool;
  std::atomic<Node*> m_head;
};

}  // namespace strobe
