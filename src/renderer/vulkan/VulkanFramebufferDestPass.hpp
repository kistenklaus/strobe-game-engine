#pragma once

#include "renderer/RenderPass.hpp"
#include "renderer/vulkan/VulkanFramebuffer.hpp"

namespace sge {

class VulkanFramebufferDestPass : public RenderPass {
 public:
  VulkanFramebufferDestPass(Renderer& renderer);
  const u32 getFramebufferSinkId();
  VulkanFramebuffer& getFramebuffer();

 private:
  const u32 m_framebuffer_sink_id;
};

VulkanFramebufferDestPass::VulkanFramebufferDestPass(Renderer& renderer)
    : RenderPass(renderer),
      m_framebuffer_sink_id(registerSink<VulkanFramebuffer>()) {}

const u32 VulkanFramebufferDestPass::getFramebufferSinkId() {
  return m_framebuffer_sink_id;
}

VulkanFramebuffer& VulkanFramebufferDestPass::getFramebuffer() {
  return getSinkResource<VulkanFramebuffer>(m_framebuffer_sink_id);
}

}  // namespace sge
