#pragma once
#include "renderer/vulkan/VRendererBackend.hpp"

namespace sge::vulkan {

class VBindable {
 public:
  ~VBindable() = default;
  virtual void bind(VRendererBackend* renderer, renderpass renderpass,
                    command_buffer commandBuffer, pipeline pipeline) = 0;
};

}  // namespace sge::vulkan
