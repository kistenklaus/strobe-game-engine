#include "renderer/vulkan/passes/VulkanCmdPoolSrcPass.hpp"

namespace sge::vulkan {

VulkanCmdPoolSrcPass::VulkanCmdPoolSrcPass(VulkanRendererBackend* renderer,
                                           const std::string name,
                                           QueueFamilyType queueFamily)
    : RenderPass(renderer, name, false),
      m_pool(m_vrenderer->createCommandPool(queueFamily)),
      m_poolSource(registerSource<u32>("cmdpool")) {
  //
  setSourceResource<u32>(m_poolSource, &m_pool);
}

}  // namespace sge::vulkan
