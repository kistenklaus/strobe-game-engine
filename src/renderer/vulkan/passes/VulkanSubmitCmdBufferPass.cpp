#include "renderer/vulkan/passes/VulkanSubmitCmdBufferPass.hpp"

namespace sge::vulkan {

VulkanSubmitCmdBufferPass::VulkanSubmitCmdBufferPass(
    VulkanRendererBackend* renderer)
    : RenderPass(renderer) {}

void VulkanSubmitCmdBufferPass::execute() {
  // nothing for now.
}

}  // namespace sge::vulkan
