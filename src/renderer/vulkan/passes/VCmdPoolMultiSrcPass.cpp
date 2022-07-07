#include "renderer/vulkan/passes/VCmdPoolMultiSrcPass.hpp"

namespace sge::vulkan {

VCmdPoolMultiSrcPass::VCmdPoolMultiSrcPass(VRendererBackend* renderer,
                                           const std::string name,
                                           const QueueFamilyType queueFamily,
                                           const u32 count)
    : RenderPass(renderer, name, false),
      m_poolsSource(source<std::vector<command_pool>>("cmdpools", &m_pools)),
      m_pools([&]() {
        std::vector<command_pool> pools(count);
        for (u32 i = 0; i < count; i++) {
          pools[i] = m_vrenderer->createCommandPool(queueFamily);
        }
        return pools;
      }()) {
  registerSource(&m_poolsSource);
}

void VCmdPoolMultiSrcPass::dispose() {
  for (const command_pool& pool : m_pools) {
    m_vrenderer->destroyCommandPool(pool);
  }
  m_poolsSource.reset();
}

}  // namespace sge::vulkan
