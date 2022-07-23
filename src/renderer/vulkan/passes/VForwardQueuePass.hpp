#pragma once

#include "renderer/vulkan/VRenderQueuePass.hpp"

namespace sge::vulkan {

class VForwardQueuePass : public VRenderQueuePass {
 public:
  VForwardQueuePass(VRendererBackend* renderer, const std::string name);

  void drawInstance(VRenderable& drawJob, pipeline pipeline) override;
  void beforeEach() override;
  void afterEach() override;
};
}  // namespace sge::vulkan
