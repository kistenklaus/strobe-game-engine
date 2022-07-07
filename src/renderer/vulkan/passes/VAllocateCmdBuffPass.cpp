#include "renderer/vulkan/passes/VAllocateCmdBuffPass.hpp"

namespace sge::vulkan {

VAllocateCmdBuffPass::VAllocateCmdBuffPass(VRendererBackend* renderer,
                                           const std::string name)
    : RenderPass(renderer, name),
      m_poolSink(registerSink<command_pool>("cmdpool")),
      m_bufferSource(registerSource<command_buffer>("cmdbuffer")) {}

void VAllocateCmdBuffPass::execute() {
  command_pool* pool = getSinkResource<command_pool>(m_poolSink);
  m_buffer = m_vrenderer->allocateCommandBuffers(*pool, 1)[0];
  setSourceResource(m_bufferSource, &m_buffer);
}

}  // namespace sge::vulkan
