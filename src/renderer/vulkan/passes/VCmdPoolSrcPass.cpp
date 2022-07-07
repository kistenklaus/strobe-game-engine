#include "renderer/vulkan/passes/VCmdPoolSrcPass.hpp"

namespace sge::vulkan {

VCmdPoolSrcPass::VCmdPoolSrcPass(VRendererBackend* renderer,
                                 const std::string name,
                                 QueueFamilyType queueFamily)
    : RenderPass(renderer, name, false),
      m_pool(m_vrenderer->createCommandPool(queueFamily)),
      m_poolSource(source<command_pool>("cmdpool")) {
  //
  registerSource(&m_poolSource);
  m_poolSource.set(&m_pool);
}

}  // namespace sge::vulkan
