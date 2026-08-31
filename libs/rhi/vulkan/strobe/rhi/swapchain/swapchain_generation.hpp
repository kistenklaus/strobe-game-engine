#pragma once

#include "strobe/rhi/objects/object.hpp"
#include "strobe/rhi/swapchain/swapchain_frame.hpp"
#include "strobe/rhi/swapchain/swapchain_image.hpp"
#include "strobe/rhi/swapchain/swapchain_image.hpp"
#include "strobe/rhi/sync/binary_semaphore.hpp"
#include "strobe/rhi/sync/fence.hpp"

namespace strobe::rhi {

class SwapchainGeneration : Object<SwapchainGeneration> {
  friend class SwapchainImage;
public:
  explicit SwapchainGeneration(void *handle) noexcept : Object(handle) {}
  SwapchainGeneration() noexcept : Object(nullptr) {}
  SwapchainGeneration(const SwapchainGeneration &) noexcept;
  SwapchainGeneration(SwapchainGeneration &&) noexcept;
  SwapchainGeneration &operator=(const SwapchainGeneration &) noexcept;
  SwapchainGeneration &operator=(SwapchainGeneration &&) noexcept;
  ~SwapchainGeneration() noexcept;
  explicit operator bool() const noexcept { return m_handle != nullptr; }
  friend bool operator==(const SwapchainGeneration &lhs,
                         const SwapchainGeneration &rhs) noexcept {
    return lhs.m_handle == rhs.m_handle;
  }
  friend bool operator!=(const SwapchainGeneration &lhs,
                         const SwapchainGeneration &rhs) noexcept {
    return lhs.m_handle != rhs.m_handle;
  }

  // may return null, then the swapchain has to be recreated immediatly, otherwise 
  SwapchainImage acquire();

  // Afterwards vkQueuePresentKHR must be called!
  std::pair<BinarySemaphore, Fence> present();

  bool suboptimal() const noexcept;

  const SwapchainFrame &frame(uint32_t imageIndex) const noexcept;
  SwapchainFrame &frame(uint32_t imageIndex) noexcept;
};

} // namespace strobe::rhi
