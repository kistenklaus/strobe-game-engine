#pragma once
#include "renderer/RenderPass.hpp"
#include "renderer/vulkan/VRendererBackend.hpp"

namespace sge::vulkan {

class VulkanTriangleRenderPass : public RenderPass {
 public:
  explicit VulkanTriangleRenderPass(VRendererBackend* renderer,
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

  u32 m_pipeline = 0;

  std::optional<std::tuple<u32, u32>> m_current;

  u32 m_vertexShader = 0;
  u32 m_fragmentShader = 0;
};

}  // namespace sge::vulkan
