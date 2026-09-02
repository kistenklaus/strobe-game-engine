#pragma once

#include "strobe/core/containers/small_vector.hpp"
#include "strobe/core/memory/mpsc_monotonic_pool_resource.hpp"
#include "strobe/rhi/allocator.hpp"
#include "strobe/rhi/context/context.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/sync/fence_impl.hpp"
#include "strobe/rhi/sync/fence_node.hpp"
#include "strobe/rhi/vulkan/fence.hpp"
#include <mutex>
#include <atomic>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

namespace strobe::rhi {

struct FencePoolImpl {
public:
  FencePoolImpl(Context context,
                handle_allocator_ref<FenceImpl> fenceHandleAllocator,
                strobe::rhi::allocator_ref alloc) noexcept
      : context(std::move(context)), m_pool(alloc),
        m_fenceAlloc(fenceHandleAllocator) {}

  ~FencePoolImpl() noexcept {
    destroy_nodes(m_ready);
    destroy_nodes(m_returned.load(std::memory_order_relaxed));
  }

  FenceNode *allocate() {
    ZoneScopedN("sync/allocate-fence");
    std::lock_guard lck{m_mutex};
    if (m_ready == nullptr) {
      m_ready = m_returned.exchange(nullptr, std::memory_order_acquire);
      if (m_ready == nullptr) {
        return create_node();
      } else {
        // We just popped possibly signaled fences,
        // => reset all of those in batches.
        FenceNode *it = m_ready;
        while (it != nullptr) {
          static constexpr size_t RESET_BATCH_SIZE = 16;
          SmallVector<VkFence, RESET_BATCH_SIZE> fences;
          while (it != nullptr && fences.size() < RESET_BATCH_SIZE) {
            fences.push(it->fence.handle);
            it = it->next;
          }
          VkResult result;
          {
            ZoneScopedN("vkResetFences");
            result = vkResetFences(context.ctx()->device(), fences.size(),
                                   fences.data());
          }
          if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to reset fences");
          }
        }
      }
    }
    assert(m_ready != nullptr);
    // pop from ready
    auto *node = m_ready;
    m_ready = node->next;
    node->next = nullptr;
    return node;
  }

  void recycle(FenceNode *node) noexcept {
    assert(node);
    assert(node->next == nullptr);

    // lockfree push to returend
    auto *head = m_returned.load(std::memory_order_relaxed);
    do {
      node->next = head;
    } while (!m_returned.compare_exchange_weak(
        head, node, std::memory_order_release, std::memory_order_relaxed));
  }

  handle_allocator_ref<FenceImpl> get_fence_handle_alloc() const noexcept {
    return m_fenceAlloc;
  }

  vulkan::Context* ctx() const noexcept {
    return context.ctx();
  }

private:
  FenceNode *create_node() {
    auto *node = static_cast<FenceNode *>(m_pool.allocate());
    std::construct_at(node);
    node->fence = vulkan::create_fence(context.ctx());
    return node;
  }

  void destroy_nodes(FenceNode *node) noexcept {
    while (node != nullptr) {
      auto *next = node->next;
      vulkan::destroy_fence(context.ctx(), node->fence);
      m_pool.deallocate(node);
      node = next;
    }
  }

private:
  using pool_alloc =
      MPSCMonotonicPoolResource<sizeof(FenceNode), alignof(FenceNode),
                                strobe::rhi::allocator_ref>;

  Context context;
  pool_alloc m_pool;
  handle_allocator_ref<FenceImpl> m_fenceAlloc;

#ifdef STROBE_RHI_TRACE_LOCKS
  TracyLockableN(std::mutex, m_mutex, "FencePool-mutex");
#else
  std::mutex m_mutex{};
#endif
  FenceNode *m_ready = nullptr;
  std::atomic<FenceNode *> m_returned{nullptr};
};

} // namespace strobe::rhi
