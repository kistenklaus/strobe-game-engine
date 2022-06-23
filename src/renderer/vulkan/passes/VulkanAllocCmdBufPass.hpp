#pragma once

#include "renderer/RenderPass.hpp"
#include "renderer/vulkan/VulkanRendererBackend.hpp"

namespace sge::vulkan {

class VulkanAllocCmdBufPass : public RenderPass {
  //
  VulkanAllocCmdBufPass(VulkanRendererBackend* renderer);
};

}  // namespace sge::vulkan
