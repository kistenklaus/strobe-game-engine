#include "strobe/rhi/vulkan/swapchain.hpp"
#include "strobe/core/memory/inplace_monotonic_resource.hpp"
#include "strobe/rhi/error/vulkan_error.hpp"
#include "strobe/rhi/vulkan/context/pnf.hpp"

#include <cstring>
#include <fmt/format.h>
#include <stdexcept>
#include <tracy/Tracy.hpp>
#include <vulkan/vulkan_core.h>

namespace strobe::rhi::vulkan {

Swapchain create_swapchain(Context *context, const SwapchainInfo &info) {
  assert(context != nullptr);
  assert(context->properties().swapchain);

  const VkSharingMode sharingMode = info.queueFamilyIndicies.size() > 1
                                        ? VK_SHARING_MODE_CONCURRENT
                                        : VK_SHARING_MODE_EXCLUSIVE;

  const VkSwapchainCreateInfoKHR createInfo{
      .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .pNext = nullptr,
      .flags = 0,
      .surface = info.surface.handle,
      .minImageCount = info.minImageCount,
      .imageFormat = info.format.format,
      .imageColorSpace = info.format.colorSpace,
      .imageExtent =
          {
              .width = info.extent.x(),
              .height = info.extent.y(),
          },
      .imageArrayLayers = 1,
      .imageUsage = info.usage,
      .imageSharingMode = sharingMode,
      .queueFamilyIndexCount =
          sharingMode == VK_SHARING_MODE_CONCURRENT
              ? static_cast<uint32_t>(info.queueFamilyIndicies.size())
              : 0,
      .pQueueFamilyIndices = sharingMode == VK_SHARING_MODE_CONCURRENT
                                 ? info.queueFamilyIndicies.data()
                                 : nullptr,
      .preTransform = info.preTransform,
      .compositeAlpha = info.compositeAlpha,
      .presentMode = info.presentMode,
      .clipped = info.clipped,
      .oldSwapchain = info.oldSwapchain.handle,
  };

  Swapchain swapchain{};

  {
#ifdef STROBE_RHI_TRACE_VK
    ZoneScopedN("vkCreateSwapchainKHR");
#endif
    const VkResult result =
        vkCreateSwapchainKHR(context->device(), &createInfo,
                             context->driver_alloc(), &swapchain.handle);
    if (result != VK_SUCCESS) {
      vulkan_error(result, "Failed to create swapchain");
    }
  }

  return swapchain;
}

void destroy_swapchain(Context *context, Swapchain swapchain) noexcept {
  assert(context != nullptr);
  assert(context->properties().swapchain);
  assert(swapchain);
#ifdef STROBE_RHI_TRACE_VK
  ZoneScopedN("vkDestroySwapchainKHR");
#endif
  vkDestroySwapchainKHR(context->device(), swapchain.handle,
                        context->driver_alloc());
}

uint32_t get_swapchain_images(Context *context, Swapchain swapchain,
                              span<Image> images) {
  assert(context != nullptr);
  assert(context->properties().swapchain);
  assert(swapchain);

  uint32_t count = 0;
  {
#ifdef STROBE_RHI_TRACE_VK
    ZoneScopedN("vkGetSwapchainImagesKHR");
#endif
    VkResult result = vkGetSwapchainImagesKHR(
        context->device(), swapchain.handle, &count, nullptr);
    if (result != VK_SUCCESS) {
      vulkan_error(result, "Failed to get swapchain images");
    }
  }
  if (count > images.size()) {
    return count;
  }
  static constexpr size_t SCRATCH_SIZE = sizeof(VkImage) * 16;
  using scratch_allocator =
      InplaceMonotonicResource<strobe::Mallocator, SCRATCH_SIZE>;
  using scratch_allocator_ref = AllocatorReference<scratch_allocator>;
  scratch_allocator scratch{};

  Vector<VkImage, scratch_allocator_ref> native{count, &scratch};
  {
#ifdef STROBE_RHI_TRACE_VK
    ZoneScopedN("vkGetSwapchainImagesKHR");
#endif
    VkResult result = vkGetSwapchainImagesKHR(
        context->device(), swapchain.handle, &count, native.data());
    if (result != VK_SUCCESS) {
      vulkan_error(result, "Failed to get swapchain images");
    }
  }
  for (size_t i = 0; i < count; ++i) {
    images[i] = Image{.handle = native[i]};
  }
  return count;
}

SwapchainAcquireStatus
acquire_next_swapchain_image(Context *context, Swapchain swapchain,
                             const SwapchainAcquireInfo &info,
                             uint32_t *imageIndex) {
  assert(context != nullptr);
  assert(context->properties().swapchain);
  assert(swapchain);

  VkAcquireNextImageInfoKHR acquireInfo{
      .sType = VK_STRUCTURE_TYPE_ACQUIRE_NEXT_IMAGE_INFO_KHR,
      .pNext = nullptr,
      .swapchain = swapchain.handle,
      .timeout = info.timeout,
      .semaphore = info.signalSemaphore.handle,
      .fence = info.fence.handle,
      .deviceMask = 1,
  };

  VkResult result;
  {
#ifdef STROBE_RHI_TRACE_VK
    ZoneScopedN("vkAcquireNextImage2KHR");
#endif
    result =
        vkAcquireNextImage2KHR(context->device(), &acquireInfo, imageIndex);
  }
  if (result == VK_SUCCESS) {
    return SwapchainAcquireStatus::success;
  } else if (result == VK_SUBOPTIMAL_KHR) {
    return SwapchainAcquireStatus::suboptimal;
  } else if (result == VK_TIMEOUT) {
    return SwapchainAcquireStatus::timeout;
  } else if (result == VK_NOT_READY) {
    return SwapchainAcquireStatus::not_ready;
  } else if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    return SwapchainAcquireStatus::out_of_date;
  } else {
    vulkan_error(result, "Failed to acquire next swapchain image");
  }
}

void release_swapchain_image(Context *context, Swapchain swapchain,
                             uint32_t imageIndex) noexcept {
  VkReleaseSwapchainImagesInfoKHR releaseInfo{
      .sType = VK_STRUCTURE_TYPE_RELEASE_SWAPCHAIN_IMAGES_INFO_KHR,
      .pNext = nullptr,
      .swapchain = swapchain.handle,
      .imageIndexCount = 1,
      .pImageIndices = &imageIndex,
  };

  {
#ifdef STROBE_RHI_TRACE_VK
    ZoneScopedN("vkReleaseSwapchainImages");
#endif
    const VkResult result = vulkan::vk_release_swapchain_images(
        context->pnf(), context->device(), &releaseInfo);
    if (result != VK_SUCCESS) {
      vulkan_error(result, "Failed to release swapchain image");
    }
  }
}

} // namespace strobe::rhi::vulkan
