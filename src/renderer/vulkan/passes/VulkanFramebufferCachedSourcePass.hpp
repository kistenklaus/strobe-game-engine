#pragma once
#include <map>

#include "renderer/RenderPass.hpp"
#include "renderer/vulkan/VulkanRendererBackend.hpp"

namespace sge::vulkan {

class VulkanFramebufferCachedSourcePass : public RenderPass {
 public:
  explicit VulkanFramebufferCachedSourcePass(VulkanRendererBackend* renderer,
                                             const std::string name);

  void execute() override;
  void recreate() override;
  void dispose() override;

  u32 getImageViewSink() const { return m_imageViewSink; }
  u32 getRenderPassSink() const { return m_renderPassSink; }
  u32 getFramebufferSource() const { return m_framebufferSource; }

 private:
  const u32 m_imageViewSink;
  const u32 m_renderPassSink;
  const u32 m_framebufferSource;
  std::map<std::pair<u32, u32>, u32> m_cache;
};

}  // namespace sge::vulkan
