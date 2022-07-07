#include "renderer/vulkan/passes/VEndCmdBuffPass.hpp"

namespace sge::vulkan {

VEndCmdBuffPass::VEndCmdBuffPass(VRendererBackend* renderer,
                                 const std::string name)
    : RenderPass(renderer, name),
      m_cmdBuffSink(sink<command_buffer>("cmdbuffer")),
      m_cmdBuffSource(source<command_buffer>("cmdbuffer")) {
  registerSink(&m_cmdBuffSink);
  registerSource(&m_cmdBuffSource);
}

void VEndCmdBuffPass::execute() {
  m_vrenderer->endCommandBuffer(*m_cmdBuffSink);
  m_cmdBuffSource.set(m_cmdBuffSink);
}

}  // namespace sge::vulkan
