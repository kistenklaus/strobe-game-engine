#pragma once
#include "renderer/RenderPass.hpp"
#include "renderer/vulkan/VulkanRendererBackend.hpp"

namespace sge::vulkan {

class VulkanPipelineLayoutSourcePass : public RenderPass {
 public:
  VulkanPipelineLayoutSourcePass(VulkanRendererBackend* renderer,
                                 const std::string name);
  ~VulkanPipelineLayoutSourcePass();

  u32 getPipelineLayoutSource() const { return m_pipelineLayoutSource; }

 private:
  const u32 m_pipelineLayout;
  const u32 m_pipelineLayoutSource;
};

}  // namespace sge::vulkan
