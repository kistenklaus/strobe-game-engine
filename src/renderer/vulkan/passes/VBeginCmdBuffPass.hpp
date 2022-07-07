#pragma once

#include "renderer/RenderPass.hpp"
#include "renderer/vulkan/VRendererBackend.hpp"

namespace sge::vulkan {

class VBeginCmdBuffPass : public RenderPass {
 public:
  explicit VBeginCmdBuffPass(VRendererBackend* renderer,
                             const std::string name);
  void execute() override;

 private:
  const u32 m_cmdBufferSink;
  const u32 m_cmdBufferSource;
};

}  // namespace sge::vulkan
