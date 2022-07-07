#include "renderer/vulkan/passes/VAllocateCmdBuffPass.hpp"

namespace sge::vulkan {

VAllocateCmdBuffPass::VAllocateCmdBuffPass(VRendererBackend* renderer,
                                           const std::string name)
    : RenderPass(renderer, name),
      m_poolSink(sink<command_pool>("cmdpool")),
      m_bufferSource(source<command_buffer>("cmdbuffer")) {
  registerSink(&m_poolSink);
  registerSource(&m_bufferSource);
}

void VAllocateCmdBuffPass::execute() {
  m_buffer = m_vrenderer->allocateCommandBuffers(*m_poolSink, 1)[0];
  m_bufferSource.set(&m_buffer);
}

}  // namespace sge::vulkan
