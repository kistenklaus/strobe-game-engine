#pragma once

#include "renderer/RenderPass.hpp"
#include "renderer/vulkan/VulkanRendererBackend.hpp"

namespace sge::vulkan {

class VulkanCmdPoolSrcPass : public RenderPass {
 public:
  VulkanCmdPoolSrcPass(VulkanRendererBackend* renderer,
                       QueueFamilyType queueFamily);

 private:
  const u32 m_pool;
  const u32 m_poolSource;
};

}  // namespace sge::vulkan
