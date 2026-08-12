#pragma once
#include "strobe/core/memory/concurrent_monotonic_pool.hpp"
#include <optional>

namespace strobe {

// NOTE: This one as a higher chance of working (i think it solves ABA)
template <typename T, Allocator A> class LockFreeMPSCStack {
private:
  using Pool = ConcurrentMonotonicPool<T, A>;
  using Node = typename Pool::Node;

public:
  explicit LockFreeMPSCStack(const A &alloc = {}) : m_pool(alloc) {}

  LockFreeMPSCStack(const LockFreeMPSCStack &) = delete;
  LockFreeMPSCStack &operator=(const LockFreeMPSCStack &) = delete;

  ~LockFreeMPSCStack() {
    // No concurrent operations allowed during destruction.

    destroy_chain(m_local);

    Node *incoming = m_incoming.exchange(nullptr, std::memory_order_relaxed);

    destroy_chain(incoming);
  }

  template <typename U> void push(U &&value) {
    Node *node = m_pool.allocate();

    try {
      std::construct_at(node->value(), std::forward<U>(value));
    } catch (...) {
      m_pool.deallocate(node);
      throw;
    }

    Node *head = m_incoming.load(std::memory_order_relaxed);

    do {
      node->next = head;
    } while (!m_incoming.compare_exchange_weak(
        head, node, std::memory_order_release, std::memory_order_relaxed));
  }

  std::optional<T> pop() {
    // Single consumer only.

    if (!m_local) {
      m_local = m_incoming.exchange(nullptr, std::memory_order_acquire);

      if (!m_local) {
        return std::nullopt;
      }
    }

    Node *node = m_local;

    // Do this before modifying the list in case T's move throws.
    T result = std::move(*node->value());

    m_local = node->next;

    std::destroy_at(node->value());
    m_pool.deallocate(node);

    return result;
  }

private:
  void destroy_chain(Node *node) noexcept {
    while (node) {
      Node *next = node->next;

      std::destroy_at(node->value());
      m_pool.deallocate(node);

      node = next;
    }
  }

private:
  Pool m_pool;

  // Multi-producer side.
  std::atomic<Node *> m_incoming{nullptr};

  // Single-consumer side. No atomic required.
  Node *m_local = nullptr;
};

} // namespace strobe
