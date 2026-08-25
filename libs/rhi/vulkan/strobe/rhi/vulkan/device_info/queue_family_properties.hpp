#pragma once

#include "strobe/core/containers/vector.hpp"
#include "strobe/rhi/allocator.hpp"

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace strobe::rhi::vulkan {

struct QueueFamilyProperties {
  // VK_API_VERSION_1_0
  VkQueueFlags queueFlags;
  uint32_t queueCount;
  uint32_t timestampValidBits;
  VkExtent3D minImageTransferGranularity;

  // VK_API_VERSION_1_1
  // ...

  // VK_API_VERSION_1_2
  // ...

  // VK_API_VERSION_1_3
  // ...

  // VK_API_VERSION_1_4
  // ...

  // GLFW presentation support
  bool presentationSupport;
};

namespace details {

Vector<QueueFamilyProperties, strobe::rhi::allocator_ref>
query_queue_family_properties(VkInstance instance,
                              VkPhysicalDevice physicalDevice,
                              const strobe::rhi::allocator_ref &alloc) noexcept;
}

} // namespace strobe::rhi::vulkan
