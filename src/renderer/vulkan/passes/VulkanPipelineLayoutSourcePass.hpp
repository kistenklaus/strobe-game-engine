#pragma once
#include "renderer/RenderPass.hpp"
#include "renderer/vulkan/VRendererBackend.hpp"

namespace sge::vulkan {

class VulkanPipelineLayoutSourcePass : public RenderPass {
 public:
  explicit VulkanPipelineLayoutSourcePass(VRendererBackend* renderer,
                                          const std::string name);

  void dispose() override;

 private:
  const u32 m_pipelineLayout;
  const u32 m_pipelineLayoutSource;
};

}  // namespace sge::vulkan
