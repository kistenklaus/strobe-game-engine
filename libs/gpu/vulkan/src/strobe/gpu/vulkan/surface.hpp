#pragma once

#include "strobe/core/memory/AllocatorTraits.hpp"
#include "strobe/gpu/vulkan/context/context.hpp"
#include <vulkan/vulkan_core.h>
namespace strobe::gpu::vulkan {

struct Surface {
  VkSurfaceKHR handle = VK_NULL_HANDLE;

  [[nodiscard]]
  explicit operator bool() const noexcept {
    return handle != VK_NULL_HANDLE;
  }
};

Surface create_surface(Context *context, GLFWwindow *window);
void destroy_surface(Context *context, Surface surface);

// ===== capability queries =====
struct SurfaceCapabilities {
  uint32_t minImageCount;
  uint32_t maxImageCount;
  VkExtent2D currentExtent;
  VkExtent2D minImageExtent;
  VkExtent2D maxImageExtent;
  uint32_t maxImageArrayLayers;
  VkSurfaceTransformFlagsKHR supportedTransforms;
  VkSurfaceTransformFlagBitsKHR currentTransform;
  VkCompositeAlphaFlagsKHR supportedCompositeAlpha;
  VkImageUsageFlags supportedUsageFlags;
};

SurfaceCapabilities query_surface_capabilities(Context *context,
                                               Surface surface);

template <Allocator Alloc = strobe::Mallocator>
Vector<VkSurfaceFormatKHR, Alloc>
query_surface_formats(Context *context, Surface surface,
                      const Alloc &alloc = {}) {
  assert(context != nullptr);
  assert(surface != VK_NULL_HANDLE);

  uint32_t count = 0;

  VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(
      context->physicalDevice(), surface.handle, &count, nullptr);

  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to query surface format count");
  }

  Vector<VkSurfaceFormatKHR, Alloc> formats{alloc};

  while (true) {
    formats.resize(count);

    result = vkGetPhysicalDeviceSurfaceFormatsKHR(
        context->physicalDevice(), surface.handle, &count, formats.data());

    if (result == VK_SUCCESS) {
      formats.resize(count);
      return formats;
    }

    if (result != VK_INCOMPLETE) {
      throw std::runtime_error("Failed to query surface formats");
    }

    // The number of formats changed between the two calls.
    // Query the new count and retry.
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(
        context->physicalDevice(), surface.handle, &count, nullptr);

    if (result != VK_SUCCESS) {
      throw std::runtime_error("Failed to query surface format count");
    }
  }
}

template <Allocator Alloc = strobe::Mallocator>
Vector<VkPresentModeKHR, Alloc> query_present_modes(Context *context,
                                                    Surface surface,
                                                    const Alloc &alloc = {}) {
  assert(context != nullptr);
  assert(surface != VK_NULL_HANDLE);

  uint32_t count = 0;

  VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(
      context->physicalDevice(), surface.handle, &count, nullptr);

  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to query present mode count");
  }

  Vector<VkPresentModeKHR, Alloc> modes{alloc};

  while (true) {
    modes.resize(count);

    result = vkGetPhysicalDeviceSurfacePresentModesKHR(
        context->physicalDevice(), surface.handle, &count, modes.data());

    if (result == VK_SUCCESS) {
      modes.resize(count);
      return modes;
    }

    if (result != VK_INCOMPLETE) {
      throw std::runtime_error("Failed to query present modes");
    }

    // Count changed between calls. Query it again and retry.
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(
        context->physicalDevice(), surface.handle, &count, nullptr);

    if (result != VK_SUCCESS) {
      throw std::runtime_error("Failed to query present mode count");
    }
  }
}

} // namespace strobe::gpu::vulkan
