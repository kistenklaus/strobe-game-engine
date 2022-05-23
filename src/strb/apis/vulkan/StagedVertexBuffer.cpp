#include "strb/apis/vulkan/StagedVertexBuffer.hpp"

namespace strb::vulkan {

StagedVertexBuffer::StagedVertexBuffer(const Device &device,
                                       uint32_t vertexCount,
                                       const VertexBindingLayout layout)
    : m_buffer(Buffer(device, vertexCount * layout.vertexSize,
                      VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                          VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)),
      m_stagingBuffer(Buffer(device, vertexCount * layout.vertexSize,
                             VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {}

void StagedVertexBuffer::mapStagedMemory(uint64_t offset, uint64_t size,
                                         void **content) {
  m_stagingBuffer.mapMemory(offset, size, content);
}
void StagedVertexBuffer::unmapStagedMemory() { m_stagingBuffer.unmapMemory(); }

void StagedVertexBuffer::bind(const CommandBuffer &commandBuffer) {
  VkBuffer buffers[] = {m_buffer};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
}

void StagedVertexBuffer::commit(CommandBuffer &commandBuffer) {
  VkBufferCopy copyRegion;
  copyRegion.srcOffset = 0;
  copyRegion.dstOffset = 0;
  copyRegion.size = m_buffer.getSize();
  vkCmdCopyBuffer(commandBuffer, m_stagingBuffer, m_buffer, 1, &copyRegion);
}

void StagedVertexBuffer::commit(CommandPool &commandPool, Queue &queue) {
  CommandBuffer commandBuffer = commandPool.allocate(1)[0];
  commandBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
  commit(commandBuffer);
  commandBuffer.end();
  queue.submit(commandBuffer, {}, {});
  queue.waitIdle();
  commandPool.free({commandBuffer});
}

void StagedVertexBuffer::destroy() {
  m_buffer.destroy();
  m_stagingBuffer.destroy();
}

} // namespace strb::vulkan
