#include "renderer/vulkan/passes/VBeginCmdBuffPass.hpp"

namespace sge::vulkan {

VBeginCmdBuffPass::VBeginCmdBuffPass(VRendererBackend* renderer,
                                     const std::string name)
    : RenderPass(renderer, name),
      m_cmdBufferSink(registerSink<command_buffer>("cmdbuffer")),
      m_cmdBufferSource(registerSource<command_buffer>("cmdbuffer")) {}

void VBeginCmdBuffPass::execute() {
  command_buffer* p_cmdBuff = getSinkResource<command_buffer>(m_cmdBufferSink);
  m_vrenderer->beginCommandBuffer(*p_cmdBuff);
  setSourceResource(m_cmdBufferSource, p_cmdBuff);
}

}  // namespace sge::vulkan
