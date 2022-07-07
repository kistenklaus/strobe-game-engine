#pragma once
#include "renderer/RenderPass.hpp"
#include "renderer/vulkan/VRendererBackend.hpp"

namespace sge::vulkan {

class VTriangleRenderPass : public RenderPass {
 public:
  explicit VTriangleRenderPass(VRendererBackend* renderer,
                               const std::string name);
  void recreate() override;
  void execute() override;
  void dispose() override;

 private:
  const u32 m_framebufferSink;
  const u32 m_renderPassSink;
  const u32 m_pipelineLayoutSink;
  const u32 m_cmdBuffSink;
  const u32 m_cmdBuffSource;

  pipeline m_pipeline;

  std::optional<std::tuple<renderpass, pipeline_layout>> m_current;

  shader_module m_vertexShaderHandle;
  shader_module m_fragmentShaderHandle;
};

}  // namespace sge::vulkan
