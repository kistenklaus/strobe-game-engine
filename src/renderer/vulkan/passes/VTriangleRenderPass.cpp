#include "renderer/vulkan/passes/VTriangleRenderPass.hpp"

#include "fileio/fileio.hpp"

namespace sge::vulkan {
VTriangleRenderPass::VTriangleRenderPass(VRendererBackend* renderer,
                                         const std::string name)
    : RenderPass(renderer, name),
      m_framebufferSink(sink<framebuffer>("framebuffer")),
      m_renderPassSink(sink<renderpass>("renderpass")),
      m_pipelineLayoutSink(sink<pipeline_layout>("pipelinelayout")),
      m_cmdBuffSink(sink<command_buffer>("cmdbuffer")),
      m_cmdBuffSource(source<command_buffer>("cmdbuffer")) {
  registerSink(&m_framebufferSink);
  registerSink(&m_renderPassSink);
  registerSink(&m_pipelineLayoutSink);
  registerSink(&m_cmdBuffSink);
  registerSource(&m_cmdBuffSource);

  m_vertexShaderHandle = m_vrenderer->createShaderModule(
      "renderer/vulkan/shaders/shader.vert", SHADER_TYPE_VERTEX);

  m_fragmentShaderHandle = m_vrenderer->createShaderModule(
      "renderer/vulkan/shaders/shader.frag", SHADER_TYPE_FRAGMENT);
}

void VTriangleRenderPass::recreate() {
  if (m_current.has_value()) {
    m_current = std::nullopt;
    m_vrenderer->destroyPipeline(m_pipeline);
  }
}

void VTriangleRenderPass::dispose() {}

void VTriangleRenderPass::execute() {
  const std::tuple<renderpass, pipeline_layout> expected = {
      *m_renderPassSink, *m_pipelineLayoutSink};

  if (m_current.has_value() && expected != m_current) {
    m_vrenderer->destroyPipeline(m_pipeline);
    m_current = std::nullopt;
  }

  if (!m_current.has_value()) {
    m_current = expected;
    std::pair<u32, u32> dim =
        m_vrenderer->getFramebufferDimensions(*m_framebufferSink);

    m_pipeline = m_vrenderer->createPipeline(
        *m_renderPassSink, *m_pipelineLayoutSink, dim.first, dim.second,
        m_vertexShaderHandle, m_fragmentShaderHandle);
  }

  m_vrenderer->beginRenderPass(
      *m_renderPassSink, *m_framebufferSink,
      m_vrenderer->getFramebufferDimensions(*m_framebufferSink).first,
      m_vrenderer->getFramebufferDimensions(*m_framebufferSink).second,
      *m_cmdBuffSink);

  m_vrenderer->bindPipeline(m_pipeline, *m_cmdBuffSink);

  m_vrenderer->drawCall(3, 1, *m_cmdBuffSink);

  m_vrenderer->endRenderPass(*m_cmdBuffSink);

  m_cmdBuffSource.set(m_cmdBuffSink);
}

}  // namespace sge::vulkan
