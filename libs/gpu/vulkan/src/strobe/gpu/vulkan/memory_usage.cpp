#include "strobe/gpu/vulkan/memory_usage.hpp"
#include <cassert>
#include <iterator>
#include <type_traits>

namespace strobe::gpu::vulkan {

VmaAllocationCreateInfo g_lookup[] = {
    VmaAllocationCreateInfo{
        // automatic
        .flags = 0,
        .usage = VMA_MEMORY_USAGE_AUTO,
        .requiredFlags = 0,
        .preferredFlags = 0,
        .memoryTypeBits = 0,
        .pool = 0,
        .pUserData = nullptr,
        .priority = 0.0f,
    },
    VmaAllocationCreateInfo{
        // device
        .flags = 0,
        .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
        .requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        .preferredFlags = 0,
        .memoryTypeBits = 0,
        .pool = VK_NULL_HANDLE,
        .pUserData = nullptr,
        .priority = 0.0f,
    },
    VmaAllocationCreateInfo{
        // mapped
        .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT |
                 VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,

        .usage = VMA_MEMORY_USAGE_AUTO,
        .requiredFlags = 0,
        .preferredFlags = VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
        .memoryTypeBits = 0,
        .pool = VK_NULL_HANDLE,
        .pUserData = nullptr,
        .priority = 0.0f,
    },
    VmaAllocationCreateInfo{
        // mapped_write_sequential
        .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT |
                 VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,

        .usage = VMA_MEMORY_USAGE_AUTO,
        .requiredFlags = 0,
        .preferredFlags = 0,
        .memoryTypeBits = 0,
        .pool = VK_NULL_HANDLE,
        .pUserData = nullptr,
        .priority = 0.0f,
    },
};

const VmaAllocationCreateInfo *
details::get_allocation_create_info(MemoryUsage usage) noexcept {
  using type = std::underlying_type_t<MemoryUsage>;
  const type index = static_cast<type>(usage);
  assert(static_cast<size_t>(index) < std::size(g_lookup));
  return &g_lookup[index];
}

} // namespace strobe::gpu::vulkan
