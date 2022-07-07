
#include "renderer/vulkan/passes/VCmdBuffMSrcPass.hpp"

namespace sge::vulkan {

VCmdBuffMSrcPass::VCmdBuffMSrcPass(VRendererBackend* renderer,
                                   const std::string name, const u32 count)
    : RenderPass(renderer, name, false),
      m_poolSink(sink<command_pool>("cmdpool")),
      m_cmdbufferSrc(source<std::vector<command_buffer>>("cmdbuffers")),
      m_count(count) {
  registerSink(&m_poolSink);
  registerSource(&m_cmdbufferSrc);
}

void VCmdBuffMSrcPass::create() {
  m_buffers = m_vrenderer->allocateCommandBuffers(*m_poolSink, m_count);
  m_cmdbufferSrc.set(&m_buffers);
}

void VCmdBuffMSrcPass::dispose() {
  m_vrenderer->freeCommandBuffers(m_buffers);
  m_cmdbufferSrc.reset();
}

}  // namespace sge::vulkan
