#include "renderer/vulkan/passes/VCmdPoolMultiSrcPass.hpp"

namespace sge::vulkan {

VCmdPoolMultiSrcPass::VCmdPoolMultiSrcPass(VRendererBackend* renderer,
                                           const std::string name,
                                           const QueueFamilyType queueFamily,
                                           const u32 count)
    : RenderPass(renderer, name, false),
      m_poolsSource(registerSource<std::vector<u32>>("cmdpools")),
      m_pools([&]() {
        std::vector<command_pool> pools(count);
        for (u32 i = 0; i < count; i++) {
          pools[i] = m_vrenderer->createCommandPool(queueFamily);
        }
        return pools;
      }()) {
  setSourceResource(m_poolsSource, &m_pools);
}

void VCmdPoolMultiSrcPass::dispose() {
  for (const command_pool& pool : m_pools) {
    m_vrenderer->destroyCommandPool(pool);
  }
  setSourceResource<std::vector<u32>>(m_poolsSource, nullptr);
}

}  // namespace sge::vulkan
