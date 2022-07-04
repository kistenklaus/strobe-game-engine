#include "renderer/vulkan/passes/VulkanTriangleRenderPass.hpp"

#include "fileio/fileio.hpp"

namespace sge::vulkan {
VulkanTriangleRenderPass::VulkanTriangleRenderPass(
    VulkanRendererBackend* renderer, const std::string name)
    : RenderPass(renderer, name),
      m_framebufferSink(registerSink<u32>("framebuffer")),
      m_renderPassSink(registerSink<u32>("renderpass")),
      m_pipelineLayoutSink(registerSink<u32>("pipelinelayout")),
      m_cmdBuffSink(registerSink<u32>("cmdbuffer")),
      m_cmdBuffSource(registerSource<u32>("cmdbuffer")) {
  const std::vector<char> vertexSrc =
      fileio::read("renderer/vulkan/shaders/shader.vert.spv");
  m_vertexShader = m_vrenderer->createShaderModule(vertexSrc);

  const std::vector<char> fragmentSrc =
      fileio::read("renderer/vulkan/shaders/shader.frag.spv");
  m_fragmentShader = m_vrenderer->createShaderModule(fragmentSrc);
}

void VulkanTriangleRenderPass::recreate() {
  if (m_current.has_value()) {
    m_current = std::nullopt;
    m_vrenderer->destroyPipeline(m_pipeline);
  }
}

void VulkanTriangleRenderPass::dispose() {}

void VulkanTriangleRenderPass::execute() {
  const u32* p_framebuffer = getSinkResource<u32>(m_framebufferSink);
  const u32* p_renderPass = getSinkResource<u32>(m_renderPassSink);
  const u32* p_pipelineLayout = getSinkResource<u32>(m_pipelineLayoutSink);

  const std::tuple<u32, u32> expected = {*p_renderPass, *p_pipelineLayout};

  if (m_current.has_value() && expected != m_current) {
    m_vrenderer->destroyPipeline(m_pipeline);
    m_current = std::nullopt;
  }

  if (!m_current.has_value()) {
    m_current = expected;
    std::pair<u32, u32> dim =
        m_vrenderer->getFramebufferDimensions(*p_framebuffer);

    m_pipeline = m_vrenderer->createPipeline(*p_renderPass, *p_pipelineLayout,
                                             dim.first, dim.second,
                                             m_vertexShader, m_fragmentShader);
  }

  const u32* p_cmdBuff = getSinkResource<u32>(m_cmdBuffSink);

  m_vrenderer->beginRenderPass(
      *p_renderPass, *p_framebuffer,
      m_vrenderer->getFramebufferDimensions(*p_framebuffer).first,
      m_vrenderer->getFramebufferDimensions(*p_framebuffer).second, *p_cmdBuff);

  m_vrenderer->bindPipeline(m_pipeline, *p_cmdBuff);

  m_vrenderer->drawCall(3, 1, *p_cmdBuff);

  m_vrenderer->endRenderPass(*p_cmdBuff);

  setSourceResource(m_cmdBuffSource, p_cmdBuff);
}

}  // namespace sge::vulkan
