#include "renderer/vulkan/passes/VCmdPoolSrcPass.hpp"

namespace sge::vulkan {

VCmdPoolSrcPass::VCmdPoolSrcPass(VRendererBackend* renderer,
                                 const std::string name,
                                 QueueFamilyType queueFamily)
    : RenderPass(renderer, name, false),
      m_pool(m_vrenderer->createCommandPool(queueFamily)),
      m_poolSource(registerSource<command_pool>("cmdpool")) {
  //
  setSourceResource(m_poolSource, &m_pool);
}

}  // namespace sge::vulkan
