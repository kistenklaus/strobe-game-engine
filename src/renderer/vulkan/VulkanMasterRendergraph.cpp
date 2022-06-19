#include "renderer/vulkan/VulkanMasterRendergraph.hpp"

#include <vulkan/vulkan.h>

#include "logging/print.hpp"
#include "renderer/vulkan/VulkanAssertion.hpp"

namespace sge::vulkan {

VulkanMasterRendergraph::VulkanMasterRendergraph(
    VulkanRendererBackend* renderer)
    : Rendergraph(reinterpret_cast<RendererBackend*>(renderer)),
      m_imageAvaiableSemaphoreId(m_vrenderer->createSemaphore()),
      m_renderingDoneSemaphoreId(m_vrenderer->createSemaphore()),
      m_swapchain_src_pass_id(createPass<VulkanFramebufferSrcPass>(renderer)),
      m_swapchain_dest_pass_id(createPass<VulkanFramebufferDestPass>(renderer)),
      m_triangle_pass_id(createPass<VulkanDrawTrianglePass>(renderer)) {
  addLinkage(m_swapchain_src_pass_id,
             getPassById<VulkanFramebufferSrcPass>(m_swapchain_src_pass_id)
                 .getFramebufferSourceId(),
             m_triangle_pass_id,
             getPassById<VulkanDrawTrianglePass>(m_triangle_pass_id)
                 .getFramebufferSinkId());
  addLinkage(m_triangle_pass_id,
             getPassById<VulkanDrawTrianglePass>(m_triangle_pass_id)
                 .getFramebufferSourceId(),
             m_swapchain_dest_pass_id,
             getPassById<VulkanFramebufferDestPass>(m_swapchain_dest_pass_id)
                 .getFramebufferSinkId());
}

void VulkanMasterRendergraph::beginFrame() {
  // 1..select framebuffer from
  //     swapchain.VulkanFramebuffer* framebuffer;  // TODO implement
  // VulkanFramebuffer& swapchainFramebuffer = *framebuffer;
  // getPassById<VulkanFramebufferSrcPass>(m_swapchain_src_pass_id)
  //     .setFramebuffer(swapchainFramebuffer);
  m_vrenderer->acquireNextSwapchainFrame(m_imageAvaiableSemaphoreId);

  Rendergraph::beginFrame();  // call to super method.
  println("BEGIN-FRAME");
}

void VulkanMasterRendergraph::endFrame() {
  // VulkanFramebuffer& swapchainFramebuffer =
  //     getPassById<VulkanFramebufferDestPass>(m_swapchain_dest_pass_id)
  //         .getFramebuffer();
  // present swapchainFramebuffer
  println("END-FRAME");
}

}  // namespace sge::vulkan
