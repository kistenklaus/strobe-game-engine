#include "renderer/vulkan/passes/VulkanRenderPassSourcePass.hpp"

namespace sge::vulkan {

VulkanRenderPassSourcePass::VulkanRenderPassSourcePass(
    VulkanRendererBackend* renderer, const std::string name,
    const VkFormat colorFormat)
    : RenderPass(renderer, name),
      m_renderPass(m_vrenderer->createRenderPass(colorFormat)),
      m_renderPassSource(registerSource<u32>("renderpass")) {
  setSourceResource(m_renderPassSource, &m_renderPass);
}

void VulkanRenderPassSourcePass::dispose() {
  setSourceResource<u32>(m_renderPassSource, nullptr);
  m_vrenderer->destroyRenderPass(m_renderPass);
}

}  // namespace sge::vulkan
