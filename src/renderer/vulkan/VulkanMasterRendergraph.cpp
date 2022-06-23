#include "renderer/vulkan/VulkanMasterRendergraph.hpp"

#include <vulkan/vulkan.h>

#include "logging/print.hpp"
#include "renderer/passes/WrapWithVectorPass.hpp"
#include "renderer/vulkan/VulkanAssertion.hpp"
#include "renderer/vulkan/passes/VulkanAcquireQueuePass.hpp"
#include "renderer/vulkan/passes/VulkanAcquireSwapchainFrame.hpp"
#include "renderer/vulkan/passes/VulkanPresentQueuePass.hpp"
#include "renderer/vulkan/passes/VulkanSubmitCmdBufferPass.hpp"

namespace sge::vulkan {

VulkanMasterRendergraph::VulkanMasterRendergraph(
    VulkanRendererBackend* renderer)
    : Rendergraph(reinterpret_cast<RendererBackend*>(renderer)),
      m_imageAvaiableSemaphoreId(m_vrenderer->createSemaphore()),
      m_renderingDoneSemaphoreId(m_vrenderer->createSemaphore()),
      m_acquireSwapchainFramePass(
          createPass<VulkanAcquireSwapchainFrame>(renderer)),
      m_acquireQueuePass(
          createPass<VulkanAcquireQueuePass>(renderer, GRAPHICS_QUEUE)),
      m_submitPass(createPass<VulkanSubmitCmdBufferPass>(renderer)),
      m_wrapWithVectorPass(createPass<WrapWithVectorPass<u32>>(renderer)),
      m_presentQueuePass(createPass<VulkanPresentQueuePass>(renderer)) {
  markPassAsRoot(m_presentQueuePass);

  addLinkage(
      m_acquireSwapchainFramePass,
      getPassById<VulkanAcquireSwapchainFrame>(m_acquireSwapchainFramePass)
          .getSignalSemaphoreSource(),
      m_wrapWithVectorPass,
      getPassById<WrapWithVectorPass<u32>>(m_wrapWithVectorPass)
          .getSingleSink());
  addLinkage(m_wrapWithVectorPass,
             getPassById<WrapWithVectorPass<u32>>(m_wrapWithVectorPass)
                 .getVectorSource(),
             m_presentQueuePass,
             getPassById<VulkanPresentQueuePass>(m_presentQueuePass)
                 .getWaitSemaphoresSink());
  addLinkage(
      m_acquireQueuePass,
      getPassById<VulkanAcquireQueuePass>(m_acquireQueuePass).getQueueSource(),
      m_presentQueuePass,
      getPassById<VulkanPresentQueuePass>(m_presentQueuePass).getQueueSink());
}

}  // namespace sge::vulkan
