#pragma once
#include "renderer/RenderPass.hpp"
#include "renderer/vulkan/VulkanRendererBackend.hpp"

namespace sge::vulkan {

class VulkanCmdPoolMultiSrcPass : public RenderPass {
 public:
  VulkanCmdPoolMultiSrcPass(VulkanRendererBackend* renderer,
                            const std::string name,
                            const QueueFamilyType queueFamily, const u32 count);
  ~VulkanCmdPoolMultiSrcPass() override;

 private:
  const std::vector<u32> m_pools;
  const u32 m_poolsSource;
};

}  // namespace sge::vulkan
