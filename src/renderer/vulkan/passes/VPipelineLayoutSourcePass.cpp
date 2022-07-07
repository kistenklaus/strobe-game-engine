#include "renderer/vulkan/passes/VPipelineLayoutSourcePass.hpp"

#include "logging/print.hpp"

namespace sge::vulkan {

VPipelineLayoutSourcePass::VPipelineLayoutSourcePass(VRendererBackend* renderer,
                                                     const std::string name)
    : RenderPass(renderer, name, false),
      m_pipelineLayout(m_vrenderer->createPipelineLayout()),
      m_pipelineLayoutSource(source<pipeline_layout>("pipelinelayout")) {
  registerSource(&m_pipelineLayoutSource);
  m_pipelineLayoutSource.set(&m_pipelineLayout);
}

void VPipelineLayoutSourcePass::dispose() {
  m_vrenderer->destroyPipelineLayout(m_pipelineLayout);
  m_pipelineLayoutSource.reset();
}

}  // namespace sge::vulkan
