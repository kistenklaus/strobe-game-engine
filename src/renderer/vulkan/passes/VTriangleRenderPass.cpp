#include "renderer/vulkan/passes/VTriangleRenderPass.hpp"

#include "fileio/fileio.hpp"
#include "logging/print.hpp"

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
  m_vertexBuffer = m_vrenderer->createVertexBuffer(sizeof(float) * 3 * 4);
  float vertexData[] = {
      0.5f,  -0.5f, 0.0f,  //
      0.5f,  0.5f,  0.0f,  //
      -0.5f, 0.5f,  0.0f,  //
      -0.5f, -0.5f, 0.0f   //
  };
  m_vrenderer->uploadToBuffer(m_vertexBuffer, vertexData);
  m_indexBuffer = m_vrenderer->createIndexBuffer(sizeof(u32) * 3);
  u32 indicies[] = {0, 1, 2};
  u32* ptr = indicies;
  m_vrenderer->uploadToBuffer(m_indexBuffer, ptr);

  m_descriptorPool = m_vrenderer->createDescriptorPool(1);
  m_descriptorSetLayout =
      m_vrenderer->createDescriptorSetLayout(0, 1, VK_SHADER_STAGE_VERTEX_BIT);
  m_desciprotSet = m_vrenderer->allocateDescriptorSets(
      m_descriptorPool, m_descriptorSetLayout, 1)[0];

  m_uniformBuffer = m_vrenderer->createUniformBuffer(4 * sizeof(float));
  float uniformData[] = {0.0f, 1.0f, 0.0f, 1.0f};
  m_vrenderer->uploadToBuffer(m_uniformBuffer, uniformData);

  m_vrenderer->updateDescriptorSet(m_desciprotSet, m_uniformBuffer);
  std::vector<descriptor_set_layout> layouts = {m_descriptorSetLayout};
  m_pipelineLayout = m_vrenderer->createPipelineLayout(layouts);
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
        *m_renderPassSink, m_pipelineLayout, dim.first, dim.second,
        m_vertexShaderHandle, m_fragmentShaderHandle);
  }

  m_vrenderer->beginRenderPass(
      *m_renderPassSink, *m_framebufferSink,
      m_vrenderer->getFramebufferDimensions(*m_framebufferSink).first,
      m_vrenderer->getFramebufferDimensions(*m_framebufferSink).second,
      *m_cmdBuffSink);

  m_vrenderer->bindPipeline(m_pipeline, *m_cmdBuffSink);
  m_vrenderer->bindVertexBuffer(m_vertexBuffer, *m_cmdBuffSink);
  m_vrenderer->bindIndexBuffer(m_indexBuffer, *m_cmdBuffSink);
  m_vrenderer->bindDescriptorSet(m_desciprotSet, m_pipelineLayout,
                                 *m_cmdBuffSink);

  m_vrenderer->indexedDrawCall(3, *m_cmdBuffSink);
  // m_vrenderer->drawCall(3, 1, *m_cmdBuffSink);

  m_vrenderer->endRenderPass(*m_cmdBuffSink);

  m_cmdBuffSource.set(m_cmdBuffSink);
}

}  // namespace sge::vulkan
