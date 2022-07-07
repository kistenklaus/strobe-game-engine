#pragma once

#include <string>

#include "renderer/RenderPass.hpp"
#include "renderer/vulkan/VRendererBackend.hpp"

namespace sge::vulkan {

class VFenceMultiSrcPass : public RenderPass {
 public:
  VFenceMultiSrcPass(VRendererBackend* renderer, const std::string name,
                     const u32 count);
  void recreate() override;
  void dispose() override;

 private:
  source<std::vector<fence>> m_fencesSource;
  std::vector<fence> m_fences;
};

}  // namespace sge::vulkan
