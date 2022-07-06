
#include "renderer/vulkan/passes/VulkanCmdBuffMSrcPass.hpp"

namespace sge::vulkan {

VulkanCmdBuffMSrcPass::VulkanCmdBuffMSrcPass(VRendererBackend* renderer,
                                             const std::string name,
                                             const u32 count)
    : RenderPass(renderer, name, false),
      m_poolSink(registerSink<u32>("cmdpool")),
      m_cmdbufferSrc(registerSource<std::vector<u32>>("cmdbuffers")),
      m_count(count) {}

void VulkanCmdBuffMSrcPass::create() {
  const u32* p_pool = getSinkResource<u32>(m_poolSink);
  m_buffers = m_vrenderer->allocateCommandBuffers(*p_pool, m_count);
  setSourceResource(m_cmdbufferSrc, &m_buffers);
}

void VulkanCmdBuffMSrcPass::dispose() {
  const u32* p_pool = getSinkResource<u32>(m_poolSink);
  m_vrenderer->freeCommandBuffers(m_buffers);
  setSourceResource<std::vector<u32>>(m_cmdbufferSrc, nullptr);
}

}  // namespace sge::vulkan
