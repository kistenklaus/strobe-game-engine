#pragma once

#include <string>

#include "renderer/RenderPass.hpp"
#include "renderer/vulkan/VRendererBackend.hpp"

namespace sge::vulkan {

class VCmdPoolSrcPass : public RenderPass {
 public:
  explicit VCmdPoolSrcPass(VRendererBackend* renderer, const std::string name,
                           QueueFamilyType queueFamily);

  u32 getPoolSource() const { return m_poolSource; }

 private:
  const command_pool m_pool;
  const u32 m_poolSource;
};

}  // namespace sge::vulkan
