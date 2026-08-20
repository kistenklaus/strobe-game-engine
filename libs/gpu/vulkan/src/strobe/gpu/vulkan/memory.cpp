#include "strobe/gpu/vulkan/memory.hpp"
#include "strobe/gpu/vulkan/context/context.hpp"
#include "strobe/gpu/vulkan/memory_usage.hpp"
#include <stdexcept>
#include <vulkan/vulkan_core.h>

namespace strobe::gpu::vulkan {

Memory allocate_memory(Context *context, MemoryRequirements requirements,
                       MemoryUsage usage, bool alias) {
  assert(requirements.requiresDedicated == false);

  VkMemoryRequirements req{
      .size = requirements.size,
      .alignment = requirements.alignment,
      .memoryTypeBits = requirements.memoryTypeBits,
  };
  VmaAllocationCreateInfo allocInfo =
      details::get_allocation_create_info(requirements, usage, alias);

  Memory memory{};
  {
    ZoneScopedN("vmaAllocateMemory");
    VkResult result = vmaAllocateMemory(context->vma(), &req, &allocInfo,
                                        &memory.handle, nullptr);
    if (result != VK_SUCCESS) {
      throw std::runtime_error("Failed to allocate memory");
    }
  }
  return memory;
}

void free_memory(Context *context, Memory memory) {
  assert(context);
  assert(memory);
  ZoneScopedN("vmaFreeMemory");
  vmaFreeMemory(context->vma(), memory.handle);
}

std::pair<Memory, Buffer> allocate_buffer(Context *context,
                                          const BufferInfo &info,
                                          MemoryUsage memoryUsage) {
  const VkBufferUsageFlags2CreateInfo usageInfo{
      .sType = VK_STRUCTURE_TYPE_BUFFER_USAGE_FLAGS_2_CREATE_INFO,
      .pNext = nullptr,
      .usage = info.usage,
  };
  const VkBufferCreateInfo bufferInfo{
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .pNext = &usageInfo,
      .flags = 0,
      .size = info.size,
      .usage = 0,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount = 0,
      .pQueueFamilyIndices = nullptr,
  };
  Buffer buffer{};
  Memory memory{};
  {
    ZoneScopedN("vmaCreateBuffer");
    VkResult result =
        vmaCreateBuffer(context->vma(), &bufferInfo,
                        details::get_auto_allocation_create_info(memoryUsage),
                        &buffer.handle, &memory.handle, nullptr);
    if (result != VK_SUCCESS) {
      throw std::runtime_error("Failed to create and allocate buffer");
    }
  }
  return std::make_pair(memory, buffer);
}

void free_allocated_buffer(Context *context, Memory memory, Buffer buffer) {
  assert(context);
  assert(buffer);
  assert(memory);
  ZoneScopedN("vmaDestroyBuffer");
  vmaDestroyBuffer(context->vma(), buffer.handle, memory.handle);
}

void destroy_allocated_buffer(Context *context, Buffer buffer) {
  assert(context);
  assert(buffer);
  ZoneScopedN("vmaDestroyBuffer");
  vmaDestroyBuffer(context->vma(), buffer.handle, VK_NULL_HANDLE);
}

void *map_memory(Context *context, Memory memory) {
  void *mapped;
  {
    ZoneScopedN("vmaMapMemory");
    VkResult result = vmaMapMemory(context->vma(), memory.handle, &mapped);
    if (result != VK_SUCCESS) {
      throw std::runtime_error("Failed to map memory");
    }
  }
  return mapped;
}

void unmap_memory(Context *context, Memory memory) noexcept {
  ZoneScopedN("vmaUnmapMemory");
  vmaUnmapMemory(context->vma(), memory.handle);
}

void flush_memory(Context *context, Memory memory, VkDeviceSize offset,
                  VkDeviceSize size) {
  ZoneScopedN("vmaFlushAllocation");
  VkResult result =
      vmaFlushAllocation(context->vma(), memory.handle, offset, size);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to flush memory");
  }
}

void invalidate_memory(Context *context, Memory memory, VkDeviceSize offset,
                       VkDeviceSize size) {
  ZoneScopedN("vmaInvalidateAllocation");
  VkResult result =
      vmaInvalidateAllocation(context->vma(), memory.handle, offset, size);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to invalidate memory");
  }
}

} // namespace strobe::gpu::vulkan
