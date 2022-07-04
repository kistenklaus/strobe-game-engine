#pragma once

#include <string>

#include "renderer/RenderPass.hpp"
#include "renderer/vulkan/VulkanRendererBackend.hpp"

namespace sge::vulkan {

class VulkanFenceMultiSrcPass : public RenderPass {
 public:
  VulkanFenceMultiSrcPass(VulkanRendererBackend* renderer,
                          const std::string name, const u32 count);
  void recreate() override;
  void dispose() override;

 private:
  const u32 m_fencesSource;
  std::vector<u32> m_fences;
};

}  // namespace sge::vulkan
