#pragma once

#include "strobe/core/containers/small_vector.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/img/image_impl.hpp"
#include "strobe/rhi/objects/binary_semaphore.hpp"
#include "strobe/rhi/objects/fence.hpp"
#include "strobe/rhi/objects/image.hpp"
#include "strobe/rhi/swapchain/surface.hpp"
#include "strobe/rhi/swapchain/surface_impl.hpp"
#include "strobe/rhi/vulkan/command_buffer.hpp"
#include "strobe/rhi/vulkan/command_pool.hpp"
#include "strobe/rhi/vulkan/swapchain.hpp"
#include <vulkan/vulkan_core.h>

namespace strobe::rhi {

struct SwapchainImageState {
  Image image;
  ImageView view;
  vulkan::BinarySemaphore presentReady;
  vulkan::Fence presentFence;
  bool pending = false;

  // just to keep them alive, until the next acquire
  BinarySemaphore acquireSignal{};
  Fence acquireFence{};
};

struct SwapchainGenerationImpl {
  SwapchainGenerationImpl(Surface surface, vulkan::Swapchain swapchain,
                          SmallVector<SwapchainImageState, 4> images,
                          uvec2 extent, Format format)
      : surface(std::move(surface)), images(std::move(images)),
        swapchain(swapchain), extent(extent), format(format) {}

  ~SwapchainGenerationImpl() noexcept {
    for (auto &state : images) {
      if (state.pending) {
        state.presentFence.wait();
      }
    }
    auto *surface_impl = void_handle_ptr<SurfaceImpl>(surface.m_handle);
    vulkan::destroy_swapchain(surface_impl->context.ctx(), swapchain);
  }

  const Surface surface;
  SmallVector<SwapchainImageState> images;

  const vulkan::Swapchain swapchain;
  const uvec2 extent;
  const Format format;
};

} // namespace strobe::rhi
