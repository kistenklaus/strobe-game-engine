#pragma once
#include "renderer/Renderer.hpp"
#include "renderer/vulkan/VulkanRendererBackend.hpp"

namespace sge::vulkan {

class VulkanEndCmdBuffPass : public RenderPass {
 public:
  VulkanEndCmdBuffPass(VulkanRendererBackend* renderer, const std::string name);
  void execute() override;

  u32 getCmdBuffSink() const { return m_cmdBuffSink; }
  u32 getCmdBuffSource() const { return m_cmdBuffSource; }

 private:
  const u32 m_cmdBuffSink;
  const u32 m_cmdBuffSource;
};

}  // namespace sge::vulkan
