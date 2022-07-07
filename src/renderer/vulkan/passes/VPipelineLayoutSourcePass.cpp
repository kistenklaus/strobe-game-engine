#include "renderer/vulkan/passes/VPipelineLayoutSourcePass.hpp"

#include "logging/print.hpp"

namespace sge::vulkan {

VPipelineLayoutSourcePass::VPipelineLayoutSourcePass(VRendererBackend* renderer,
                                                     const std::string name)
    : RenderPass(renderer, name, false),
      m_pipelineLayout(m_vrenderer->createPipelineLayout()),
      m_pipelineLayoutSource(
          registerSource<pipeline_layout>("pipelinelayout")) {
  setSourceResource(m_pipelineLayoutSource, &m_pipelineLayout);
}

void VPipelineLayoutSourcePass::dispose() {
  m_vrenderer->destroyPipelineLayout(m_pipelineLayout);
  setSourceResource<pipeline_layout>(m_pipelineLayoutSource, nullptr);
}

}  // namespace sge::vulkan
