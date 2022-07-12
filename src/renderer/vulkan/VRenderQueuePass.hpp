#pragma once
#include <map>

#include "renderer/RenderPass.hpp"
#include "renderer/vulkan/DrawJob.hpp"
#include "renderer/vulkan/VRendererBackend.hpp"
#include "types/sarray.hpp"

namespace sge::vulkan {

class VRenderQueuePass : public RenderPass {
 public:
  VRenderQueuePass(VRendererBackend* renderer, const std::string name)
      : RenderPass(renderer, name),
        mp_renderPass(sinksource<renderpass>("renderpass")),
        mp_framebuffer(sinksource<framebuffer>("framebuffer")) {
    registerSinkSource(&mp_renderPass);
  }
  virtual void create() {}
  virtual void recreate() {}
  virtual void beginFrame() {
    // maybe sort the drawJobs
  }
  virtual void execute() {
    sarray<pipeline> pipelineMapping = m_pipelineMappings[*mp_renderPass];
    for (VDrawJob& drawJob : m_drawJobs) {
      pipeline pipeline;
      if (pipelineMapping.isValid(drawJob.m_pipelineIndex)) {
        pipeline = pipelineMapping[drawJob.m_pipelineIndex];
      } else {
        shader_module vertexShader = m_vrenderer->createShaderModule(
            drawJob.m_vertexShaderPath, SHADER_TYPE_VERTEX);
        shader_module fragmentShader = m_vrenderer->createShaderModule(
            drawJob.m_fragmentShaderPath, SHADER_TYPE_FRAGMENT);

        std::pair<u32, u32> framebufferDimensions =
            m_vrenderer->getFramebufferDimensions(*mp_framebuffer);

        // TODO parse descriptor set layout for the vertex and fragment shader
        // requires better shader parsing
        std::vector<descriptor_set_layout> descriptorLayouts = {};

        pipeline_layout pipelineLayout =
            m_vrenderer->createPipelineLayout(descriptorLayouts);
        pipeline = m_vrenderer->createPipeline(
            *mp_renderPass, pipelineLayout, framebufferDimensions.first,
            framebufferDimensions.second, vertexShader, fragmentShader);

        pipelineMapping[drawJob.m_pipelineIndex] = pipeline;
      }

      drawInstance(drawJob, pipeline);
    }
  }
  virtual void endFrame() { m_drawJobs.clear(); }
  virtual void dispose() {}

  virtual void drawInstance(VDrawJob& drawJob, pipeline pipeline);

 protected:
  sinksource<renderpass> mp_renderPass;
  sinksource<framebuffer> mp_framebuffer;

 private:
  std::vector<VDrawJob> m_drawJobs;
  std::map<renderpass, sarray<pipeline>> m_pipelineMappings;
};

}  // namespace sge::vulkan
