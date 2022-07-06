#include "renderer/vulkan/passes/VulkanPresentQueuePass.hpp"

#include "logging/print.hpp"
#include "renderer/RendergraphResourcesDeprecatedException.hpp"

namespace sge::vulkan {

VulkanPresentQueuePass::VulkanPresentQueuePass(VRendererBackend* renderer,
                                               const std::string name)
    : RenderPass(renderer, name),
      m_queueSink(registerSink<u32>("queue")),
      m_waitSemsSink(registerSink<std::vector<u32>>("wait")) {}

void VulkanPresentQueuePass::execute() {
  u32* p_queue = getSinkResource<u32>(m_queueSink);

  std::vector<u32>* p_semaphores =
      getSinkResource<std::vector<u32>>(m_waitSemsSink);

  const boolean deprecated = m_vrenderer->presentQueue(*p_queue, *p_semaphores);
  if (deprecated) {
    throw RendergraphResourcesDeprecatedException();
  }
}

}  // namespace sge::vulkan
