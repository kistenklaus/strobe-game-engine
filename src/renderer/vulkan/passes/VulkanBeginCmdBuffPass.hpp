#pragma once

#include "renderer/RenderPass.hpp"
#include "renderer/vulkan/VRendererBackend.hpp"

namespace sge::vulkan {

class VulkanBeginCmdBuffPass : public RenderPass {
 public:
  explicit VulkanBeginCmdBuffPass(VRendererBackend* renderer,
                                  const std::string name);
  void execute() override;

  u32 getCmdBuffSink() const { return m_cmdBufferSink; }
  u32 getCmdBuffSource() const { return m_cmdBufferSource; }

 private:
  const u32 m_cmdBufferSink;
  const u32 m_cmdBufferSource;
};

}  // namespace sge::vulkan