#include "renderer/vulkan/passes/VulkanPresentQueuePass.hpp"

#include "logging/print.hpp"

namespace sge::vulkan {

VulkanPresentQueuePass::VulkanPresentQueuePass(VulkanRendererBackend* renderer)
    : RenderPass(renderer),
      m_queueSink(registerSink<u32>()),
      m_waitSemsSink(registerSink<std::vector<u32>>()) {}

void VulkanPresentQueuePass::execute() {
  print("VulkanPresnetQueuePass QUEUE-ID:");
  println(getSinkResource<u32>(m_queueSink));

  u32* p_queue = getSinkResource<u32>(m_queueSink);
  std::vector<u32>* p_semaphores =
      getSinkResource<std::vector<u32>>(m_waitSemsSink);

  m_vrenderer->presentQueue(*p_queue, *p_semaphores);
}

}  // namespace sge::vulkan
