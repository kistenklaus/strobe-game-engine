#include "renderer/vulkan/passes/VSubmitCmdBufferPass.hpp"

#include "logging/print.hpp"

namespace sge::vulkan {

VSubmitCmdBufferPass::VSubmitCmdBufferPass(VRendererBackend* renderer,
                                           const std::string name)
    : RenderPass(renderer, name),
      m_fenceSink(sink<fence>("fence")),
      m_queueSink(sink<queue>("queue")),
      m_commandBuffersSink(sink<command_buffer>("cmdbuffer")),
      m_waitSemaphoresSink(sink<std::vector<semaphore>>("wait")),
      m_signalSemaphoresSource(
          source<std::vector<semaphore>>("signal", &m_signalSemaphores)),
      m_signalSemaphores({m_vrenderer->createSemaphore()}) {
  registerSink(&m_fenceSink);
  registerSink(&m_queueSink);
  registerSink(&m_commandBuffersSink);
  registerSink(&m_waitSemaphoresSink);
  registerSource(&m_signalSemaphoresSource);
}

void VSubmitCmdBufferPass::execute() {
  std::optional<fence> fence = std::nullopt;
  if (m_fenceSink) {
    fence = *m_fenceSink;
  }

  m_vrenderer->submitCommandBuffers(*m_queueSink, {*m_commandBuffersSink},
                                    *m_waitSemaphoresSink, m_signalSemaphores,
                                    fence);
}

}  // namespace sge::vulkan
