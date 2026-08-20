#pragma once

#include "strobe/gpu/vulkan/buffer.hpp"
#include "strobe/gpu/vulkan/context/context.hpp"
#include "strobe/gpu/vulkan/memory_usage.hpp"
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

void free_memory(Context *context, Memory memory);

// a buffer created like this must be freed with free_allocated_buffer!
std::pair<Memory, Buffer>
allocate_buffer(Context *context, const BufferInfo &info, MemoryUsage usage);
// requires buffer and memory to be allocated with allocate_buffer
void free_allocated_buffer(Context *context, Memory memory, Buffer buffer);
// requires buffer and memory to be allocated with allocate_buffer
// only destroy the buffer
void destroy_allocated_buffer(Context *context, Buffer buffer);

void *map_memory(Context *context, Memory memory);

void unmap_memory(Context *context, Memory memory) noexcept;

void flush_memory(Context *context, Memory memory, VkDeviceSize offset, VkDeviceSize size = VK_WHOLE_SIZE);
void invalidate_memory(Context *context, Memory memory, VkDeviceSize offset,
                       VkDeviceSize size = VK_WHOLE_SIZE);

} // namespace strobe::gpu::vulkan
