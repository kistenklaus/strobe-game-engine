#include "renderer/vulkan/passes/VTriangleRenderPass.hpp"

#include "fileio/fileio.hpp"

namespace sge::vulkan {
VTriangleRenderPass::VTriangleRenderPass(VRendererBackend* renderer,
                                         const std::string name)
    : RenderPass(renderer, name),
      m_framebufferSink(registerSink<framebuffer>("framebuffer")),
      m_renderPassSink(registerSink<renderpass>("renderpass")),
      m_pipelineLayoutSink(registerSink<pipeline_layout>("pipelinelayout")),
      m_cmdBuffSink(registerSink<command_buffer>("cmdbuffer")),
      m_cmdBuffSource(registerSource<command_buffer>("cmdbuffer")) {
  const std::vector<char> vertexSrc =
      fileio::read("renderer/vulkan/shaders/shader.vert.spv");
  m_vertexShaderHandle = m_vrenderer->createShaderModule(vertexSrc);

  const std::vector<char> fragmentSrc =
      fileio::read("renderer/vulkan/shaders/shader.frag.spv");
  m_fragmentShaderHandle = m_vrenderer->createShaderModule(fragmentSrc);
}

void VTriangleRenderPass::recreate() {
  if (m_current.has_value()) {
    m_current = std::nullopt;
    m_vrenderer->destroyPipeline(m_pipeline);
  }
}

void VTriangleRenderPass::dispose() {}

void VTriangleRenderPass::execute() {
  const framebuffer* p_framebuffer =
      getSinkResource<framebuffer>(m_framebufferSink);
  const renderpass* p_renderPass =
      getSinkResource<renderpass>(m_renderPassSink);
  const pipeline_layout* p_pipelineLayout =
      getSinkResource<pipeline_layout>(m_pipelineLayoutSink);

  const std::tuple<renderpass, pipeline_layout> expected = {*p_renderPass,
                                                            *p_pipelineLayout};

  if (m_current.has_value() && expected != m_current) {
    m_vrenderer->destroyPipeline(m_pipeline);
    m_current = std::nullopt;
  }

  if (!m_current.has_value()) {
    m_current = expected;
    std::pair<u32, u32> dim =
        m_vrenderer->getFramebufferDimensions(*p_framebuffer);

    m_pipeline = m_vrenderer->createPipeline(
        *p_renderPass, *p_pipelineLayout, dim.first, dim.second,
        m_vertexShaderHandle, m_fragmentShaderHandle);
  }

  const command_buffer* p_cmdBuff =
      getSinkResource<command_buffer>(m_cmdBuffSink);

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
