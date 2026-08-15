#include "strobe/gpu/vulkan/buffer.hpp"

#include <stdexcept>

namespace strobe::gpu::vulkan {

Buffer create_buffer(Context *context, const BufferInfo &info) {
  assert(context != nullptr);
  assert(info.size != 0);
  assert(info.usage != 0);

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

      // Ignored because VkBufferUsageFlags2CreateInfo is in pNext.
      .usage = 0,

      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount = 0,
      .pQueueFamilyIndices = nullptr,
  };

  Buffer buffer{
      .handle = VK_NULL_HANDLE,
      .allocation = VK_NULL_HANDLE,
  };

  {
    ZoneScopedN("vmaCreateBuffer");
    const VkResult result =
        vmaCreateBuffer(context->vma(), &bufferInfo,
                        details::get_allocation_create_info(info.memory_usage),
                        &buffer.handle, &buffer.allocation, nullptr);

    if (result != VK_SUCCESS) {
      throw std::runtime_error{"Failed to create Vulkan buffer"};
    }
  }

  return buffer;
}

void destroy_buffer(Context *context, Buffer buffer) noexcept {
  assert(context != nullptr);
  assert(buffer);
  ZoneScopedN("vmaDestroyBuffer")
      vmaDestroyBuffer(context->vma(), buffer.handle, buffer.allocation);
}
void *map_buffer(Context *context, Buffer buffer) {
  assert(context != nullptr);
  assert(buffer);
  assert(buffer.allocation != VK_NULL_HANDLE);
  void *data;
  {
    ZoneScopedN("vmaMapMemory");
    VkResult result = vmaMapMemory(context->vma(), buffer.allocation, &data);
    if (result != VK_SUCCESS) {
      throw std::runtime_error("Failed to map buffer");
    }
  }
  return data;
}
void unmap_buffer(Context *context, Buffer buffer) noexcept {
  assert(context != nullptr);
  assert(buffer);
  assert(buffer.allocation != VK_NULL_HANDLE);
  ZoneScopedN("vmaUnmapMemory");
  vmaUnmapMemory(context->vma(), buffer.allocation);
}

void flush_buffer(Context *context, Buffer buffer, size_t offset, size_t size) {
  assert(context != nullptr);
  assert(buffer);
  assert(buffer.allocation != VK_NULL_HANDLE);
  {
    ZoneScopedN("vmaFlushAllocation");
    VkResult result =
        vmaFlushAllocation(context->vma(), buffer.allocation, offset, size);
    if (result != VK_SUCCESS) {
      throw std::runtime_error("Failed to flush buffer range");
    }
  }
}
void invalidate_buffer(Context *context, Buffer buffer, size_t offset,
                       size_t size) {
  assert(context != nullptr);
  assert(buffer);
  assert(buffer.allocation != VK_NULL_HANDLE);
  {
    ZoneScopedN("vmaInvalidateAllocation");
    VkResult result = vmaInvalidateAllocation(context->vma(), buffer.allocation,
                                              offset, size);
    if (result != VK_SUCCESS) {
      throw std::runtime_error("Failed to invalidate buffer range");
    }
  }
}
void *get_persistantly_mapped_buffer_ptr(Context *context,
                                         Buffer buffer) noexcept {
  assert(context != nullptr);
  assert(buffer);
  assert(buffer.allocation != VK_NULL_HANDLE);
  VmaAllocationInfo allocInfo{};
  {
    ZoneScopedN("vmaGetAllocationInfo");
    vmaGetAllocationInfo(context->vma(), buffer.allocation, &allocInfo);
  }
  assert(allocInfo.pMappedData != nullptr);
  return allocInfo.pMappedData;
}
VkDeviceAddress get_buffer_device_address(Context *context,
                                          Buffer buffer) noexcept {
  assert(context != nullptr);
  assert(context->properties().deviceAddress);
  assert(buffer);

  const VkBufferDeviceAddressInfo info{
      .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
      .pNext = nullptr,
      .buffer = buffer.handle,
  };

  ZoneScopedN("vkGetBufferDeviceAddress");
  return vkGetBufferDeviceAddress(context->device(), &info);
}

} // namespace strobe::gpu::vulkan
