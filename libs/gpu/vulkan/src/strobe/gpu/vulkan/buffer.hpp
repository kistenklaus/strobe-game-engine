#pragma once

#include "strobe/gpu/vulkan/context/context.hpp"
#include "strobe/gpu/vulkan/memory_requirements.hpp"

#include <vulkan/vulkan_core.h>

namespace strobe::gpu::vulkan {
// fwd
struct Memory;

struct Buffer {
  VkBuffer handle = VK_NULL_HANDLE;

  [[nodiscard]]
  explicit operator bool() const noexcept {
    return handle != VK_NULL_HANDLE;
  }
};

struct BufferInfo {
  VkDeviceSize size = 0;
  VkBufferUsageFlags2 usage = 0;
};

Buffer create_buffer(Context *context, const BufferInfo &info);

void destroy_buffer(Context *context, Buffer buffer) noexcept;

MemoryRequirements get_buffer_memory_requirements(Context *context,
                                                  Buffer buffer);

VkDeviceAddress get_buffer_device_address(Context *context, Buffer buffer);

void bind_buffer_memory(Context *context, const Memory &memory, Buffer buffer,
                        VkDeviceSize offset);

} // namespace strobe::gpu::vulkan
