#pragma once

#include "strobe/core/containers/vector.hpp"
#include "strobe/rhi/allocator.hpp"
#include <vulkan/vulkan_core.h>

namespace strobe::rhi::vulkan {

struct MemoryProperties {
  // VK_API_VERSION_1_0
  Vector<VkMemoryType, strobe::rhi::allocator_ref> memoryTypes;
  Vector<VkMemoryHeap, strobe::rhi::allocator_ref> memoryHeaps;
  // VK_API_VERSION_1_1
  // ...
  // VK_API_VERSION_1_2
  // ...
  // VK_API_VERSION_1_3
  // ...
  // VK_API_VERSION_1_4
};

namespace details {

MemoryProperties
query_memory_properties(VkPhysicalDevice physicalDevice,
                        const strobe::rhi::allocator_ref &alloc) noexcept;
} // namespace details

} // namespace strobe::rhi::vulkan
