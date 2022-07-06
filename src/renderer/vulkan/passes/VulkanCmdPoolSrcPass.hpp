#pragma once

#include <string>

#include "renderer/RenderPass.hpp"
#include "renderer/vulkan/VRendererBackend.hpp"

namespace sge::vulkan {

class VulkanCmdPoolSrcPass : public RenderPass {
 public:
  explicit VulkanCmdPoolSrcPass(VRendererBackend* renderer,
                                const std::string name,
                                QueueFamilyType queueFamily);

  u32 getPoolSource() const { return m_poolSource; }

 private:
  const u32 m_pool;
  const u32 m_poolSource;
};

}  // namespace sge::vulkan