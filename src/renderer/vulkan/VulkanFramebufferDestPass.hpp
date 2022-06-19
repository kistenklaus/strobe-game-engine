#pragma once

#include "renderer/RenderPass.hpp"
#include "renderer/vulkan/VulkanFramebuffer.hpp"

namespace sge {

class VulkanFramebufferDestPass : public RenderPass {
 public:
  VulkanFramebufferDestPass(RendererBackend* renderer);
  const u32 getFramebufferSinkId();
  VulkanFramebuffer& getFramebuffer();
  void execute() override;

 private:
  const u32 m_framebuffer_sink_id;
};

}  // namespace sge
