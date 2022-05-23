#include "strb/apis/vulkan/Buffer.hpp"

namespace strb::vulkan {

Buffer::Buffer(const Device &device, uint64_t size, VkBufferUsageFlags usage,
               VkMemoryPropertyFlags properties)
    : m_device(&device), m_size(size) {
  VkBufferCreateInfo createInfo;
  createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  createInfo.pNext = nullptr;
  createInfo.flags = 0;
  createInfo.size = size;
  createInfo.usage = usage;
  createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  createInfo.queueFamilyIndexCount = 0;
  createInfo.pQueueFamilyIndices = nullptr;

  VkResult result = vkCreateBuffer(*m_device, &createInfo, nullptr, &m_buffer);
  ASSERT_VKRESULT(result);
  VkMemoryRequirements memReq;
  vkGetBufferMemoryRequirements(*m_device, m_buffer, &memReq);
  VkMemoryAllocateInfo allocInfo;
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.pNext = nullptr;
  allocInfo.allocationSize = memReq.size;
  allocInfo.memoryTypeIndex =
      m_device->findMemoryTypeIndex(memReq.memoryTypeBits, properties);

  result = vkAllocateMemory(*m_device, &allocInfo, nullptr, &m_memory);
  ASSERT_VKRESULT(result);
  vkBindBufferMemory(*m_device, m_buffer, m_memory, 0);
}

void Buffer::mapMemory(uint64_t offset, uint64_t size, void **content) {
  VkResult result = vkMapMemory(*m_device, m_memory, offset, size, 0, content);
  ASSERT_VKRESULT(result);
}

void Buffer::unmapMemory() { vkUnmapMemory(*m_device, m_memory); }
void Buffer::destroy() {
  vkDestroyBuffer(*m_device, m_buffer, nullptr);
  vkFreeMemory(*m_device, m_memory, nullptr);
}
} // namespace strb::vulkan
