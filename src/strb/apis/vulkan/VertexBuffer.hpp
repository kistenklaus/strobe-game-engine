#pragma once
#include "strb/apis/vulkan/Buffer.hpp"
#include "strb/apis/vulkan/Device.hpp"
#include "strb/apis/vulkan/VertexInputLayout.hpp"
#include "strb/apis/vulkan/assert.hpp"
#include "strb/apis/vulkan/vulkan.lib.hpp"
#include "strb/stl.hpp"

namespace strb::vulkan {

class VertexBuffer {
protected:
  const Device *m_device;
  const VertexBindingLayout m_layout;
  Buffer m_buffer;

public:
  VertexBuffer()
      : m_device(nullptr), m_layout(VertexBindingLayout(-1, {}, false)),
        m_buffer({}) {}

  VertexBuffer(const Device &device, uint32_t vertexCount,
               const VertexBindingLayout layout);

protected:
  VertexBuffer(const Device &device, uint32_t vertexCount,
               const VertexBindingLayout layout, const VkBufferUsageFlags usage,
               const VkMemoryPropertyFlags properties);

public:
  virtual ~VertexBuffer() = default;
  VertexBuffer(VertexBuffer &&) = default;
  VertexBuffer(VertexBuffer &) = default;

  virtual void bind(const CommandBuffer &commandBuffer);
  virtual void mapMemory(void **content);
  virtual void unmapMemory();
  virtual void destroy();
};

} // namespace strb::vulkan
