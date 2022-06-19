#include "renderer/vulkan/VulkanDrawTrianglePass.hpp"

#include "logging/print.hpp"

namespace sge {

VulkanDrawTrianglePass::VulkanDrawTrianglePass(RendererBackend* renderer)
    : RenderPass(renderer),
      m_framebuffer_sink_id(registerSink<VulkanFramebuffer>()),
      m_framebuffer_source_id(registerSource<VulkanFramebuffer>()) {}

void VulkanDrawTrianglePass::execute() {
  // TODO implement
  println("EXECUTE-TRIANGLE");
}

const u32 VulkanDrawTrianglePass::getFramebufferSinkId() {
  return m_framebuffer_sink_id;
}

const u32 VulkanDrawTrianglePass::getFramebufferSourceId() {
  return m_framebuffer_source_id;
}
}  // namespace sge
