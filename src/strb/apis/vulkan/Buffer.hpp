#pragma once
#include "strb/apis/vulkan/Device.hpp"
#include "strb/apis/vulkan/assert.hpp"
#include "strb/apis/vulkan/vulkan.lib.hpp"

namespace strb::vulkan {

class Buffer {
private:
  const Device *m_device;
  const uint64_t m_size;
  VkBuffer m_buffer;
  VkDeviceMemory m_memory;

public:
  // leaks vulkan api
  Buffer() : m_device(nullptr), m_size(0) {}
  Buffer(const Device &device, uint64_t size, VkBufferUsageFlags usage,
         VkMemoryPropertyFlags properties);

  void mapMemory(uint64_t offset, uint64_t size, void **content);
  void unmapMemory();
  void destroy();
  inline operator VkBuffer() const { return m_buffer; }
  inline uint64_t getSize() const { return m_size; }
};

} // namespace strb::vulkan
