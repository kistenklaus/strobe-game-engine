#pragma once

#include "renderer/Rendergraph.hpp"
#include "renderer/vulkan/VulkanDrawTrianglePass.hpp"
#include "renderer/vulkan/VulkanFramebufferDestPass.hpp"
#include "renderer/vulkan/VulkanFramebufferSrcPass.hpp"

namespace sge::vulkan {

class VulkanRendergraph : public Rendergraph {
 public:
  VulkanRendergraph(Renderer& renderer);
  void beginFrame() override;
  void endFrame() override;

 private:
  const u32 m_swapchain_src_pass_id;
  const u32 m_swapchain_dest_pass_id;
  const u32 m_triangle_pass_id;
};

VulkanRendergraph::VulkanRendergraph(Renderer& renderer)
    : Rendergraph(renderer),
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
  build();
}

void VulkanRendergraph::beginFrame() {
  // 1. .select framebuffer from swapchain.
  VulkanFramebuffer* framebuffer;  // TODO implement
  VulkanFramebuffer& swapchainFramebuffer = *framebuffer;
  getPassById<VulkanFramebufferSrcPass>(m_swapchain_src_pass_id)
      .setFramebuffer(swapchainFramebuffer);

  RenderPass::beginFrame();  // call to super method.
}

void VulkanRendergraph::endFrame() {
  VulkanFramebuffer& swapchainFramebuffer =
      getPassById<VulkanFramebufferDestPass>(m_swapchain_dest_pass_id)
          .getFramebuffer();
  // present swapchainFramebuffer
}

}  // namespace sge::vulkan
