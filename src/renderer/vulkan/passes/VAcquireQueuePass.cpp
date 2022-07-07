#include "renderer/vulkan/passes/VAcquireQueuePass.hpp"

#include <exception>

#include "logging/print.hpp"

namespace sge::vulkan {

VAcquireQueuePass::VAcquireQueuePass(VRendererBackend* renderer,
                                     const std::string name,
                                     VulkanQueueAcquireTarget queueType)
    : RenderPass(renderer, name),
      m_queueType(queueType),
      m_queueSource(registerSource<queue>("queue")) {}

void VAcquireQueuePass::execute() {
  m_queueHandle = [&]() -> queue {
    switch (m_queueType) {
      case GRAPHICS_QUEUE:
        return m_vrenderer->getAnyGraphicsQueue();
      case TRANSFER_QUEUE:
        return m_vrenderer->getAnyTransferQueue();
      case COMPUTE_QUEUE:
        return m_vrenderer->getAnyComputeQueue();
    }
    throw std::runtime_error("invalid queue target");
  }();
  setSourceResource(m_queueSource, &m_queueHandle);
}

}  // namespace sge::vulkan
