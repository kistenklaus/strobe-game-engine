#include "renderer/vulkan/passes/VResetCmdPoolPass.hpp"

namespace sge::vulkan {
VResetCmdPoolPass::VResetCmdPoolPass(VRendererBackend* renderer,
                                     const std::string name)
    : RenderPass(renderer, name),
      m_fenceSink(sink<fence>("fence", true)),
      m_poolSink(sink<command_pool>("cmdpool")),
      m_fenceSource(source<fence>("fence")),
      m_poolSource(source<command_pool>("cmdpool")) {
  registerSink(&m_fenceSink);
  registerSink(&m_poolSink);
  registerSource(&m_fenceSource);
  registerSource(&m_poolSource);
}

void VResetCmdPoolPass::recreate() { m_seenFences.clear(); }

void VResetCmdPoolPass::execute() {
  if (m_fenceSink) {
    if (m_seenFences.contains(*m_fenceSink)) {
      m_vrenderer->waitForFence(*m_fenceSink);
      m_vrenderer->resetFence(*m_fenceSink);
    } else {
      m_seenFences.insert(*m_fenceSink);
    }
  }
  m_vrenderer->resetCommandPool(*m_poolSink);
  m_poolSource.set(m_poolSink);
  m_fenceSource.set(m_fenceSink);
}

}  // namespace sge::vulkan
