#include "renderer/vulkan/passes/VulkanBeginCmdBuffPass.hpp"

namespace sge::vulkan {

VulkanBeginCmdBuffPass::VulkanBeginCmdBuffPass(VRendererBackend* renderer,
                                               const std::string name)
    : RenderPass(renderer, name),
      m_cmdBufferSink(registerSink<u32>("cmdbuffer")),
      m_cmdBufferSource(registerSource<u32>("cmdbuffer")) {}

void VulkanBeginCmdBuffPass::execute() {
  u32* p_cmdBuff = getSinkResource<u32>(m_cmdBufferSink);
  m_vrenderer->beginCommandBuffer(*p_cmdBuff);
  setSourceResource(m_cmdBufferSource, p_cmdBuff);
}

}  // namespace sge::vulkan
