#pragma once

#include <string>

#include "renderer/RenderPass.hpp"
#include "renderer/vulkan/VulkanRendererBackend.hpp"

namespace sge::vulkan {

class VulkanResetCmdPoolPass : public RenderPass {
 public:
  explicit VulkanResetCmdPoolPass(VulkanRendererBackend* renderer,
                                  const std::string name);
  void execute() override;
  void recreate() override;

 private:
  const u32 m_fenceSink;
  const u32 m_poolSink;
  const u32 m_fenceSource;
  const u32 m_poolSource;
  std::set<u32> m_seenFences;
};

}  // namespace sge::vulkan
