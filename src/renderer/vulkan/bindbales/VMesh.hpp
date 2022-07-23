#pragma once

#include "renderer/vulkan/VBindable.hpp"

namespace sge::vulkan {

class VMesh : public VBindable {
 public:
  VMesh(VRendererBackend* renderer);
  void bind(VRendererBackend* renderer, renderpass renderpass,
            command_buffer commandBuffer, pipeline pipeline) override;

 private:
  vertex_buffer m_vertexBuffer;
  index_buffer m_indexBuffer;
};

}  // namespace sge::vulkan
