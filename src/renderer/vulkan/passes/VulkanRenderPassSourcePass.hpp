#pragma once

#include "renderer/RenderPass.hpp"
#include "renderer/vulkan/VulkanRendererBackend.hpp"

namespace sge::vulkan {

class VulkanRenderPassSourcePass : public RenderPass {
 public:
  explicit VulkanRenderPassSourcePass(VulkanRendererBackend* renderer,
                                      const std::string name,
                                      const VkFormat colorFormat);

  void dispose() override;

 private:
  const u32 m_renderPass;
  const u32 m_renderPassSource;
};

}  // namespace sge::vulkan
