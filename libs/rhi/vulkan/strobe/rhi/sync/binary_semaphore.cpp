#include "strobe/rhi/sync/binary_semaphore.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/sync/binary_semaphore_node.hpp"
#include "strobe/rhi/sync/binary_semaphore_pool_impl.hpp"
#include "strobe/rhi/utils/pipeline_stage_utils.hpp"
#include <atomic>
#include <utility>
#include <vulkan/vulkan_core.h>

namespace strobe::rhi {

BinarySemaphore::BinarySemaphore(const BinarySemaphore &o) noexcept
    : m_handle(o.m_handle) {
  if (m_handle != nullptr) {
    auto *node = static_cast<BinarySemaphoreNode *>(m_handle);
    node->refCount.fetch_add(1, std::memory_order_relaxed);
  }
}

BinarySemaphore::BinarySemaphore(BinarySemaphore &&o) noexcept
    : m_handle(std::exchange(o.m_handle, nullptr)) {}

BinarySemaphore &BinarySemaphore::operator=(const BinarySemaphore &o) noexcept {
  if (this == &o) {
    return *this;
  }
  if (o.m_handle != nullptr) {
    auto *node = static_cast<BinarySemaphoreNode *>(o.m_handle);
    node->refCount.fetch_add(1, std::memory_order_relaxed);
  }
  { // unpin
    auto *node = static_cast<BinarySemaphoreNode *>(m_handle);
    if (node->refCount.fetch_sub(1, std::memory_order_acq_rel) == 1) {
      void_handle_ptr<BinarySemaphorePoolImpl>(node->pool)->recycle(node);
    }
  }
  m_handle = o.m_handle;
  return *this;
}

BinarySemaphore &BinarySemaphore::operator=(BinarySemaphore &&o) noexcept {
  if (this == &o) {
    return *this;
  }
  { // unpin
    auto *node = static_cast<BinarySemaphoreNode *>(m_handle);
    if (node->refCount.fetch_sub(1, std::memory_order_acq_rel) == 1) {
      void_handle_ptr<BinarySemaphorePoolImpl>(node->pool)->recycle(node);
    }
  }
  m_handle = std::exchange(o.m_handle, nullptr);
  return *this;
}

BinarySemaphore::~BinarySemaphore() noexcept {
  { // unpin
    auto *node = static_cast<BinarySemaphoreNode *>(m_handle);
    if (node->refCount.fetch_sub(1, std::memory_order_acq_rel) == 1) {
      void_handle_ptr<BinarySemaphorePoolImpl>(node->pool)->recycle(node);
    }
  }
}
VkSemaphoreSubmitInfo
BinarySemaphore::signal(PipelineStage stage) const noexcept {
  assert(m_handle);
  auto *node = static_cast<BinarySemaphoreNode *>(m_handle);
  node->signaled = true;
  return VkSemaphoreSubmitInfo{
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
      .pNext = nullptr,
      .semaphore = node->semaphore.handle,
      .value = 0,
      .stageMask = to_vk_pipeline_stage(stage),
      .deviceIndex = 0,
  };
}

} // namespace strobe::rhi
