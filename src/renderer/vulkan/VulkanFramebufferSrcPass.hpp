#pragma once

#include "renderer/RenderPass.hpp"
#include "renderer/vulkan/VulkanFramebuffer.hpp"

namespace sge {

class VulkanFramebufferSrcPass : public RenderPass {
 public:
  VulkanFramebufferSrcPass(Renderer& renderer);
  void setFramebuffer(VulkanFramebuffer& framebuffer);
  const u32 getFramebufferSourceId();

 private:
  const u32 m_framebuffer_source_id;
};

VulkanFramebufferSrcPass::VulkanFramebufferSrcPass(Renderer& renderer)
    : RenderPass(renderer),
      m_framebuffer_source_id(registerSource<VulkanFramebuffer>()) {}

void VulkanFramebufferSrcPass::setFramebuffer(VulkanFramebuffer& framebuffer) {
  setSourceResource(m_framebuffer_source_id, framebuffer);
}

const u32 VulkanFramebufferSrcPass::getFramebufferSourceId() {
  return m_framebuffer_source_id;
}

}  // namespace sge
