#pragma once

#include "renderer/RenderPass.hpp"
#include "renderer/vulkan/VulkanFramebuffer.hpp"

namespace sge {

class VulkanFramebufferSrcPass : public RenderPass {
 public:
  VulkanFramebufferSrcPass(RendererBackend* renderer);
  void setFramebuffer(VulkanFramebuffer& framebuffer);
  const u32 getFramebufferSourceId();
  void execute() override;

 private:
  const u32 m_framebuffer_source_id;
};

}  // namespace sge
