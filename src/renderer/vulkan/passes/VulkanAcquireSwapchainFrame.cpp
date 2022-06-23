#include "renderer/vulkan/passes/VulkanAcquireSwapchainFrame.hpp"

namespace sge::vulkan {

VulkanAcquireSwapchainFrame::VulkanAcquireSwapchainFrame(
    VulkanRendererBackend* renderer)
    : RenderPass(renderer),
      m_signalSem(m_vrenderer->createSemaphore()),
      m_singalSemSrc(registerSource<u32>()) {}

void VulkanAcquireSwapchainFrame::execute() {
  m_vrenderer->acquireNextSwapchainFrame(m_signalSem);
  setSourceResource(m_singalSemSrc, &m_signalSem);
}

}  // namespace sge::vulkan
