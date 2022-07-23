#include "renderer/vulkan/bindbales/VMesh.hpp"

namespace sge::vulkan {

VMesh::VMesh(VRendererBackend* renderer) {
  m_vertexBuffer = renderer->createVertexBuffer(sizeof(float) * 3 * 4);

  float vertexData[] = {
      0.5f,  -0.5f, 0.0f,  //
      0.5f,  0.5f,  0.0f,  //
      -0.5f, 0.5f,  0.0f,  //
      -0.5f, -0.5f, 0.0f   //
  };
  renderer->uploadToBuffer(m_vertexBuffer, vertexData);
  m_indexBuffer = renderer->createIndexBuffer(sizeof(u32) * 3);
  u32 indicies[] = {0, 1, 2};
  renderer->uploadToBuffer(m_indexBuffer, indicies);
}

void VMesh::bind(VRendererBackend* renderer, renderpass renderpass,
                 command_buffer commandBuffer, pipeline pipeline) {
  renderer->bindVertexBuffer(m_vertexBuffer, commandBuffer);
  renderer->bindIndexBuffer(m_indexBuffer, commandBuffer);
}

}  // namespace sge::vulkan
