#include "strb/apis/vulkan/VertexBuffer.hpp"
#include <cstring>

namespace strb::vulkan {

VertexBuffer::VertexBuffer(const Device &device, uint32_t vertexCount,
                           const VertexBindingLayout layout)
    : VertexBuffer(device, vertexCount, layout,
                   VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                       VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {}

VertexBuffer::VertexBuffer(const Device &device, uint32_t vertexCount,
                           const VertexBindingLayout layout,
                           const VkBufferUsageFlags usage,
                           const VkMemoryPropertyFlags properties)
    : m_device(&device), m_layout(layout),
      m_buffer(
          Buffer(device, vertexCount * layout.vertexSize, usage, properties)) {}

void VertexBuffer::bind(const CommandBuffer &commandBuffer) {
  VkBuffer buffers[] = {m_buffer};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
}

void VertexBuffer::mapMemory(void **content) {
  m_buffer.mapMemory(0, m_buffer.getSize(), content);
}

void VertexBuffer::unmapMemory() { m_buffer.unmapMemory(); }

void VertexBuffer::destroy() { m_buffer.destroy(); }

} // namespace strb::vulkan
