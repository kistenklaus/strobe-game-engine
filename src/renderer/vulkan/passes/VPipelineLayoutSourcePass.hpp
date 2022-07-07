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
  const pipeline_layout m_pipelineLayout;
  const u32 m_pipelineLayoutSource;
};

}  // namespace sge::vulkan
