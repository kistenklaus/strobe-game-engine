#pragma once
#include "renderer/RenderPass.hpp"
#include "renderer/vulkan/VulkanFramebuffer.hpp"

namespace sge {

class VulkanDrawTrianglePass : public RenderPass {
 public:
  VulkanDrawTrianglePass(Renderer& renderer);
  void execute() override;

  const u32 getFramebufferSourceId();
  const u32 getFramebufferSinkId();

 private:
  const u32 m_framebuffer_sink_id;
  const u32 m_framebuffer_source_id;
};

VulkanDrawTrianglePass::VulkanDrawTrianglePass(Renderer& renderer)
    : RenderPass(renderer),
      m_framebuffer_sink_id(registerSink<VulkanFramebuffer>()),
      m_framebuffer_source_id(registerSource<VulkanFramebuffer>()) {}

void VulkanDrawTrianglePass::execute() {
  // TODO implement
}

const u32 VulkanDrawTrianglePass::getFramebufferSinkId() {
  return m_framebuffer_sink_id;
}

const u32 VulkanDrawTrianglePass::getFramebufferSourceId() {
  return m_framebuffer_source_id;
}

}  // namespace sge
