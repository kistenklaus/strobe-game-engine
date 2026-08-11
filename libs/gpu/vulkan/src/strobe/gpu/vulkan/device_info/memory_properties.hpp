#pragma once

#include "strobe/core/containers/span.hpp"
#include "strobe/core/containers/vector.hpp"
#include "strobe/core/memory/AllocatorTraits.hpp"
#include "strobe/core/memory/Mallocator.hpp"
#include <vulkan/vulkan_core.h>

namespace strobe::gpu::vulkan {

template <Allocator Alloc = strobe::Mallocator> struct MemoryProperties {
  // VK_API_VERSION_1_0
  Vector<VkMemoryType, Alloc> memoryTypes;
  Vector<VkMemoryHeap, Alloc> memoryHeaps;
  // VK_API_VERSION_1_1
  // ...
  // VK_API_VERSION_1_2
  // ...
  // VK_API_VERSION_1_3
  // ...
  // VK_API_VERSION_1_4
};

template <Allocator Alloc = strobe::Mallocator>
static MemoryProperties<Alloc>
query_memory_properties(VkPhysicalDevice physicalDevice,
                        const Alloc &alloc = {}) noexcept {

  VkPhysicalDeviceMemoryProperties2 props2{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2,
      .pNext = nullptr,
      .memoryProperties = {},
  };
  vkGetPhysicalDeviceMemoryProperties2(physicalDevice, &props2);

  return MemoryProperties<Alloc>{
      .memoryTypes =
          Vector<VkMemoryType, Alloc>{
              span<VkMemoryType>{
                  props2.memoryProperties.memoryTypes,
                  static_cast<size_t>(props2.memoryProperties.memoryTypeCount)},
              alloc},
      .memoryHeaps =
          Vector<VkMemoryHeap, Alloc>{
              span<VkMemoryHeap>{
                  props2.memoryProperties.memoryHeaps,
                  static_cast<size_t>(props2.memoryProperties.memoryHeapCount)},
              alloc},
  };
}

} // namespace strobe::gpu::vulkan
