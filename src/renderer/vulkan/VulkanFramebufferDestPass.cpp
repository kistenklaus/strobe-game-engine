#include "renderer/vulkan/VulkanFramebufferDestPass.hpp"

#include "logging/print.hpp"

namespace sge {

VulkanFramebufferDestPass::VulkanFramebufferDestPass(RendererBackend* renderer)
    : RenderPass(renderer),
      m_framebuffer_sink_id(registerSink<VulkanFramebuffer>()) {}

const u32 VulkanFramebufferDestPass::getFramebufferSinkId() {
  return m_framebuffer_sink_id;
}

void VulkanFramebufferDestPass::execute() { println("EXECUTE-DEST"); }

VulkanFramebuffer& VulkanFramebufferDestPass::getFramebuffer() {
  return getSinkResource<VulkanFramebuffer>(m_framebuffer_sink_id);
}
}  // namespace sge
