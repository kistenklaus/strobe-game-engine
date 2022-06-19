#include "renderer/vulkan/VulkanFramebufferSrcPass.hpp"

#include "logging/print.hpp"

namespace sge {

VulkanFramebufferSrcPass::VulkanFramebufferSrcPass(RendererBackend* renderer)
    : RenderPass(renderer),
      m_framebuffer_source_id(registerSource<VulkanFramebuffer>()) {}

void VulkanFramebufferSrcPass::setFramebuffer(VulkanFramebuffer& framebuffer) {
  setSourceResource(m_framebuffer_source_id, framebuffer);
}

void VulkanFramebufferSrcPass::execute() { println("EXECUTE-SRC"); }

const u32 VulkanFramebufferSrcPass::getFramebufferSourceId() {
  return m_framebuffer_source_id;
}
}  // namespace sge
