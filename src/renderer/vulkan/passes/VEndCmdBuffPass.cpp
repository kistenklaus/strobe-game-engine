#include "renderer/vulkan/passes/VEndCmdBuffPass.hpp"

namespace sge::vulkan {

VEndCmdBuffPass::VEndCmdBuffPass(VRendererBackend* renderer,
                                 const std::string name)
    : RenderPass(renderer, name),
      m_cmdBuffSink(registerSink<command_buffer>("cmdbuffer")),
      m_cmdBuffSource(registerSource<command_buffer>("cmdbuffer")) {}

void VEndCmdBuffPass::execute() {
  command_buffer* p_cmdBuff = getSinkResource<command_buffer>(m_cmdBuffSink);
  m_vrenderer->endCommandBuffer(*p_cmdBuff);
  setSourceResource(m_cmdBuffSource, p_cmdBuff);
}

}  // namespace sge::vulkan
