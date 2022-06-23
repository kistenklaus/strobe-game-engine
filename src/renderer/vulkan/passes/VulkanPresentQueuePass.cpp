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

  m_vrenderer->presentQueue(getSinkResource<u32>(m_queueSink),
                            getSinkResource<std::vector<u32>>(m_waitSemsSink));
}

}  // namespace sge::vulkan
