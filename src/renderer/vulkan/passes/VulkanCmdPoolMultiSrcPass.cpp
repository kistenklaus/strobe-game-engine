#include "renderer/vulkan/passes/VulkanCmdPoolMultiSrcPass.hpp"

namespace sge::vulkan {

VulkanCmdPoolMultiSrcPass::VulkanCmdPoolMultiSrcPass(
    VulkanRendererBackend* renderer, const std::string name,
    const QueueFamilyType queueFamily, const u32 count)
    : RenderPass(renderer, name, false),
      m_poolsSource(registerSource<std::vector<u32>>("cmdpools")),
      m_pools([&]() {
        std::vector<u32> pools(count);
        for (u32 i = 0; i < count; i++) {
          pools[i] = m_vrenderer->createCommandPool(queueFamily);
        }
        return pools;
      }()) {
  setSourceResource(m_poolsSource, &m_pools);
}

void VulkanCmdPoolMultiSrcPass::dispose() {
  for (u32 i = 0; i < m_pools.size(); i++) {
    m_vrenderer->destroyCommandPool(m_pools[i]);
  }
  setSourceResource<std::vector<u32>>(m_poolsSource, nullptr);
}

}  // namespace sge::vulkan
