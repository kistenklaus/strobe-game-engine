#pragma once
#include <logging/print.hpp>
#include <map>

#include "renderer/RenderPass.hpp"
#include "renderer/vulkan/VDrawJob.hpp"
#include "renderer/vulkan/VRenderable.hpp"
#include "renderer/vulkan/VRendererBackend.hpp"
#include "types/sarray.hpp"

namespace sge::vulkan {

class VRenderQueuePass : public RenderPass {
 public:
  VRenderQueuePass(VRendererBackend* renderer, const std::string name)
      : RenderPass(renderer, name),
        mp_renderPass(sinksource<renderpass>("renderpass")),
        mp_framebuffer(sinksource<framebuffer>("framebuffer")),
        mp_commandBuffer(sinksource<command_buffer>("cmdbuffer")) {
    registerSinkSource(&mp_renderPass);
    registerSinkSource(&mp_framebuffer);
    registerSinkSource(&mp_commandBuffer);
  }
  virtual void beforeEach() {}
  virtual void afterEach() {}
  virtual void execute() {
    beforeEach();

    u32 rindex;
    if (m_renderPassIndexMapping.contains(*mp_renderPass)) {
      rindex = m_renderPassIndexMapping[*mp_renderPass];
    } else {
      rindex = m_renderPassCount++;
      m_renderPassIndexMapping[*mp_renderPass] = rindex;
      // ensure capacity of the pipelineMatrix.
      for (u32 i = 0; i < m_pipelineMatrix.size(); i++) {
        if (m_pipelineMatrix[i].size() < m_renderPassCount) {
          m_pipelineMatrix[i].resize(2 * m_renderPassCount);
        }
      }
    }
    for (VDrawJob& drawJob : m_drawJobs) {
      const u32 pindex = drawJob.m_pipelineIndex;

      pipeline pipeline;
      descriptorContainer descriptorContainer;
      if (m_pipelineMatrix[pindex][rindex]) {
        pipeline = m_pipelineMatrix[pindex][rindex].value();
        descriptorContainer = m_descriptorSetContainers[pindex];
      } else {
        shader_module vertexShader = m_vrenderer->createShaderModule(
            drawJob.m_renderable->m_vertexShaderPath, SHADER_TYPE_VERTEX);
        shader_module fragmentShader = m_vrenderer->createShaderModule(
            drawJob.m_renderable->m_fragmentShaderPath, SHADER_TYPE_FRAGMENT);

        u32 setCount;
        std::vector<descriptor_set_layout> descriptorLayouts =
            m_vrenderer->getDescriptorSetLayoutsFromShader(
                vertexShader, fragmentShader, &setCount);

        descriptor_pool descriptorPool =
            m_vrenderer->createDescriptorPool(setCount);

        std::vector<descriptor_set> sets;
        for (u32 i = 0; i < descriptorLayouts.size(); i++) {
          std::vector<descriptor_set> res = m_vrenderer->allocateDescriptorSets(
              descriptorPool, descriptorLayouts[i],
              m_vrenderer->getDescriptorSetLayoutCount(descriptorLayouts[i]));
          for (descriptor_set& set : res) {
            sets.push_back(set);
          }
        }

        pipeline_layout pipelineLayout =
            m_vrenderer->createPipelineLayout(descriptorLayouts);

        const std::pair<u32, u32> framebufferDimensions =
            m_vrenderer->getFramebufferDimensions(*mp_framebuffer);
        pipeline = m_vrenderer->createPipeline(
            *mp_renderPass, pipelineLayout, framebufferDimensions.first,
            framebufferDimensions.second, vertexShader, fragmentShader);

        m_pipelineMatrix[pindex][rindex] = pipeline;
      }
      m_vrenderer->bindPipeline(pipeline, *mp_commandBuffer);
      // TODO bind descriptorSets with correct offset.
      drawInstance(*drawJob.m_renderable, pipeline);
    }
    afterEach();
    m_drawJobs.clear();
    mp_renderPass.forward();
    mp_framebuffer.forward();
    mp_commandBuffer.forward();
  }

  virtual void drawInstance(VRenderable& drawJob, pipeline pipeline) = 0;

  virtual void submit(std::shared_ptr<VRenderable> renderable) {
    u32 pindex;
    std::pair<std::string, std::string> pipelineIdentifier = {
        renderable->m_vertexShaderPath, renderable->m_fragmentShaderPath};
    if (m_pipelineIndexMapping.contains(pipelineIdentifier)) {
      pindex = m_pipelineIndexMapping[pipelineIdentifier];
    } else {
      pindex = m_pipelineCount++;
      u32 oldSize = m_pipelineMatrix.size();
      if (m_pipelineMatrix.size() < m_pipelineCount) {
        m_pipelineMatrix.resize(m_pipelineCount * 2);
      }
      // ensure capacity of the pipelineMatrix.
      for (u32 i = oldSize; i < m_pipelineMatrix.size(); i++) {
        m_pipelineMatrix[i].resize(2 * m_renderPassCount);
      }
      m_pipelineIndexMapping[pipelineIdentifier] = pindex;
    }

    m_drawJobs.push_back(VDrawJob(renderable, pindex));
  }

 protected:
  sinksource<renderpass> mp_renderPass;
  sinksource<framebuffer> mp_framebuffer;
  sinksource<command_buffer> mp_commandBuffer;

 private:
  struct descriptorContainer {
    descriptor_pool descriptorPool;
  };

 private:
  std::vector<VDrawJob> m_drawJobs;

  std::vector<std::vector<std::optional<pipeline>>> m_pipelineMatrix;
  std::vector<descriptorContainer> m_descriptorSetContainers;

  std::map<std::pair<std::string, std::string>, u32> m_pipelineIndexMapping;
  u32 m_pipelineCount = 0;
  std::map<renderpass, u32> m_renderPassIndexMapping;
  u32 m_renderPassCount = 0;
};

}  // namespace sge::vulkan
