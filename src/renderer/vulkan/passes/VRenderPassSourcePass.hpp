#pragma once

#include "renderer/RenderPass.hpp"
#include "renderer/vulkan/VRendererBackend.hpp"

namespace sge::vulkan {

class VRenderPassSourcePass : public RenderPass {
 public:
  explicit VRenderPassSourcePass(VRendererBackend* renderer,
                                 const std::string name,
                                 const VkFormat colorFormat);

  void dispose() override;

 private:
  renderpass m_renderPass;
  source<renderpass> m_renderPassSource;
};

}  // namespace sge::vulkan
