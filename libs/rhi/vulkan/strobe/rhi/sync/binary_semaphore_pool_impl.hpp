#pragma once

#include "strobe/core/memory/mpsc_monotonic_pool_resource.hpp"
#include "strobe/rhi/context/context.hpp"
#include "strobe/rhi/sync/binary_semaphore_node.hpp"
#include "strobe/rhi/vulkan/binary_semaphore.hpp"
#include <atomic>
#include <memory>

namespace strobe::rhi {

struct BinarySemaphorePoolImpl {
public:
  explicit BinarySemaphorePoolImpl(Context context,
                                   strobe::rhi::allocator_ref alloc) noexcept
      : context(std::move(context)), m_pool(alloc) {}
  ~BinarySemaphorePoolImpl() {
    destroy_nodes(m_ready);
    destroy_nodes(m_returned.load(std::memory_order_relaxed));
  }

  // returns a unsignaled binary semaphore!
  // intenrally synchronized
  BinarySemaphoreNode *alloc() {
    std::lock_guard lck{m_mutex};
    if (m_ready == nullptr) {
      m_ready = m_returned.exchange(nullptr, std::memory_order_acquire);
      if (m_ready == nullptr) {
        return create_node();
      }
    }
    assert(m_ready != nullptr);
    // pop from ready
    auto *node = m_ready;
    m_ready = node->next;
    node->next = nullptr;
    assert(node->signaled == false);
    return node;
  }

  void recycle(BinarySemaphoreNode *node) {
    assert(node);
    assert(node->next == nullptr);
    if (!node->signaled) {
      destroy_nodes(node);
    }

    // lockfree push to returend
    auto *head = m_returned.load(std::memory_order_relaxed);
    do {
      node->next = head;
    } while (m_returned.compare_exchange_weak(
        head, node, std::memory_order_release, std::memory_order_relaxed));
  }

private:
  BinarySemaphoreNode *create_node() {
    auto *node = static_cast<BinarySemaphoreNode *>(m_pool.allocate());
    std::construct_at(node);
    node->semaphore = vulkan::create_binary_semaphore(context.ctx());
    return node;
  }

  void destroy_nodes(BinarySemaphoreNode *node) noexcept {
    while (node != nullptr) {
      BinarySemaphoreNode *next = node->next;
      vulkan::destroy_binary_semaphore(context.ctx(), node->semaphore);
      m_pool.deallocate(node);
      node = next;
    }
  }

private:
  using pool_alloc = MPSCMonotonicPoolResource<sizeof(BinarySemaphoreNode),
                                               alignof(BinarySemaphoreNode),
                                               strobe::rhi::allocator_ref>;
  Context context;
  pool_alloc m_pool;
  std::mutex m_mutex{};
  BinarySemaphoreNode *m_ready = nullptr;
  std::atomic<BinarySemaphoreNode *> m_returned{nullptr};
};

} // namespace strobe::rhi
