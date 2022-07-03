#include "renderer/vulkan/passes/VulkanSubmitCmdBufferPass.hpp"

#include "logging/print.hpp"

namespace sge::vulkan {

VulkanSubmitCmdBufferPass::VulkanSubmitCmdBufferPass(
    VulkanRendererBackend* renderer, const std::string name)
    : RenderPass(renderer, name),
      m_queueSink(registerSink<u32>("queue")),
      m_commandBuffersSink(registerSink<std::vector<u32>>("cmdbuffer")),
      m_waitSemaphoresSink(registerSink<std::vector<u32>>("wait")),
      m_signalSemaphoresSource(registerSource<std::vector<u32>>("signal")),
      m_signalSemaphores({m_vrenderer->createSemaphore()}) {
  setSourceResource(m_signalSemaphoresSource, &m_signalSemaphores);
}

void VulkanSubmitCmdBufferPass::execute() {
  const u32* p_queue = getSinkResource<u32>(m_queueSink);
  const std::vector<u32>* p_commandBuffers =
      getSinkResource<std::vector<u32>>(m_commandBuffersSink);
  const std::vector<u32>* p_waitSemaphores =
      getSinkResource<std::vector<u32>>(m_waitSemaphoresSink);

  u32 fence = m_vrenderer->createFence();
  m_vrenderer->submitCommandBuffers(*p_queue, *p_commandBuffers,
                                    *p_waitSemaphores, m_signalSemaphores,
                                    fence);
  m_vrenderer->waitForFence(fence);
  m_vrenderer->destroyFence(fence);
}

}  // namespace sge::vulkan
