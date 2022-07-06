#pragma once

#include <string>

#include "renderer/RenderPass.hpp"
#include "renderer/vulkan/VRendererBackend.hpp"

namespace sge::vulkan {

class VulkanFenceMultiSrcPass : public RenderPass {
 public:
  VulkanFenceMultiSrcPass(VRendererBackend* renderer, const std::string name,
                          const u32 count);
  void recreate() override;
  void dispose() override;

 private:
  const u32 m_fencesSource;
  std::vector<u32> m_fences;
};

}  // namespace sge::vulkan
