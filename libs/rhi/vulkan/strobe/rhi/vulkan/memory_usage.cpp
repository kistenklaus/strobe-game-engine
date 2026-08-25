#include "strobe/rhi/vulkan/memory_usage.hpp"
#include "strobe/rhi/vulkan/context/context.hpp"
#include "strobe/rhi/vulkan/memory_requirements.hpp"
#include <cassert>
#include <iterator>
#include <type_traits>
#include <vulkan/vulkan_core.h>

namespace strobe::rhi::vulkan {

VmaAllocationCreateInfo g_auto_lookup[] = {
    VmaAllocationCreateInfo{
        // automatic
        .flags = VMA_ALLOCATION_CREATE_DONT_BIND_BIT,
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
        .flags = VMA_ALLOCATION_CREATE_DONT_BIND_BIT,
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
        .flags = VMA_ALLOCATION_CREATE_DONT_BIND_BIT |
                 VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,

        .usage = VMA_MEMORY_USAGE_AUTO,
        .requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        .preferredFlags = VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
        .memoryTypeBits = 0,
        .pool = VK_NULL_HANDLE,
        .pUserData = nullptr,
        .priority = 0.0f,
    },
    VmaAllocationCreateInfo{
        // mapped_write_sequential
        .flags = VMA_ALLOCATION_CREATE_DONT_BIND_BIT |
                 VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,

        .usage = VMA_MEMORY_USAGE_AUTO,
        .requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        .preferredFlags = 0,
        .memoryTypeBits = 0,
        .pool = VK_NULL_HANDLE,
        .pUserData = nullptr,
        .priority = 0.0f,
    },
    VmaAllocationCreateInfo{
        // mapped_incoherent
        .flags = VMA_ALLOCATION_CREATE_DONT_BIND_BIT |
                 VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO,
        .requiredFlags = 0,
        .preferredFlags = VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
        .memoryTypeBits = 0,
        .pool = VK_NULL_HANDLE,
        .pUserData = nullptr,
        .priority = 0.0f,
    },
};

const VmaAllocationCreateInfo *
details::get_auto_allocation_create_info(MemoryUsage usage) noexcept {
  using type = std::underlying_type_t<MemoryUsage>;
  const type index = static_cast<type>(usage);
  assert(static_cast<size_t>(index) < std::size(g_auto_lookup));
  return &g_auto_lookup[index];
}

const VmaAllocationCreateInfo g_lookup[] = {
    VmaAllocationCreateInfo{
        // automatic: best generic GPU memory
        .flags = 0,
        .usage = VMA_MEMORY_USAGE_UNKNOWN,
        .requiredFlags = 0,
        .preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        .memoryTypeBits = 0,
        .pool = VK_NULL_HANDLE,
        .pUserData = nullptr,
        .priority = 0.5f,
    },
    VmaAllocationCreateInfo{
        // device: must be device-local
        .flags = 0,
        .usage = VMA_MEMORY_USAGE_UNKNOWN,
        .requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        .preferredFlags = 0,
        .memoryTypeBits = 0,
        .pool = VK_NULL_HANDLE,
        .pUserData = nullptr,
        .priority = 0.5f,
    },
    VmaAllocationCreateInfo{
        // mapped: CPU reads/random access
        .flags = 0,
        .usage = VMA_MEMORY_USAGE_UNKNOWN,
        .requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        .preferredFlags = VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
        .memoryTypeBits = 0,
        .pool = VK_NULL_HANDLE,
        .pUserData = nullptr,
        .priority = 0.5f,
    },
    VmaAllocationCreateInfo{
        // mapped_write_sequential
        .flags = 0,
        .usage = VMA_MEMORY_USAGE_UNKNOWN,
        .requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        .preferredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        .memoryTypeBits = 0,
        .pool = VK_NULL_HANDLE,
        .pUserData = nullptr,
        .priority = 0.5f,
    },
    VmaAllocationCreateInfo{
        // mapped: CPU reads/random access
        .flags = 0,
        .usage = VMA_MEMORY_USAGE_UNKNOWN,
        .requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
        .preferredFlags = VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
        .memoryTypeBits = 0,
        .pool = VK_NULL_HANDLE,
        .pUserData = nullptr,
        .priority = 0.5f,
    },
};

const VmaAllocationCreateInfo
details::get_allocation_create_info(const MemoryRequirements &requirements,
                                    MemoryUsage usage, bool alias) noexcept {
  using type = std::underlying_type_t<MemoryUsage>;
  const type index = static_cast<type>(usage);
  assert(static_cast<size_t>(index) < std::size(g_auto_lookup));
  VmaAllocationCreateInfo info = g_lookup[index];
  info.memoryTypeBits = requirements.memoryTypeBits;
  if (alias) {
    info.flags |= VMA_ALLOCATION_CREATE_CAN_ALIAS_BIT;
  }
  return info;
}

} // namespace strobe::rhi::vulkan
