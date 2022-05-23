#pragma once
#include "strb/apis/vulkan/CommandPool.hpp"
#include "strb/apis/vulkan/Device.hpp"
#include "strb/apis/vulkan/VertexBuffer.hpp"

namespace strb::vulkan {

class StagedVertexBuffer {
private:
  Buffer m_buffer;
  Buffer m_stagingBuffer;

public:
  StagedVertexBuffer(const Device &device, uint32_t vertexCount,
                     const VertexBindingLayout layout);
  void mapStagedMemory(uint64_t offset, uint64_t size, void **content);
  void bind(const CommandBuffer &commandBuffer);
  inline void mapStagedMemory(void **content) {
    mapStagedMemory(0, m_buffer.getSize(), content);
  }
  void unmapStagedMemory();
  void commit(CommandBuffer &commandBuffer);
  void commit(CommandPool &commandPool, Queue &queue);
  void destroy();
};

} // namespace strb::vulkan
