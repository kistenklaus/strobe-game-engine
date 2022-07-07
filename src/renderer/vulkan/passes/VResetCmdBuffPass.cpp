
#include "renderer/vulkan/passes/VResetCmdBuffPass.hpp"

namespace sge::vulkan {

VResetCmdBuffPass::VResetCmdBuffPass(VRendererBackend* renderer,
                                     const std::string name)
    : RenderPass(renderer, name),
      m_cmdbufferSink(sink<command_buffer>("cmdbuffer")),
      m_fenceSink(sink<fence>("fence")),
      m_cmdbufferSrc(source<command_buffer>("cmdbuffer")),
      m_fenceSrc(source<fence>("fence")) {
  registerSink(&m_cmdbufferSink);
  registerSink(&m_fenceSink);
  registerSource(&m_cmdbufferSrc);
  registerSource(&m_fenceSrc);
}

void VResetCmdBuffPass::recreate() { m_seenFences.clear(); }

void VResetCmdBuffPass::execute() {
  if (m_seenFences.contains(*m_fenceSink)) {
    m_vrenderer->waitForFence(*m_fenceSink);
    m_vrenderer->resetFence(*m_fenceSink);
  } else {
    m_seenFences.insert(*m_fenceSink);
  }
  m_vrenderer->resetCommandBuffer(*m_cmdbufferSink);

  m_cmdbufferSrc.set(m_cmdbufferSink);
  m_fenceSrc.set(m_fenceSink);
}

}  // namespace sge::vulkan
