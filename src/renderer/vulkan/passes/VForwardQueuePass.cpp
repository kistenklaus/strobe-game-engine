#include "renderer/vulkan/passes/VForwardQueuePass.hpp"

#include "logging/print.hpp"
#include "renderer/vulkan/bindbales/VMesh.hpp"

namespace sge::vulkan {

VForwardQueuePass::VForwardQueuePass(VRendererBackend* renderer,
                                     const std::string name)
    : VRenderQueuePass(renderer, name) {}

void VForwardQueuePass::beforeEach() {
  std::shared_ptr<VMesh> mesh = std::make_shared<VMesh>(m_vrenderer);
  std::vector<std::shared_ptr<VBindable>> bindables = {
      std::static_pointer_cast<VBindable>(mesh)};

  submit(std::make_shared<VRenderable>("renderer/vulkan/shaders/shader.vert",
                                       "renderer/vulkan/shaders/shader.frag",
                                       bindables, true, 3, 1));

  m_vrenderer->beginRenderPass(*mp_renderPass, *mp_framebuffer,
                               *mp_commandBuffer);
}

void VForwardQueuePass::drawInstance(VRenderable& renderable,
                                     pipeline pipeline) {
  for (const std::shared_ptr<VBindable> bindable : renderable.m_bindables) {
    bindable->bind(m_vrenderer, *mp_renderPass, *mp_commandBuffer, pipeline);
  }

  if (renderable.m_drawIndexed) {
    m_vrenderer->indexedDrawCall(renderable.m_drawCount,
                                 renderable.m_instanceCount, *mp_commandBuffer);
  } else {
    m_vrenderer->drawCall(renderable.m_drawCount, renderable.m_instanceCount,
                          *mp_commandBuffer);
  }
}

void VForwardQueuePass::afterEach() {
  VRenderQueuePass::endFrame();
  m_vrenderer->endRenderPass(*mp_commandBuffer);
}

}  // namespace sge::vulkan
