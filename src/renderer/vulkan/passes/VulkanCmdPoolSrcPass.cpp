#include "renderer/vulkan/passes/VulkanCmdPoolSrcPass.hpp"

namespace sge::vulkan {

VulkanCmdPoolSrcPass::VulkanCmdPoolSrcPass(VulkanRendererBackend* renderer,
                                           QueueFamilyType queueFamily)
    : RenderPass(renderer),
      m_pool(m_vrenderer->createCommandPool(queueFamily)),
      m_poolSource(registerSource<u32>()) {
  //
  setSourceResource<u32>(m_poolSource, &m_pool);
}

}  // namespace sge::vulkan
