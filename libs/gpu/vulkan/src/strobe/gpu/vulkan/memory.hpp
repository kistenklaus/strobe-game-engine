#pragma once

#include "strobe/gpu/vulkan/buffer.hpp"
#include "strobe/gpu/vulkan/context/context.hpp"
#include "strobe/gpu/vulkan/image.hpp"
#include "strobe/gpu/vulkan/memory_usage.hpp"
#include <tuple>
#include <vulkan/vulkan_core.h>

namespace strobe::gpu::vulkan {

struct Memory {
  VmaAllocation handle = VK_NULL_HANDLE;
  [[nodiscard]]

  explicit operator bool() const noexcept {
    return handle != VK_NULL_HANDLE;
  }
};

Memory allocate_memory(Context *context, MemoryRequirements requirements,
                       MemoryUsage usage, bool alias);

Memory allocate_dedicated_memory(Context *context,
                                 MemoryRequirements requirements,
                                 MemoryUsage usage, Buffer buffer);

Memory allocate_dedicated_memory(Context *context,
                                 MemoryRequirements requirements,
                                 MemoryUsage usage, Image image);

void free_memory(Context *context, Memory memory);

// a buffer created like this must be freed with free_allocated_buffer!
std::tuple<Memory, Buffer, VkDeviceSize>
allocate_buffer(Context *context, const BufferInfo &info,
                MemoryUsage memoryUsage);
// requires buffer and memory to be allocated with allocate_buffer
void free_allocated_buffer(Context *context, Memory memory, Buffer buffer);
// // requires buffer and memory to be allocated with allocate_buffer
// // only destroy the buffer
void destroy_allocated_buffer(Context *context, Buffer buffer);

std::tuple<Memory, Image, VkDeviceSize> allocate_image(Context *context,
                                                       const ImageInfo &info,
                                                       MemoryUsage memoryUsage);

void free_allocated_image(Context *context, Memory memory,
                          Image image) noexcept;

void destroy_allocated_image(Context *context, Image image) noexcept;

void *map_memory(Context *context, Memory memory);

void unmap_memory(Context *context, Memory memory) noexcept;

void flush_memory(Context *context, Memory memory, VkDeviceSize offset,
                  VkDeviceSize size = VK_WHOLE_SIZE);
void invalidate_memory(Context *context, Memory memory, VkDeviceSize offset,
                       VkDeviceSize size = VK_WHOLE_SIZE);

} // namespace strobe::gpu::vulkan
