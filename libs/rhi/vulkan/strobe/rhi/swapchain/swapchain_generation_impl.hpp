#pragma once

#include "strobe/core/containers/small_vector.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/memory/image_impl.hpp"
#include "strobe/rhi/objects/fence.hpp"
#include "strobe/rhi/swapchain/surface.hpp"
#include "strobe/rhi/swapchain/surface_impl.hpp"
// #include "strobe/rhi/vulkan/cmd/barrier.hpp"
#include "strobe/rhi/vulkan/command_buffer.hpp"
#include "strobe/rhi/vulkan/command_pool.hpp"
#include "strobe/rhi/vulkan/swapchain.hpp"
#include <vulkan/vulkan_core.h>

namespace strobe::rhi {

struct SwapchainImageState {
  Image image;
  ImageView view;
  BinarySemaphore presentReady;
  Fence fence;
  bool pending = false;

  // strong refs
  BinarySemaphore acquireSignal{};
  Fence acquireFence{};

  struct Transitions {
    vulkan::CommandBuffer postAcquire;
    vulkan::CommandBuffer prePresent;
  };
  SmallVector<Transitions, 4> transitions;
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
        state.fence.wait();
      }
    }
    auto *surface_impl = void_handle_ptr<SurfaceImpl>(surface.m_handle);
    for (vulkan::CommandPool cmdPool : cmdPools) {
      if (cmdPool) {
        vulkan::destroy_command_pool(surface_impl->context.ctx(), cmdPool);
      }
    }

    vulkan::destroy_swapchain(surface_impl->context.ctx(), swapchain);
  }

  vulkan::CommandBuffer getPostAcquireCmdBuf(uint32_t imageIndex,
                                             uint32_t queueFamilyIndex) {
    ensure_transition_cache(queueFamilyIndex);

    assert(imageIndex < images.size());
    return images[imageIndex].transitions[queueFamilyIndex].postAcquire;
  }

  vulkan::CommandBuffer getPrePresentCmdBuf(uint32_t imageIndex,
                                            uint32_t queueFamilyIndex) {
    ensure_transition_cache(queueFamilyIndex);
    assert(imageIndex < images.size());
    return images[imageIndex].transitions[queueFamilyIndex].prePresent;
    ensure_transition_cache(queueFamilyIndex);
  }

  uint32_t findAcquireImage(const BinarySemaphore &semaphore) const {
    for (uint32_t i = 0; i < images.size(); ++i) {
      if (images[i].acquireSignal == semaphore) {
        return i;
      }
    }
    assert(false && "Swapchain acquire semaphore not found");
    return 0;
  }

  uint32_t findPresentImage(const BinarySemaphore &semaphore) const {
    for (uint32_t i = 0; i < images.size(); ++i) {
      if (images[i].presentReady == semaphore) {
        return i;
      }
    }
    assert(false && "Swapchain present semaphore not found");
    return 0;
  }

  const Surface surface;
  SmallVector<vulkan::CommandPool, 2>
      cmdPools; // sparsly index by queue family index.

  SmallVector<SwapchainImageState> images;

  const vulkan::Swapchain swapchain;
  const uvec2 extent;
  const Format format;

private:
  void ensure_transition_cache(uint32_t queueFamilyIndex) {
    if (cmdPools.size() <= queueFamilyIndex) {
      cmdPools.resize(queueFamilyIndex + 1);
    }
    if (cmdPools[queueFamilyIndex]) {
      return;
    }
    auto *surfaceImpl = void_handle_ptr<SurfaceImpl>(surface.m_handle);
    vulkan::CommandPool pool{};
    VkCommandPoolCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .queueFamilyIndex = queueFamilyIndex,
    };

    {
      ZoneScopedN("vkCreateCommandPool");
      VkResult result = vkCreateCommandPool(
          surfaceImpl->context.ctx()->device(), &createInfo,
          surfaceImpl->context.ctx()->driver_alloc(), &pool.handle);
      if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to create command buffer");
      }
    }

    cmdPools[queueFamilyIndex] = pool;

    for (auto &state : images) {
      if (state.transitions.size() <= queueFamilyIndex) {
        state.transitions.resize(queueFamilyIndex + 1);
      }

      auto postAcquire = vulkan::alloc_command_buffer(
          surfaceImpl->context.ctx(), {.pool = pool});

      auto prePresent = vulkan::alloc_command_buffer(surfaceImpl->context.ctx(),
                                                     {.pool = pool});

      cmd_record_post_acquire(
          postAcquire, void_handle_ptr<ImageImpl>(state.image.m_handle)->image);

      cmd_record_pre_present(
          prePresent, void_handle_ptr<ImageImpl>(state.image.m_handle)->image);

      state.transitions[queueFamilyIndex] = {
          .postAcquire = postAcquire,
          .prePresent = prePresent,
      };
    }
  }

  void cmd_record_post_acquire(vulkan::CommandBuffer cmd, vulkan::Image image) {
    // TODO: Replace me
    // vulkan::begin_command_buffer(cmd);
    // vulkan::ImageMemoryBarrier barrier{
    //     .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    //     .newLayout = VK_IMAGE_LAYOUT_GENERAL,
    //     .image = image,
    // };
    // vulkan::cmd_pipeline_barrier(cmd, {.imageBarriers = {&barrier, 1}});
    // vulkan::end_command_buffer(cmd);
  }

  void cmd_record_pre_present(vulkan::CommandBuffer cmd, vulkan::Image image) {
    // TODO: Replace me
    // vulkan::begin_command_buffer(cmd);
    // vulkan::ImageMemoryBarrier barrier{
    //     .oldLayout = VK_IMAGE_LAYOUT_GENERAL,
    //     .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    //     .image = image,
    // };
    // vulkan::cmd_pipeline_barrier(cmd, {.imageBarriers = {&barrier, 1}});
    // vulkan::end_command_buffer(cmd);
  }
};

} // namespace strobe::rhi
