#pragma once

#include "strobe/rhi/objects/object.hpp"
#include "strobe/rhi/objects/swapchain_image.hpp"
#include "strobe/rhi/swapchain/swapchain_frame.hpp"
#include "strobe/rhi/sync/binary_semaphore.hpp"
#include "strobe/rhi/sync/fence.hpp"

namespace strobe::rhi {

class SwapchainGeneration : public Object<SwapchainGeneration> {
  friend Object<SwapchainGeneration>;
  static void pin(void *) noexcept;
  static void unpin(void *) noexcept;

public:
  using Object::Object;
  explicit SwapchainGeneration(void *handle) noexcept : Object(handle) {}

  // may return null, then the swapchain has to be recreated immediatly,
  // otherwise
  SwapchainImage acquire();

  // Afterwards vkQueuePresentKHR must be called!
  std::pair<BinarySemaphore, Fence> present(uint32_t index);

  bool suboptimal() const noexcept;

  const SwapchainFrame &frame(uint32_t imageIndex) const noexcept;
  SwapchainFrame &frame(uint32_t imageIndex) noexcept;

  void release(uint32_t imageIndex) const noexcept;
};

} // namespace strobe::rhi
