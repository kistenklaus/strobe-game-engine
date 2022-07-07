#include "renderer/vulkan/passes/VSubmitCmdBufferPass.hpp"

#include "logging/print.hpp"

namespace sge::vulkan {

VSubmitCmdBufferPass::VSubmitCmdBufferPass(VRendererBackend* renderer,
                                           const std::string name)
    : RenderPass(renderer, name),
      m_fenceSink(registerSink<fence>("fence")),
      m_queueSink(registerSink<queue>("queue")),
      m_commandBuffersSink(registerSink<command_buffer>("cmdbuffer")),
      m_waitSemaphoresSink(registerSink<std::vector<semaphore>>("wait")),
      m_signalSemaphoresSource(
          registerSource<std::vector<semaphore>>("signal")),
      m_signalSemaphores({m_vrenderer->createSemaphore()}) {
  setSourceResource(m_signalSemaphoresSource, &m_signalSemaphores);
}

void VSubmitCmdBufferPass::execute() {
  const queue* p_queue = getSinkResource<queue>(m_queueSink);

  const command_buffer* p_commandBuffer =
      getSinkResource<command_buffer>(m_commandBuffersSink);

  const std::vector<semaphore>* p_waitSemaphores =
      getSinkResource<std::vector<semaphore>>(m_waitSemaphoresSink);

  const fence* p_fence = getSinkResource<fence>(m_fenceSink);
  std::optional<fence> fence = std::nullopt;
  if (p_fence != nullptr) {
    fence = *p_fence;
  }

  m_vrenderer->submitCommandBuffers(*p_queue, {*p_commandBuffer},
                                    *p_waitSemaphores, m_signalSemaphores,
                                    fence);
}

}  // namespace sge::vulkan
