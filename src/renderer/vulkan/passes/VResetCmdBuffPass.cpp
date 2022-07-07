
#include "renderer/vulkan/passes/VResetCmdBuffPass.hpp"

namespace sge::vulkan {

VResetCmdBuffPass::VResetCmdBuffPass(VRendererBackend* renderer,
                                     const std::string name)
    : RenderPass(renderer, name),
      m_cmdbufferSS(sinksource<command_buffer>("cmdbuffer")),
      m_fenceSS(sinksource<fence>("fence")) {
  registerSinkSource(&m_cmdbufferSS);
  registerSinkSource(&m_fenceSS);
}

void VResetCmdBuffPass::recreate() { m_seenFences.clear(); }

void VResetCmdBuffPass::execute() {
  if (m_seenFences.contains(*m_fenceSS)) {
    m_vrenderer->waitForFence(*m_fenceSS);
    m_vrenderer->resetFence(*m_fenceSS);
  } else {
    m_seenFences.insert(*m_fenceSS);
  }
  m_vrenderer->resetCommandBuffer(*m_cmdbufferSS);

  m_cmdbufferSS.forward();
  m_fenceSS.forward();
}

}  // namespace sge::vulkan
