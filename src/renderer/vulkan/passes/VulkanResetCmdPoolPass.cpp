#include "renderer/vulkan/passes/VulkanResetCmdPoolPass.hpp"

namespace sge::vulkan {
VulkanResetCmdPoolPass::VulkanResetCmdPoolPass(VRendererBackend* renderer,
                                               const std::string name)
    : RenderPass(renderer, name),
      m_fenceSink(registerSink<u32>("fence")),
      m_poolSink(registerSink<u32>("cmdpool")),
      m_fenceSource(registerSource<u32>("fence")),
      m_poolSource(registerSource<u32>("cmdpool")) {}

void VulkanResetCmdPoolPass::recreate() { m_seenFences.clear(); }

void VulkanResetCmdPoolPass::execute() {
  u32* p_fence = getSinkResource<u32>(m_fenceSink);
  if (p_fence != nullptr) {
    if (m_seenFences.contains(*p_fence)) {
      m_vrenderer->waitForFence(*p_fence);
      m_vrenderer->resetFence(*p_fence);
    } else {
      m_seenFences.insert(*p_fence);
    }
  }
  u32* p_pool = getSinkResource<u32>(m_poolSink);
  m_vrenderer->resetCommandPool(*p_pool);
  setSourceResource(m_poolSource, p_pool);
  setSourceResource(m_fenceSource, p_fence);
}

}  // namespace sge::vulkan
