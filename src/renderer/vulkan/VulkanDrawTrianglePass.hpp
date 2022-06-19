#pragma once
#include "renderer/RenderPass.hpp"
#include "renderer/vulkan/VulkanFramebuffer.hpp"

namespace sge {

class VulkanDrawTrianglePass : public RenderPass {
 public:
  VulkanDrawTrianglePass(RendererBackend* renderer);
  void execute() override;

  const u32 getFramebufferSourceId();
  const u32 getFramebufferSinkId();

 private:
  const u32 m_framebuffer_sink_id;
  const u32 m_framebuffer_source_id;
};

}  // namespace sge
