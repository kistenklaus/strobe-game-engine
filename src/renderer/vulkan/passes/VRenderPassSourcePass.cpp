#include "renderer/vulkan/passes/VRenderPassSourcePass.hpp"

namespace sge::vulkan {

VRenderPassSourcePass::VRenderPassSourcePass(VRendererBackend* renderer,
                                             const std::string name,
                                             const VkFormat colorFormat)
    : RenderPass(renderer, name, false),
      m_renderPass(m_vrenderer->createRenderPass(colorFormat)),
      m_renderPassSource(source<renderpass>("renderpass")) {
  registerSource(&m_renderPassSource);
  m_renderPassSource.set(&m_renderPass);
}

void VRenderPassSourcePass::dispose() {
  m_renderPassSource.reset();
  m_vrenderer->destroyRenderPass(m_renderPass);
}

}  // namespace sge::vulkan
