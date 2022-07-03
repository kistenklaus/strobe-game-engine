#include "renderer/vulkan/passes/VulkanResetCmdPoolPass.hpp"

namespace sge::vulkan {
VulkanResetCmdPoolPass::VulkanResetCmdPoolPass(VulkanRendererBackend* renderer,
                                               const std::string name)
    : RenderPass(renderer, name),
      m_poolSink(registerSink<u32>("cmdpool")),
      m_poolSource(registerSource<u32>("cmdpool")) {}

void VulkanResetCmdPoolPass::execute() {
  u32* pool = getSinkResource<u32>(m_poolSink);
  m_vrenderer->resetCommandPool(*pool);
  setSourceResource(m_poolSource, pool);
}

}  // namespace sge::vulkan
