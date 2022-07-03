#include "renderer/vulkan/passes/VulkanAllocateCmdBuffPass.hpp"

namespace sge::vulkan {

VulkanAllocateCmdBuffPass::VulkanAllocateCmdBuffPass(
    VulkanRendererBackend* renderer, const std::string name)
    : RenderPass(renderer, name),
      m_poolSink(registerSink<u32>("cmdpool")),
      m_bufferSource(registerSource<u32>("cmdbuffer")) {}

void VulkanAllocateCmdBuffPass::execute() {
  u32* pool = getSinkResource<u32>(m_poolSink);
  m_buffer = m_vrenderer->allocateCommandBuffers(*pool, 1)[0];
  setSourceResource(m_bufferSource, &m_buffer);
}

}  // namespace sge::vulkan
