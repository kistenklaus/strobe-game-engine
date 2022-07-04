#include "renderer/vulkan/passes/VulkanFenceMultiSrcPass.hpp"

namespace sge::vulkan {

VulkanFenceMultiSrcPass::VulkanFenceMultiSrcPass(
    VulkanRendererBackend* renderer, const std::string name, const u32 count)
    : RenderPass(renderer, name),
      m_fencesSource(registerSource<std::vector<u32>>("fences")),
      m_fences([&]() {
        std::vector<u32> fences(count);
        for (u32 i = 0; i < count; i++) {
          fences[i] = m_vrenderer->createFence();
        }
        return fences;
      }()) {
  setSourceResource(m_fencesSource, &m_fences);
}

void VulkanFenceMultiSrcPass::recreate() {
  for (u32 i = 0; i < m_fences.size(); i++) {
    m_vrenderer->destroyFence(m_fences[i]);
    m_fences[i] = m_vrenderer->createFence();
  }
}

void VulkanFenceMultiSrcPass::dispose() {
  setSourceResource<std::vector<u32>>(m_fencesSource, nullptr);
  for (const u32 fence : m_fences) {
    m_vrenderer->destroyFence(fence);
  }
}

}  // namespace sge::vulkan
