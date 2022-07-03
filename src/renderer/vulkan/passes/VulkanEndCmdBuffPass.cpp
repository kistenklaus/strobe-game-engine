#include "renderer/vulkan/passes/VulkanEndCmdBuffPass.hpp"

namespace sge::vulkan {

VulkanEndCmdBuffPass::VulkanEndCmdBuffPass(VulkanRendererBackend* renderer,
                                           const std::string name)
    : RenderPass(renderer, name),
      m_cmdBuffSink(registerSink<u32>("cmdbuffer")),
      m_cmdBuffSource(registerSource<u32>("cmdbuffer")) {}

void VulkanEndCmdBuffPass::execute() {
  u32* p_cmdBuff = getSinkResource<u32>(m_cmdBuffSink);
  m_vrenderer->endCommandBuffer(*p_cmdBuff);
  setSourceResource(m_cmdBuffSource, p_cmdBuff);
}

}  // namespace sge::vulkan
