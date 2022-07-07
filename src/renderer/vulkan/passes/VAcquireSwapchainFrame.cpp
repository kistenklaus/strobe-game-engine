#include "renderer/vulkan/passes/VAcquireSwapchainFrame.hpp"

#include "renderer/RendergraphResourcesDeprecatedException.hpp"

namespace sge::vulkan {

VAcquireSwapchainFrame::VAcquireSwapchainFrame(VRendererBackend* renderer,
                                               const std::string name)
    : RenderPass(renderer, name),
      m_singalSemaphoresSource(
          registerSource<std::vector<semaphore>>("signal")),
      m_swapchainImageViewSource(registerSource<imageview>("imageview")),
      m_signalSemphores({m_vrenderer->createSemaphore()}),
      m_swapchainFrameIndexSource(registerSource<u32>("frameindex")) {
  setSourceResource(m_singalSemaphoresSource, &m_signalSemphores);
  setSourceResource(m_swapchainImageViewSource, &m_imageView);
  setSourceResource(m_swapchainFrameIndexSource, &m_frameIndex);
}

void VAcquireSwapchainFrame::execute() {
  const boolean deprecated =
      m_vrenderer->acquireNextSwapchainFrame(m_signalSemphores[0]);
  if (deprecated) {
    throw RendergraphResourcesDeprecatedException();
  }

  m_imageView = m_vrenderer->getCurrentSwapchainImageView();
  m_frameIndex = m_vrenderer->getCurrentSwapchainFrameIndex();
}

}  // namespace sge::vulkan
