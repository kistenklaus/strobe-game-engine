#include "renderer/vulkan/passes/VPresentQueuePass.hpp"

#include "logging/print.hpp"
#include "renderer/RendergraphResourcesDeprecatedException.hpp"

namespace sge::vulkan {

VPresentQueuePass::VPresentQueuePass(VRendererBackend* renderer,
                                     const std::string name)
    : RenderPass(renderer, name),
      m_queueSink(registerSink<queue>("queue")),
      m_waitSemsSink(registerSink<std::vector<semaphore>>("wait")) {}

void VPresentQueuePass::execute() {
  queue* p_queue = getSinkResource<queue>(m_queueSink);

  std::vector<semaphore>* p_semaphores =
      getSinkResource<std::vector<semaphore>>(m_waitSemsSink);

  const boolean deprecated = m_vrenderer->presentQueue(*p_queue, *p_semaphores);
  if (deprecated) {
    throw RendergraphResourcesDeprecatedException();
  }
}

}  // namespace sge::vulkan
