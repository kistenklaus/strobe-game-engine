#pragma once

#include "strobe/core/containers/small_vector.hpp"
#include "strobe/core/containers/span.hpp"
#include "strobe/rhi/device/context.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/memory/image_handle_alloc.hpp"
#include "strobe/rhi/memory/image_impl.hpp"
#include "strobe/rhi/queue/binary_semaphore_impl.hpp"
#include "strobe/rhi/queue/fence_impl.hpp"
#include "strobe/rhi/swapchain/surface.hpp"
#include "strobe/rhi/swapchain/surface_impl.hpp"
#include "strobe/rhi/swapchain/swapchain_generation.hpp"
#include "strobe/rhi/swapchain/swapchain_generation_impl.hpp"
#include "strobe/rhi/utils/format_utilts.hpp"
#include "strobe/rhi/vulkan/binary_semaphore.hpp"
#include "strobe/rhi/vulkan/debug_name.hpp"
#include "strobe/rhi/vulkan/fence.hpp"
#include "strobe/rhi/vulkan/swapchain.hpp"
#include <vulkan/vulkan_core.h>

namespace strobe::rhi {

struct SwapchainImpl {

  SwapchainImpl(Context context, Surface surface, uint32_t minImageCount,
                VkSurfaceFormatKHR format, VkPresentModeKHR presentMode,
                VkImageUsageFlags imageUsage, span<uint32_t> queueFamilyIndices,
                uvec2 extent, bool clipped)
      : context(std::move(context)), surface(std::move(surface)), m_alloc{},
        m_imageHandleAlloc{&m_alloc}, imageCount(minImageCount), format(format),
        presentMode(presentMode), image_usage(imageUsage),
        queueFamilyIndicies(queueFamilyIndices), clipped(clipped),
        desired_extent(extent), generation{} {}

  SwapchainImpl(const SwapchainImpl &) = delete;
  SwapchainImpl(SwapchainImpl &&) = delete;
  SwapchainImpl &operator=(const SwapchainImpl &) = delete;
  SwapchainImpl &operator=(SwapchainImpl &&) = delete;
  ~SwapchainImpl() noexcept = default;

  void recreate() {
    ZoneScopedN("swapchain-recreate");
    // create new swapchain
    // auto *deviceImpl = void_handle_ptr<DeviceImpl>(device.m_handle);
    auto *surfaceImpl = void_handle_ptr<SurfaceImpl>(surface.m_handle);

    const auto capabilities =
        vulkan::query_surface_capabilities(context.ctx(), surfaceImpl->surface);

    uvec2 extent;
    if (capabilities.currentExtent.width != UINT32_MAX) {
      extent = {capabilities.currentExtent.width,
                capabilities.currentExtent.height};
    } else {
      const uvec2 desired = desired_extent.load(std::memory_order_relaxed);
      extent = {
          std::clamp(desired.x(), capabilities.minImageExtent.width,
                     capabilities.maxImageExtent.width),
          std::clamp(desired.y(), capabilities.minImageExtent.height,
                     capabilities.maxImageExtent.height),
      };
    }

    VkCompositeAlphaFlagBitsKHR compositeAlpha;
    if (capabilities.supportedCompositeAlpha &
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR) {
      compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    } else if (capabilities.supportedCompositeAlpha &
               VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR) {
      compositeAlpha = VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR;
    } else if (capabilities.supportedCompositeAlpha &
               VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR) {
      compositeAlpha = VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR;
    } else {
      compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
    }

    vulkan::Swapchain oldSwapchain{};
    if (generation) {
      auto *oldGeneration =
          void_handle_ptr<SwapchainGenerationImpl>(generation.m_handle);
      // wait for old swapchain to be truely gone!
      oldSwapchain = oldGeneration->swapchain;
    }

    const vulkan::Swapchain nativeSwapchain = vulkan::create_swapchain(
        context.ctx(),
        vulkan::SwapchainInfo{
            .surface = surfaceImpl->surface,
            .minImageCount = imageCount,
            .format = format,
            .extent = extent,
            .usage = image_usage,
            .queueFamilyIndicies = span<const uint32_t>{queueFamilyIndicies},
            .preTransform = capabilities.currentTransform,
            .compositeAlpha = compositeAlpha,
            .presentMode = presentMode,
            .clipped = clipped,
            .oldSwapchain = oldSwapchain,
        });
    uint32_t count =
        vulkan::get_swapchain_images(context.ctx(), nativeSwapchain);
    SmallVector<vulkan::Image, 8> nativeImages{count};
    vulkan::get_swapchain_images(context.ctx(), nativeSwapchain, nativeImages);

    SmallVector<SwapchainImageState, 4> images{nativeImages.size()};
    for (uint32_t i = 0; i < images.size(); ++i) {

      images[i].image =
          Image{alloc_void_handle<ImageImpl, image_handle_alloc_ref>(
              image_handle_alloc_ref{&m_imageHandleAlloc}, context,
              MemoryAllocation{}, nativeImages[i], ImageType::image_2d,
              from_vk_format(format.format), uvec3{extent.x(), extent.y(), 1},
              1, 1, SampleCount::x1)};

      images[i].view = images[i].image.create_view(ImageViewType::image_2d,
                                                   ImageAspect::color);

      vulkan::BinarySemaphore sem =
          vulkan::create_binary_semaphore(context.ctx());
      vulkan::set_debug_name(context.ctx(), sem, "swapchain-present-ready");
      images[i].presentReady =
          BinarySemaphore{make_void_handle<BinarySemaphoreImpl>(context, sem)};

      vulkan::Fence fence = vulkan::create_fence(
          context.ctx(), {.flags = VK_FENCE_CREATE_SIGNALED_BIT});
      images[i].fence = Fence{make_void_handle<FenceImpl>(context, fence)};
      images[i].pending = false;
    }

    SwapchainGeneration newGeneration{make_void_handle<SwapchainGenerationImpl>(
        surface, nativeSwapchain, std::move(images), extent,
        from_vk_format(format.format))};

    auto *genImpl =
        void_handle_ptr<SwapchainGenerationImpl>(newGeneration.m_handle);
    for (auto &state : genImpl->images) {
      auto *semImpl =
          void_handle_ptr<BinarySemaphoreImpl>(state.presentReady.m_handle);
      assert(semImpl->swapchainGeneration == nullptr);
      semImpl->swapchainGeneration = genImpl;
    }

    generation = std::move(newGeneration);
  }

  const Context context;
  const Surface surface;
  strobe::rhi::allocator m_alloc;
  image_handle_alloc m_imageHandleAlloc;
  // recreate info
  // - static
  const uint32_t imageCount;
  const VkSurfaceFormatKHR format;
  const VkPresentModeKHR presentMode;
  const VkImageUsageFlags image_usage;
  const SmallVector<uint32_t, 4> queueFamilyIndicies;
  const bool clipped;
  // - dynamic
  std::atomic<uvec2> desired_extent;

  SwapchainGeneration generation; // <- handle to live generation. (ref counted)
};

} // namespace strobe::rhi
