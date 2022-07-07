
#include "renderer/vulkan/passes/VResetCmdBuffPass.hpp"

namespace sge::vulkan {

VResetCmdBuffPass::VResetCmdBuffPass(VRendererBackend* renderer,
                                     const std::string name)
    : RenderPass(renderer, name),
      m_cmdbufferSink(registerSink<command_buffer>("cmdbuffer")),
      m_fenceSink(registerSink<fence>("fence")),
      m_cmdbufferSrc(registerSource<command_buffer>("cmdbuffer")),
      m_fenceSrc(registerSource<fence>("fence")) {}

void VResetCmdBuffPass::recreate() { m_seenFences.clear(); }

void VResetCmdBuffPass::execute() {
  const fence* p_fence = getSinkResource<fence>(m_fenceSink);
  if (m_seenFences.contains(*p_fence)) {
    m_vrenderer->waitForFence(*p_fence);
    m_vrenderer->resetFence(*p_fence);
  } else {
    m_seenFences.insert(*p_fence);
  }
  const command_buffer* p_cmdbuffer =
      getSinkResource<command_buffer>(m_cmdbufferSink);
  m_vrenderer->resetCommandBuffer(*p_cmdbuffer);
  setSourceResource(m_cmdbufferSrc, p_cmdbuffer);
  setSourceResource(m_fenceSrc, p_fence);
}

}  // namespace sge::vulkan
