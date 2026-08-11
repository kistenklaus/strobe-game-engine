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

template <Allocator Alloc = strobe::Mallocator> struct SurfaceCapabilities {
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

  Vector<VkSurfaceFormatKHR, Alloc> formats{};
  Vector<VkPresentModeKHR, Alloc> presentModes{};
};

template <Allocator Alloc = strobe::Mallocator>
SurfaceCapabilities<Alloc> get_surface_capabilities(Context *context,
                                                    VkSurfaceKHR surface,
                                                    const Alloc &alloc = {}) {
  assert(context != nullptr);
  assert(surface != VK_NULL_HANDLE);
  SurfaceCapabilities<Alloc> cap{
      .formats = Vector<VkSurfaceFormatKHR, Alloc>{alloc},
      .presentModes = Vector<VkPresentModeKHR, Alloc>{alloc},
  };
  {
    VkSurfaceCapabilitiesKHR native{};
    const VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        context->physicalDevice(), surface, &native);
    if (result != VK_SUCCESS) {
      throw std::runtime_error("Failed to query surface capabilities");
    }
    cap.minImageCount = native.minImageCount;
    cap.maxImageCount = native.maxImageCount;
    cap.currentExtent = native.currentExtent;
    cap.minImageExtent = native.minImageExtent;
    cap.maxImageExtent = native.maxImageExtent;
    cap.maxImageArrayLayers = native.maxImageArrayLayers;
    cap.supportedTransforms = native.supportedTransforms;
    cap.currentTransform = native.currentTransform;
    cap.supportedCompositeAlpha = native.supportedCompositeAlpha;
    cap.supportedUsageFlags = native.supportedUsageFlags;
  }
  while (true) {
    uint32_t count = 0;
    VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(
        context->physicalDevice(), surface, &count, nullptr);
    if (result != VK_SUCCESS) {
      throw std::runtime_error("Failed to query surface format count");
    }
    cap.formats.resize(count);
    if (count == 0) {
      break;
    }
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(
        context->physicalDevice(), surface, &count, cap.formats.data());
    if (result == VK_SUCCESS) {
      cap.formats.resize(count);
      break;
    }
    if (result != VK_INCOMPLETE) {
      throw std::runtime_error("Failed to query surface formats");
    }
  }
  while (true) {
    uint32_t count = 0;
    VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(
        context->physicalDevice(), surface, &count, nullptr);
    if (result != VK_SUCCESS) {
      throw std::runtime_error("Failed to query present mode count");
    }
    cap.presentModes.resize(count);
    if (count == 0) {
      break;
    }
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(
        context->physicalDevice(), surface, &count, cap.presentModes.data());
    if (result == VK_SUCCESS) {
      cap.presentModes.resize(count);
      break;
    }
    if (result != VK_INCOMPLETE) {
      throw std::runtime_error("Failed to query present modes");
    }
  }
  return cap;
}

} // namespace strobe::gpu::vulkan
