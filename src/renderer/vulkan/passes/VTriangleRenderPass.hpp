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
  sink<framebuffer> m_framebufferSink;
  sink<renderpass> m_renderPassSink;
  sink<pipeline_layout> m_pipelineLayoutSink;
  sink<command_buffer> m_cmdBuffSink;
  source<command_buffer> m_cmdBuffSource;

  pipeline m_pipeline;

  std::optional<std::tuple<renderpass, pipeline_layout>> m_current;

  shader_module m_vertexShaderHandle;
  shader_module m_fragmentShaderHandle;
  vertex_buffer m_vertexBuffer;
  index_buffer m_indexBuffer;
  uniform_buffer m_uniformBuffer;
  descriptor_pool m_descriptorPool;
  descriptor_set_layout m_descriptorSetLayout;
  descriptor_set m_desciprotSet;
  pipeline_layout m_pipelineLayout;
};

}  // namespace sge::vulkan
