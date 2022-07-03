#pragma once

#include "renderer/RenderPass.hpp"
#include "renderer/vulkan/VulkanRendererBackend.hpp"

namespace sge::vulkan {

class VulkanRenderPassSourcePass : public RenderPass {
 public:
  VulkanRenderPassSourcePass(VulkanRendererBackend* renderer,
                             const std::string name,
                             const VkFormat colorFormat);
  ~VulkanRenderPassSourcePass();

  u32 getRenderPassSource() const { return m_renderPassSource; }

 private:
  const u32 m_renderPass;
  const u32 m_renderPassSource;
};

}  // namespace sge::vulkan
