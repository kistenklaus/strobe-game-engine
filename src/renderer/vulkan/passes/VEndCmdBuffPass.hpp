#pragma once
#include "renderer/Renderer.hpp"
#include "renderer/vulkan/VRendererBackend.hpp"

namespace sge::vulkan {

class VEndCmdBuffPass : public RenderPass {
 public:
  explicit VEndCmdBuffPass(VRendererBackend* renderer, const std::string name);
  void execute() override;

 private:
  sink<command_buffer> m_cmdBuffSink;
  source<command_buffer> m_cmdBuffSource;
};

}  // namespace sge::vulkan
