#include "renderer/vulkan/passes/VPresentQueuePass.hpp"

#include "logging/print.hpp"
#include "renderer/RendergraphResourcesDeprecatedException.hpp"

namespace sge::vulkan {

VPresentQueuePass::VPresentQueuePass(VRendererBackend* renderer,
                                     const std::string name)
    : RenderPass(renderer, name),
      m_queueSink(sink<queue>("queue")),
      m_waitSemsSink(sink<std::vector<semaphore>>("wait")) {
  registerSink(&m_queueSink);
  registerSink(&m_waitSemsSink);
}

void VPresentQueuePass::execute() {
  const boolean deprecated =
      m_vrenderer->presentQueue(*m_queueSink, *m_waitSemsSink);
  if (deprecated) {
    throw RendergraphResourcesDeprecatedException();
  }
}

}  // namespace sge::vulkan
