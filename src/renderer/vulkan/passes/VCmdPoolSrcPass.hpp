#pragma once

#include <string>

#include "renderer/RenderPass.hpp"
#include "renderer/vulkan/VRendererBackend.hpp"

namespace sge::vulkan {

class VCmdPoolSrcPass : public RenderPass {
 public:
  explicit VCmdPoolSrcPass(VRendererBackend* renderer, const std::string name,
                           QueueFamilyType queueFamily);

 private:
  command_pool m_pool;
  source<command_pool> m_poolSource;
};

}  // namespace sge::vulkan
