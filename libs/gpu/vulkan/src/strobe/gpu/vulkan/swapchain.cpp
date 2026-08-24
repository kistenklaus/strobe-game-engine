#include "strobe/gpu/vulkan/swapchain.hpp"

#include <cstring>
#include <fmt/format.h>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

namespace strobe::gpu::vulkan {

Swapchain create_swapchain(Context *context, const SwapchainInfo &info) {
  assert(context != nullptr);
  assert(context->properties().swapchain);

  static constexpr size_t SCRATCH_SIZE = sizeof(uint32_t) * 16;

  using scratch_allocator =
      InplaceMonotonicResource<strobe::Mallocator, SCRATCH_SIZE>;

  scratch_allocator scratch{};

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
    ZoneScopedN("vkCreateSwapchainKHR");
    const VkResult result =
        vkCreateSwapchainKHR(context->device(), &createInfo,
                             context->driver_alloc(), &swapchain.handle);
    if (result != VK_SUCCESS) {
      throw std::runtime_error("Failed to create swapchain");
    }
  }

  return swapchain;
}

void destroy_swapchain(Context *context, Swapchain swapchain) noexcept {
  assert(context != nullptr);
  assert(context->properties().swapchain);
  assert(swapchain);
  ZoneScopedN("vkDestroySwapchainKHR");
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
    ZoneScopedN("vkGetSwapchainImagesKHR");
    VkResult result = vkGetSwapchainImagesKHR(
        context->device(), swapchain.handle, &count, nullptr);
    if (result != VK_SUCCESS) {
      throw std::runtime_error("Failed to get swapchain images");
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
    ZoneScopedN("vkGetSwapchainImagesKHR");
    VkResult result = vkGetSwapchainImagesKHR(
        context->device(), swapchain.handle, &count, native.data());
    if (result != VK_SUCCESS) {
      throw std::runtime_error("Failed to get swapchain images");
    }
  }
  for (size_t i = 0; i < count; ++i) {
    images[i] = Image{.handle = native[i]};
  }
  return count;
}

SwapchainAcquireResult
acquire_next_swapchain_image(Context *context, Swapchain swapchain,
                             const SwapchainAcquireInfo &info) {
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

  uint32_t imageIndex = 0;

  VkResult result;
  {
    ZoneScopedN("vkAcquireNextImage2KHR");
    result =
        vkAcquireNextImage2KHR(context->device(), &acquireInfo, &imageIndex);
  }
  if (result == VK_SUCCESS) {
    return SwapchainAcquireResult{
        .status = SwapchainAcquireStatus::success,
        .imageIndex = imageIndex,
    };
  } else if (result == VK_SUBOPTIMAL_KHR) {
    return SwapchainAcquireResult{
        .status = SwapchainAcquireStatus::suboptimal,
        .imageIndex = std::numeric_limits<uint32_t>::max(),
    };
  } else if (result == VK_TIMEOUT) {
    return SwapchainAcquireResult{
        .status = SwapchainAcquireStatus::timeout,
        .imageIndex = std::numeric_limits<uint32_t>::max(),
    };
  } else if (result == VK_NOT_READY) {
    return SwapchainAcquireResult{
        .status = SwapchainAcquireStatus::not_ready,
        .imageIndex = std::numeric_limits<uint32_t>::max(),
    };
  } else if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    return SwapchainAcquireResult{
        .status = SwapchainAcquireStatus::out_of_date,
        .imageIndex = std::numeric_limits<uint32_t>::max(),
    };
  } else {
    throw std::runtime_error("Failed to acquire next swapchain image");
  }
}

} // namespace strobe::gpu::vulkan
