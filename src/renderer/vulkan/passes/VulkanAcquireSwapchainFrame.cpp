#include "renderer/vulkan/passes/VulkanAcquireSwapchainFrame.hpp"

#include "renderer/RendergraphResourcesDeprecatedException.hpp"

namespace sge::vulkan {

VulkanAcquireSwapchainFrame::VulkanAcquireSwapchainFrame(
    VulkanRendererBackend* renderer, const std::string name)
    : RenderPass(renderer, name),
      m_singalSemaphoresSource(registerSource<std::vector<u32>>("signal")),
      m_swapchainImageViewSource(registerSource<u32>("imageview")),
      m_signalSemphores({m_vrenderer->createSemaphore()}),
      m_swapchainFrameIndexSource(registerSource<u32>("frameindex")) {
  setSourceResource(m_singalSemaphoresSource, &m_signalSemphores);
  setSourceResource(m_swapchainImageViewSource, &m_imageView);
  setSourceResource(m_swapchainFrameIndexSource, &m_frameIndex);
}

void VulkanAcquireSwapchainFrame::execute() {
  const boolean deprecated =
      m_vrenderer->acquireNextSwapchainFrame(m_signalSemphores[0]);
  if (deprecated) {
    throw RendergraphResourcesDeprecatedException();
  }

  m_imageView = m_vrenderer->getCurrentSwapchainImageView();
  m_frameIndex = m_vrenderer->getCurrentSwapchainFrameIndex();
}

}  // namespace sge::vulkan
