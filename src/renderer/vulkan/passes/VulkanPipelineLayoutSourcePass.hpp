#pragma once
#include "renderer/RenderPass.hpp"
#include "renderer/vulkan/VulkanRendererBackend.hpp"

namespace sge::vulkan {

class VulkanPipelineLayoutSourcePass : public RenderPass {
 public:
  explicit VulkanPipelineLayoutSourcePass(VulkanRendererBackend* renderer,
                                          const std::string name);

  void dispose() override;

 private:
  const u32 m_pipelineLayout;
  const u32 m_pipelineLayoutSource;
};

}  // namespace sge::vulkan
