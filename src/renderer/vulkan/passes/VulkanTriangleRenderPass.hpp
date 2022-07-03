#pragma once
#include "renderer/RenderPass.hpp"
#include "renderer/vulkan/VulkanRendererBackend.hpp"

namespace sge::vulkan {

class VulkanTriangleRenderPass : public RenderPass {
 public:
  VulkanTriangleRenderPass(VulkanRendererBackend* renderer,
                           const std::string name);
  ~VulkanTriangleRenderPass() override;
  void execute() override;

  u32 getFramebufferSink() const { return m_framebufferSink; }
  u32 getRenderPassSink() const { return m_renderPassSink; }
  u32 getPipelineLayoutSink() const { return m_pipelineLayoutSink; }
  u32 getCmdBuffSink() const { return m_cmdBuffSink; }
  u32 getCmdBuffSource() const { return m_cmdBuffSource; }

 private:
  const u32 m_framebufferSink;
  const u32 m_renderPassSink;
  const u32 m_pipelineLayoutSink;
  const u32 m_cmdBuffSink;
  const u32 m_cmdBuffSource;

  u32 m_pipeline = 0;

  //(framebuffer, renderPass, pipelineLayout [width?, height?].
  std::optional<std::tuple<u32, u32, u32>> m_current;

  u32 m_vertexShader = 0;
  u32 m_fragmentShader = 0;
};

}  // namespace sge::vulkan
