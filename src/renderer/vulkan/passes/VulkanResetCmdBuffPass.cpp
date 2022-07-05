
#include "renderer/vulkan/passes/VulkanResetCmdBuffPass.hpp"

namespace sge::vulkan {

VulkanResetCmdBuffPass::VulkanResetCmdBuffPass(VulkanRendererBackend* renderer,
                                               const std::string name)
    : RenderPass(renderer, name),
      m_cmdbufferSink(registerSink<u32>("cmdbuffer")),
      m_fenceSink(registerSink<u32>("fence")),
      m_cmdbufferSrc(registerSource<u32>("cmdbuffer")),
      m_fenceSrc(registerSource<u32>("fence")) {}

void VulkanResetCmdBuffPass::recreate() { m_seenFences.clear(); }

void VulkanResetCmdBuffPass::execute() {
  const u32* p_fence = getSinkResource<u32>(m_fenceSink);
  if (m_seenFences.contains(*p_fence)) {
    m_vrenderer->waitForFence(*p_fence);
    m_vrenderer->resetFence(*p_fence);
  } else {
    m_seenFences.insert(*p_fence);
  }
  const u32* p_cmdbuffer = getSinkResource<u32>(m_cmdbufferSink);
  m_vrenderer->resetCommandBuffer(*p_cmdbuffer);
  setSourceResource(m_cmdbufferSrc, p_cmdbuffer);
  setSourceResource(m_fenceSrc, p_fence);
}

}  // namespace sge::vulkan
