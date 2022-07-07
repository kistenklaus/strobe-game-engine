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
  sink<command_buffer> m_cmdBufferSink;
  source<command_buffer> m_cmdBufferSource;
};

}  // namespace sge::vulkan
