#pragma once

#include "strobe/core/containers/vector.hpp"
#include "strobe/core/lina/vec.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/objects/image.hpp"
#include "strobe/rhi/swapchain/surface.hpp"
#include "strobe/rhi/swapchain/swapchain_frame.hpp"
#include "strobe/rhi/swapchain/swapchain_image_impl.hpp"
#include "strobe/rhi/swapchain/swapchain_present_frame.hpp"
#include "strobe/rhi/sync/binary_semaphore.hpp"
#include "strobe/rhi/sync/binary_semaphore_pool.hpp"
#include "strobe/rhi/sync/fence.hpp"
#include "strobe/rhi/sync/fence_pool.hpp"
#include "strobe/rhi/types/format.hpp"

#include "strobe/rhi/vulkan/swapchain.hpp"

namespace strobe::rhi {

struct SwapchainGenerationImpl {

  explicit SwapchainGenerationImpl(
      Surface surface, FencePool fencePool, BinarySemaphorePool semPool,
      vulkan::Swapchain swapchain,
      Vector<SwapchainFrame, strobe::rhi::allocator_ref> frames, uvec2 extent,
      Format format,
      handle_allocator_ref<SwapchainImageImpl> swapchainImageHandleAllocator,
      strobe::rhi::allocator_ref alloc) noexcept
      : surface(std::move(surface)), fencePool(std::move(fencePool)),
        semPool(semPool), swapchain(swapchain), frames(std::move(frames)),
        extent(extent), format(format), m_presentFramePool(alloc),
        m_imageAlloc(swapchainImageHandleAllocator) {}
  SwapchainGenerationImpl(const SwapchainGenerationImpl &) = delete;
  SwapchainGenerationImpl(SwapchainGenerationImpl &&) = delete;

  ~SwapchainGenerationImpl() noexcept {
    vulkan::destroy_swapchain(surface.ctx(), swapchain);
  }

  handle_allocator_ref<SwapchainImageImpl>
  get_swapchain_image_handle_allocator() const noexcept {
    return m_imageAlloc;
  }

  swapchain_present_frame_pool_ref get_present_frame_allocator() noexcept {
    return &m_presentFramePool;
  }

  Surface surface;
  FencePool fencePool;
  BinarySemaphorePool semPool;
  vulkan::Swapchain swapchain;
  Vector<SwapchainFrame, strobe::rhi::allocator_ref> frames;
  uvec2 extent;
  Format format;
  bool suboptimal = false;

  std::atomic<uint64_t> debugCounter{0};

private:
  swapchain_present_frame_pool m_presentFramePool;
  handle_allocator_ref<SwapchainImageImpl> m_imageAlloc;
};

} // namespace strobe::rhi
