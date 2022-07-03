#include "renderer/vulkan/passes/VulkanPipelineLayoutSourcePass.hpp"

namespace sge::vulkan {

VulkanPipelineLayoutSourcePass::VulkanPipelineLayoutSourcePass(
    VulkanRendererBackend* renderer, const std::string name)
    : RenderPass(renderer, name),
      m_pipelineLayout(m_vrenderer->createPipelineLayout()),
      m_pipelineLayoutSource(registerSource<u32>("pipelinelayout")) {
  setSourceResource(m_pipelineLayoutSource, &m_pipelineLayout);
}

VulkanPipelineLayoutSourcePass::~VulkanPipelineLayoutSourcePass() {
  m_vrenderer->destroyPipelineLayout(m_pipelineLayout);
  setSourceResource<u32>(m_pipelineLayoutSource, nullptr);
}

}  // namespace sge::vulkan
