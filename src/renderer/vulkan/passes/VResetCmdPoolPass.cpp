#include "renderer/vulkan/passes/VResetCmdPoolPass.hpp"

namespace sge::vulkan {
VResetCmdPoolPass::VResetCmdPoolPass(VRendererBackend* renderer,
                                     const std::string name)
    : RenderPass(renderer, name),
      m_fenceSink(registerSink<fence>("fence")),
      m_poolSink(registerSink<command_pool>("cmdpool")),
      m_fenceSource(registerSource<fence>("fence")),
      m_poolSource(registerSource<command_pool>("cmdpool")) {}

void VResetCmdPoolPass::recreate() { m_seenFences.clear(); }

void VResetCmdPoolPass::execute() {
  fence* p_fence = getSinkResource<fence>(m_fenceSink);
  if (p_fence != nullptr) {
    if (m_seenFences.contains(*p_fence)) {
      m_vrenderer->waitForFence(*p_fence);
      m_vrenderer->resetFence(*p_fence);
    } else {
      m_seenFences.insert(*p_fence);
    }
  }
  command_pool* p_pool = getSinkResource<command_pool>(m_poolSink);
  m_vrenderer->resetCommandPool(*p_pool);
  setSourceResource(m_poolSource, p_pool);
  setSourceResource(m_fenceSource, p_fence);
}

}  // namespace sge::vulkan
