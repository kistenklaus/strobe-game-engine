#pragma once

#include "strobe/core/containers/small_vector.hpp"
#include "strobe/core/containers/span.hpp"
#include "strobe/rhi/context/context.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/img/image_impl.hpp"
#include "strobe/rhi/img/image_view_impl.hpp"
#include "strobe/rhi/img/img.hpp"
#include "strobe/rhi/swapchain/surface.hpp"
#include "strobe/rhi/swapchain/surface_impl.hpp"
#include "strobe/rhi/swapchain/swapchain_generation.hpp"
#include "strobe/rhi/swapchain/swapchain_generation_impl.hpp"
#include "strobe/rhi/swapchain/swapchain_image_impl.hpp"
#include "strobe/rhi/sync/binary_semaphore_impl.hpp"
#include "strobe/rhi/sync/fence_impl.hpp"
#include "strobe/rhi/sync/sync.hpp"
#include "strobe/rhi/types/image_aspect.hpp"
#include "strobe/rhi/utils/format_utilts.hpp"
#include "strobe/rhi/vulkan/binary_semaphore.hpp"
#include "strobe/rhi/vulkan/debug_name.hpp"
#include "strobe/rhi/vulkan/fence.hpp"
#include "strobe/rhi/vulkan/swapchain.hpp"
#include <unistd.h>
#include <vulkan/vulkan_core.h>

namespace strobe::rhi {

struct SwapchainImpl {

  SwapchainImpl(Context context, Surface surface, uint32_t minImageCount,
                VkSurfaceFormatKHR format, VkPresentModeKHR presentMode,
                VkImageUsageFlags imageUsage, span<uint32_t> queueFamilyIndices,
                uvec2 extent, bool clipped)
      : context(std::move(context)), surface(std::move(surface)),
        imageCount(minImageCount), format(format), presentMode(presentMode),
        image_usage(imageUsage), queueFamilyIndicies(queueFamilyIndices),
        clipped(clipped), desired_extent(extent)

  {}

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

      images[i].image = Image{make_void_handle<ImageImpl>(
          imageHandleAlloc, context, MemoryAllocation{}, nativeImages[i],
          ImageType::image_2d, from_vk_format(format.format),
          uvec3{extent.x(), extent.y(), 1}, 1, 1, SampleCount::x1)};

      images[i].view =
          img::create_image_view(images[i].image,
                                 {.type = ImageViewType::image_2d,
                                  .format = Format::undefined,
                                  .range = {.aspect = ImageAspect::color}},
                                 imageViewHandleAlloc);

      vulkan::BinarySemaphore sem =
          vulkan::create_binary_semaphore(context.ctx());
      vulkan::set_debug_name(context.ctx(), sem, "swapchain-present-ready");

      images[i].presentReady =
          sync::create_binary_sem(context, {}, binarySemaphoreHandleAlloc);

      images[i].presentFence =
          sync::create_fence(context, {.signaled = true}, fenceHandleAlloc);
      images[i].pending = false;
    }

    SwapchainGeneration newGeneration{make_void_handle<SwapchainGenerationImpl>(
        &swapchainGenerationHandleAlloc, surface, nativeSwapchain,
        std::move(images), extent, from_vk_format(format.format))};

    generation = std::move(newGeneration);
  }

  const Context context;
  const Surface surface;
  // recreate info
  // - static
  const uint32_t imageCount;
  const VkSurfaceFormatKHR format;
  const VkPresentModeKHR presentMode;
  const VkImageUsageFlags image_usage;
  const SmallVector<uint32_t, 4> queueFamilyIndicies;
  const bool clipped;
  std::atomic<uvec2> desired_extent;

  // TODO, we must still initalize those.
  handle_allocator_ref<ImageImpl> imageHandleAlloc;
  handle_allocator_ref<ImageViewImpl> imageViewHandleAlloc;
  handle_allocator_ref<BinarySemaphoreImpl> binarySemaphoreHandleAlloc;
  handle_allocator_ref<FenceImpl> fenceHandleAlloc;
  handle_allocator<SwapchainImageImpl> swapchainImageHandleAlloc;
  handle_allocator<SwapchainGenerationImpl> swapchainGenerationHandleAlloc;

  // - dynamic
  SwapchainGeneration
      generation{}; // <- handle to live generation. (ref counted)
};

} // namespace strobe::rhi
