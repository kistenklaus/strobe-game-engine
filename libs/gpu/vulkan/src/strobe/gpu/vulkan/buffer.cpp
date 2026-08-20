#include "strobe/gpu/vulkan/buffer.hpp"
#include "strobe/gpu/vulkan/memory.hpp"

#include <stdexcept>
#include <vulkan/vulkan_core.h>

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
      .usage = 0,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount = 0,
      .pQueueFamilyIndices = nullptr,
  };

  Buffer buffer{
      .handle = VK_NULL_HANDLE,
  };

  {
    ZoneScopedN("vkCreateBuffer");
    const VkResult result =
        vkCreateBuffer(context->device(), &bufferInfo, context->driver_alloc(),
                       &buffer.handle);

    if (result != VK_SUCCESS) {
      throw std::runtime_error{"Failed to create Vulkan buffer"};
    }
  }

  return buffer;
}

void destroy_buffer(Context *context, Buffer buffer) noexcept {
  assert(context != nullptr);
  assert(buffer);
  ZoneScopedN("vkDestroyBuffer");
  vkDestroyBuffer(context->device(), buffer.handle, context->driver_alloc());
}

MemoryRequirements get_buffer_memory_requirements(Context *context,
                                                  Buffer buffer) {
  VkBufferMemoryRequirementsInfo2 info{
      .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2,
      .pNext = nullptr,
      .buffer = buffer.handle,
  };
  VkMemoryDedicatedRequirements dedicatedReq{
      .sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS,
      .pNext = nullptr,
      .prefersDedicatedAllocation = false,
      .requiresDedicatedAllocation = false,
  };

  VkMemoryRequirements2 req2{
      .sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2,
      .pNext = &dedicatedReq,
      .memoryRequirements = {},
  };
  {
    ZoneScopedN("vkGetBufferMemoryRequirements2");
    vkGetBufferMemoryRequirements2(context->device(), &info, &req2);
  }

  return MemoryRequirements{
      .size = req2.memoryRequirements.size,
      .alignment = req2.memoryRequirements.alignment,
      .memoryTypeBits = req2.memoryRequirements.memoryTypeBits,
      .prefersDedicated =
          static_cast<bool>(dedicatedReq.prefersDedicatedAllocation),
      .requiresDedicated =
          static_cast<bool>(dedicatedReq.requiresDedicatedAllocation),
  };
}

VkDeviceAddress get_buffer_device_address(Context *context, Buffer buffer) {
  VkBufferDeviceAddressInfo info{
      .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
      .pNext = nullptr,
      .buffer = buffer.handle,
  };
  ZoneScopedN("vkGetBufferDeviceAddress");
  return vkGetBufferDeviceAddress(context->device(), &info);
}

void bind_buffer_memory(Context *context, const Memory &memory, Buffer buffer,
                        VkDeviceSize offset) {
  assert(context != nullptr);
  assert(memory);
  assert(buffer);
  {
    ZoneScopedN("vmaBindBufferMemory2");
    VkResult result = vmaBindBufferMemory2(context->vma(), memory.handle,
                                           offset, buffer.handle, nullptr);
    if (result != VK_SUCCESS) {
      throw std::runtime_error("Failed to bind buffer memory");
    }
  }
}

} // namespace strobe::gpu::vulkan
