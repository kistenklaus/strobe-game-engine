#include "renderer/vulkan/passes/VRenderPassSourcePass.hpp"

namespace sge::vulkan {

VRenderPassSourcePass::VRenderPassSourcePass(VRendererBackend* renderer,
                                             const std::string name,
                                             const VkFormat colorFormat)
    : RenderPass(renderer, name, false),
      m_renderPass(m_vrenderer->createRenderPass(colorFormat)),
      m_renderPassSource(registerSource<renderpass>("renderpass")) {
  setSourceResource(m_renderPassSource, &m_renderPass);
}

void VRenderPassSourcePass::dispose() {
  setSourceResource<renderpass>(m_renderPassSource, nullptr);
  m_vrenderer->destroyRenderPass(m_renderPass);
}

}  // namespace sge::vulkan
