#pragma once

#include "strobe/core/containers/small_vector.hpp"
#include "strobe/core/lina/vec.hpp"
#include "strobe/rhi/vulkan/binary_semaphore.hpp"
#include "strobe/rhi/vulkan/context/context.hpp"
#include "strobe/rhi/vulkan/fence.hpp"
#include "strobe/rhi/vulkan/image.hpp"
#include "strobe/rhi/vulkan/present_mode.hpp"
#include "strobe/rhi/vulkan/surface.hpp"
#include "strobe/rhi/vulkan/surface_format.hpp"
#include <limits>
#include <vulkan/vulkan_core.h>

namespace strobe::rhi::vulkan {

struct Swapchain {
  VkSwapchainKHR handle = VK_NULL_HANDLE;

  [[nodiscard]]
  explicit operator bool() const noexcept {
    return handle != VK_NULL_HANDLE;
  }
};

struct SwapchainInfo {
  Surface surface{};

  uint32_t minImageCount = 0;

  VkSurfaceFormatKHR format = {
      .format = VK_FORMAT_UNDEFINED,
      .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
  };

  uvec2 extent = {};

  VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  span<const uint32_t> queueFamilyIndicies = {};

  VkSurfaceTransformFlagBitsKHR preTransform =
      VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

  VkCompositeAlphaFlagBitsKHR compositeAlpha =
      VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

  VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;

  bool clipped = true;

  Swapchain oldSwapchain{};
};

struct SwapchainAcquireInfo {
  // uint64_t timeout = 1'000'000'000ull; // 60 second
  uint64_t timeout = std::numeric_limits<uint64_t>::max();
  BinarySemaphore signalSemaphore = {};
  Fence fence = {};
};

enum class SwapchainAcquireStatus {
  success,
  suboptimal,
  timeout,
  not_ready,
  out_of_date,
};

Swapchain create_swapchain(Context *context, const SwapchainInfo &info = {});

void destroy_swapchain(Context *context, Swapchain swapchain) noexcept;

uint32_t get_swapchain_images(Context *context, Swapchain swapchain,
                              span<Image> images = {});

SwapchainAcquireStatus
acquire_next_swapchain_image(Context *context, Swapchain swapchain,
                             const SwapchainAcquireInfo &info,
                             uint32_t *imageIndex);

void release_swapchain_image(Context *context, Swapchain swapchain,
                             uint32_t imageIndex) noexcept;

} // namespace strobe::rhi::vulkan
