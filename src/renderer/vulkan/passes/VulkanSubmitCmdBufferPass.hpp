#pragma once
#include "renderer/RenderPass.hpp"
#include "renderer/vulkan/VulkanRendererBackend.hpp"

namespace sge::vulkan {

class VulkanSubmitCmdBufferPass : public RenderPass {
 public:
  VulkanSubmitCmdBufferPass(VulkanRendererBackend* renderer);

  void execute() override;

 private:
};
}  // namespace sge::vulkan
