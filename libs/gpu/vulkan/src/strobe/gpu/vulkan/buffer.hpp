#pragma once

#include "strobe/gpu/vulkan/context/context.hpp"
#include "strobe/gpu/vulkan/memory_usage.hpp"

#include <vulkan/vulkan_core.h>

namespace strobe::gpu::vulkan {

struct Buffer {
  VkBuffer handle = VK_NULL_HANDLE;
  VmaAllocation allocation = VK_NULL_HANDLE;

  [[nodiscard]]
  explicit operator bool() const noexcept {
    return handle != VK_NULL_HANDLE;
  }
};

struct BufferInfo {
  VkDeviceSize size = 0;
  VkBufferUsageFlags usage = 0;
  MemoryUsage memory_usage = MemoryUsage::automatic;
};

Buffer create_buffer(Context *context, const BufferInfo &info);
void destroy_buffer(Context *context, Buffer buffer) noexcept;
void *map_buffer(Context *context, Buffer buffer);
void unmap_buffer(Context *context, Buffer buffer) noexcept;
void flush_buffer(Context *context, Buffer buffer, size_t offset = 0,
                  size_t size = VK_WHOLE_SIZE);
void invalidate_buffer(Context *context, Buffer buffer, size_t offset = 0,
                       size_t size = VK_WHOLE_SIZE);
void *get_persistantly_mapped_buffer_ptr(Context *context,
                                         Buffer buffer) noexcept;
VkDeviceAddress get_buffer_device_address(Context *context,
                                          Buffer buffer) noexcept;

} // namespace strobe::gpu::vulkan
