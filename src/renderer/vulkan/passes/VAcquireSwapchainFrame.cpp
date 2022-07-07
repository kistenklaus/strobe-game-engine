#include "renderer/vulkan/passes/VAcquireSwapchainFrame.hpp"

#include "renderer/RendergraphResourcesDeprecatedException.hpp"

namespace sge::vulkan {

VAcquireSwapchainFrame::VAcquireSwapchainFrame(VRendererBackend* renderer,
                                               const std::string name)
    : RenderPass(renderer, name),
      // m_singalSemaphoresSource(
      //     registerSource<std::vector<semaphore>>("signal")),
      m_singalSemaphoresSource(
          source<std::vector<semaphore>>("signal", &m_signalSemphores)),
      m_swapchainImageViewSource(source<imageview>("imageview", &m_imageView)),
      m_signalSemphores({m_vrenderer->createSemaphore()}),
      m_swapchainFrameIndexSource(source<u32>("frameindex", &m_frameIndex)) {
  registerSource(&m_singalSemaphoresSource);
  registerSource(&m_swapchainImageViewSource);
  registerSource(&m_swapchainFrameIndexSource);
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
