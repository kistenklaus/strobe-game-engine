#include "renderer/vulkan/passes/VBeginCmdBuffPass.hpp"

namespace sge::vulkan {

VBeginCmdBuffPass::VBeginCmdBuffPass(VRendererBackend* renderer,
                                     const std::string name)
    : RenderPass(renderer, name),
      m_cmdBufferSink(sink<command_buffer>("cmdbuffer")),
      m_cmdBufferSource(source<command_buffer>("cmdbuffer")) {
  registerSink(&m_cmdBufferSink);
  registerSource(&m_cmdBufferSource);
}

void VBeginCmdBuffPass::execute() {
  m_vrenderer->beginCommandBuffer(*m_cmdBufferSink);
  m_cmdBufferSource.set(m_cmdBufferSink);
}

}  // namespace sge::vulkan
