#include "strobe/rhi/vulkan/device_info/memory_properties.hpp"
#include "strobe/core/containers/span.hpp"

namespace strobe::rhi::vulkan {

MemoryProperties details::query_memory_properties(
    VkPhysicalDevice physicalDevice,
    const strobe::rhi::allocator_ref &alloc) noexcept {

  VkPhysicalDeviceMemoryProperties2 props2{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2,
      .pNext = nullptr,
      .memoryProperties = {},
  };
  vkGetPhysicalDeviceMemoryProperties2(physicalDevice, &props2);

  return MemoryProperties{
      .memoryTypes =
          Vector<VkMemoryType, strobe::rhi::allocator_ref>{
              span<VkMemoryType>{
                  props2.memoryProperties.memoryTypes,
                  static_cast<size_t>(props2.memoryProperties.memoryTypeCount)},
              alloc},
      .memoryHeaps =
          Vector<VkMemoryHeap, strobe::rhi::allocator_ref>{
              span<VkMemoryHeap>{
                  props2.memoryProperties.memoryHeaps,
                  static_cast<size_t>(props2.memoryProperties.memoryHeapCount)},
              alloc},
  };
}
} // namespace strobe::rhi::vulkan
