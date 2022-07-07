#pragma once
#include "renderer/RenderPass.hpp"
#include "renderer/vulkan/VRendererBackend.hpp"

namespace sge::vulkan {

class VPipelineLayoutSourcePass : public RenderPass {
 public:
  explicit VPipelineLayoutSourcePass(VRendererBackend* renderer,
                                     const std::string name);

  void dispose() override;

 private:
  pipeline_layout m_pipelineLayout;
  source<pipeline_layout> m_pipelineLayoutSource;
};

}  // namespace sge::vulkan
