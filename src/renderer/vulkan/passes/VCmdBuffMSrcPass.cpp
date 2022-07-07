
#include "renderer/vulkan/passes/VCmdBuffMSrcPass.hpp"

namespace sge::vulkan {

VCmdBuffMSrcPass::VCmdBuffMSrcPass(VRendererBackend* renderer,
                                   const std::string name, const u32 count)
    : RenderPass(renderer, name, false),
      m_poolSink(registerSink<command_pool>("cmdpool")),
      m_cmdbufferSrc(registerSource<std::vector<command_buffer>>("cmdbuffers")),
      m_count(count) {}

void VCmdBuffMSrcPass::create() {
  const command_pool* p_pool = getSinkResource<command_pool>(m_poolSink);
  m_buffers = m_vrenderer->allocateCommandBuffers(*p_pool, m_count);
  setSourceResource(m_cmdbufferSrc, &m_buffers);
}

void VCmdBuffMSrcPass::dispose() {
  const command_pool* p_pool = getSinkResource<command_pool>(m_poolSink);
  m_vrenderer->freeCommandBuffers(m_buffers);
  setSourceResource<std::vector<command_buffer>>(m_cmdbufferSrc, nullptr);
}

}  // namespace sge::vulkan
