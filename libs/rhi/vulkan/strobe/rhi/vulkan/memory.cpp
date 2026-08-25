#include "strobe/rhi/vulkan/memory.hpp"
#include "strobe/rhi/vulkan/context/context.hpp"
#include "strobe/rhi/vulkan/memory_usage.hpp"
#include <stdexcept>
#include <tuple>
#include <vulkan/vulkan_core.h>

namespace strobe::rhi::vulkan {

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
  ZoneScopedN("vmaAllocateMemory");
  VkResult result = vmaAllocateMemory(context->vma(), &req, &allocInfo,
                                      &memory.handle, nullptr);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to allocate memory");
  }
  return memory;
}

Memory allocate_dedicated_memory(Context *context,
                                 MemoryRequirements requirements,
                                 MemoryUsage usage, Buffer buffer) {
  VkMemoryRequirements req{
      .size = requirements.size,
      .alignment = requirements.alignment,
      .memoryTypeBits = requirements.memoryTypeBits,
  };
  VmaAllocationCreateInfo allocInfo =
      details::get_allocation_create_info(requirements, usage, false);
  VkMemoryDedicatedAllocateInfo dedicatedInfo{
      .sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO,
      .pNext = nullptr,
      .image = VK_NULL_HANDLE,
      .buffer = buffer.handle,
  };

  Memory memory{};
  ZoneScopedN("vmaAllocateDedicatedMemory");
  VkResult result =
      vmaAllocateDedicatedMemory(context->vma(), &req, &allocInfo,
                                 &dedicatedInfo, &memory.handle, nullptr);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to allocate dedicated buffer memory");
  }
  return memory;
}

Memory allocate_dedicated_memory(Context *context,
                                 MemoryRequirements requirements,
                                 MemoryUsage usage, Image image) {
  VkMemoryRequirements req{
      .size = requirements.size,
      .alignment = requirements.alignment,
      .memoryTypeBits = requirements.memoryTypeBits,
  };
  VmaAllocationCreateInfo allocInfo =
      details::get_allocation_create_info(requirements, usage, false);
  VkMemoryDedicatedAllocateInfo dedicatedInfo{
      .sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO,
      .pNext = nullptr,
      .image = image.handle,
      .buffer = VK_NULL_HANDLE,
  };
  Memory memory{};
  ZoneScopedN("vmaAllocateDedicatedMemory");
  VkResult result =
      vmaAllocateDedicatedMemory(context->vma(), &req, &allocInfo,
                                 &dedicatedInfo, &memory.handle, nullptr);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to allocate dedicated image memory");
  }
  return memory;
}

void free_memory(Context *context, Memory memory) {
  assert(context);
  assert(memory);
  ZoneScopedN("vmaFreeMemory");
  vmaFreeMemory(context->vma(), memory.handle);
}

std::tuple<Memory, Buffer, VkDeviceSize>
allocate_buffer(Context *context, const BufferInfo &info,
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
  VmaAllocationInfo allocInfo{};
  Buffer buffer{};
  Memory memory{};
  {
    ZoneScopedN("vmaCreateBuffer");
    VkResult result =
        vmaCreateBuffer(context->vma(), &bufferInfo,
                        details::get_auto_allocation_create_info(memoryUsage),
                        &buffer.handle, &memory.handle, &allocInfo);
    if (result != VK_SUCCESS) {
      throw std::runtime_error("Failed to create and allocate buffer");
    }
  }
  return std::make_tuple(memory, buffer, allocInfo.size);
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

std::tuple<Memory, Image, VkDeviceSize>
allocate_image(Context *context, const ImageInfo &info,
               MemoryUsage memoryUsage) {
  assert(context != nullptr);
  assert(info.format != VK_FORMAT_UNDEFINED);
  assert(info.extent.width != 0);
  assert(info.extent.height != 0);
  assert(info.extent.depth != 0);
  assert(info.mip_levels != 0);
  assert(info.array_layers != 0);
  assert(info.usage != 0);

  const VkImageCreateInfo imageInfo{
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .pNext = nullptr,
      .flags = info.flags,
      .imageType = info.type,
      .format = info.format,
      .extent = info.extent,
      .mipLevels = info.mip_levels,
      .arrayLayers = info.array_layers,
      .samples = info.samples,
      .tiling = info.tiling,
      .usage = info.usage,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount = 0,
      .pQueueFamilyIndices = nullptr,
      .initialLayout = info.initial_layout,
  };
  const VmaAllocationCreateInfo *pVmaInfo =
      details::get_auto_allocation_create_info(memoryUsage);

  Memory memory{};
  Image image{};
  VmaAllocationInfo allocInfo{};
  {
    ZoneScopedN("vmaCreateImage");
    const VkResult result =
        vmaCreateImage(context->vma(), &imageInfo, pVmaInfo, &image.handle,
                       &memory.handle, &allocInfo);
    if (result != VK_SUCCESS) {
      throw std::runtime_error{"Failed to create Vulkan image"};
    }
  }
  return std::make_tuple(memory, image, allocInfo.size);
}

void free_allocated_image(Context *context, Memory memory,
                          Image image) noexcept {
  assert(context);
  assert(image);
  assert(memory);
  ZoneScopedN("vmaDestroyImage");
  vmaDestroyImage(context->vma(), image.handle, memory.handle);
}

void destroy_allocated_image(Context *context, Image image) noexcept {
  assert(context);
  assert(image);
  ZoneScopedN("vmaDestroyImage");
  vmaDestroyImage(context->vma(), image.handle, VK_NULL_HANDLE);
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

} // namespace strobe::rhi::vulkan
