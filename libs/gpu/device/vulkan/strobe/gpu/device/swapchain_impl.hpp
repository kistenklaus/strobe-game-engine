#pragma once

#include "strobe/core/containers/small_vector.hpp"
#include "strobe/core/containers/span.hpp"
#include "strobe/gpu/device/device.hpp"
#include "strobe/gpu/device/format_utilts.hpp"
#include "strobe/gpu/device/handle.hpp"
#include "strobe/gpu/device/image_impl.hpp"
#include "strobe/gpu/device/surface.hpp"
#include "strobe/gpu/device/surface_impl.hpp"
#include "strobe/gpu/device/swapchain_generation.hpp"
#include "strobe/gpu/device/swapchain_generation_impl.hpp"
#include "strobe/gpu/vulkan/swapchain.hpp"
#include <vulkan/vulkan_core.h>

namespace strobe::gpu {

struct SwapchainImpl {

  SwapchainImpl(Device device, Surface surface, uint32_t minImageCount,
                VkSurfaceFormatKHR format, VkPresentModeKHR presentMode,
                VkImageUsageFlags imageUsage, span<uint32_t> queueFamilyIndices,
                uvec2 extent, bool clipped)
      : device(std::move(device)), surface(std::move(surface)),
        minImageCount(minImageCount), format(format), presentMode(presentMode),
        image_usage(imageUsage), queueFamilyIndicies(queueFamilyIndices),
        clipped(clipped), desired_extent(extent), generation{} {}
  SwapchainImpl(const SwapchainImpl &) = delete;
  SwapchainImpl(SwapchainImpl &&) = delete;
  SwapchainImpl &operator=(const SwapchainImpl &) = delete;
  SwapchainImpl &operator=(SwapchainImpl &&) = delete;
  ~SwapchainImpl() noexcept = default;

  void recreate() {
    // create new swapchain
    auto *deviceImpl = void_handle_ptr<DeviceImpl>(device.m_handle);
    auto *surfaceImpl = void_handle_ptr<SurfaceImpl>(surface.m_handle);

    const auto capabilities = vulkan::query_surface_capabilities(
        &deviceImpl->context, surfaceImpl->surface);

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
      deviceImpl->context.wait_idle();
    }

    const vulkan::Swapchain nativeSwapchain = vulkan::create_swapchain(
        &deviceImpl->context,
        vulkan::SwapchainInfo{
            .surface = surfaceImpl->surface,
            .minImageCount = minImageCount,
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
        vulkan::get_swapchain_images(&deviceImpl->context, nativeSwapchain);
    SmallVector<vulkan::Image, 4> nativeImages{count};
    vulkan::get_swapchain_images(&deviceImpl->context, nativeSwapchain,
                                 nativeImages);

    SmallVector<Image, 4> images;
    SmallVector<ImageView, 4> views;
    images.reserve(nativeImages.size());
    views.reserve(nativeImages.size());

    for (vulkan::Image nativeImage : nativeImages) {
      Image image = Image{make_void_handle<ImageImpl>(
          device, nativeImage, ImageType::image_2d,
          from_vk_format(format.format), uvec3{extent.x(), extent.y(), 1}, 1, 1,
          SampleCount::x1)};

      ImageView view = image.create_view();

      images.push_back(std::move(image));
      views.push_back(std::move(view));
    }

    SwapchainGeneration newGeneration{make_void_handle<SwapchainGenerationImpl>(
        surface, nativeSwapchain, images, views, extent,
        from_vk_format(format.format))};

    generation = std::move(newGeneration);
  }

  const Device device;
  const Surface surface;
  // recreate info
  // - static
  const uint32_t minImageCount;
  const VkSurfaceFormatKHR format;
  const VkPresentModeKHR presentMode;
  const VkImageUsageFlags image_usage;
  const SmallVector<uint32_t, 4> queueFamilyIndicies;
  const bool clipped;
  // - dynamic
  std::atomic<uvec2> desired_extent;

  SwapchainGeneration generation; // <- handle to live generation. (ref counted)
};

} // namespace strobe::gpu
