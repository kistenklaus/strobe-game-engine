#include "renderer/vulkan/passes/VulkanAcquireQueuePass.hpp"

#include <exception>

#include "logging/print.hpp"

namespace sge::vulkan {

VulkanAcquireQueuePass::VulkanAcquireQueuePass(
    VulkanRendererBackend* renderer, VulkanQueueAcquireTarget queueType)
    : RenderPass(renderer),
      m_queueType(queueType),
      m_queueSource(registerSource<u32>()) {}

void VulkanAcquireQueuePass::execute() {
  print("VulkanAcquireQueuePass QUEUE-ID:");
  println(m_vrenderer->getAnyGraphicsQueue());
  setSourceResource(m_queueSource, [&]() -> u32 {
    switch (m_queueType) {
      case GRAPHICS_QUEUE:
        return m_vrenderer->getAnyGraphicsQueue();
      case TRANSFER_QUEUE:
        return m_vrenderer->getAnyTransferQueue();
      case COMPUTE_QUEUE:
        return m_vrenderer->getAnyComputeQueue();
    }
    throw std::runtime_error("invalid queue target");
  }());
}

}  // namespace sge::vulkan
