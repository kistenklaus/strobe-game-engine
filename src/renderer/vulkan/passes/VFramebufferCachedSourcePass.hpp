#pragma once
#include <map>

#include "renderer/RenderPass.hpp"
#include "renderer/vulkan/VRendererBackend.hpp"

namespace sge::vulkan {

class VFramebufferCachedSourcePass : public RenderPass {
 public:
  explicit VFramebufferCachedSourcePass(VRendererBackend* renderer,
                                        const std::string name);

  void execute() override;
  void recreate() override;
  void dispose() override;

 private:
  const u32 m_imageViewSink;
  const u32 m_renderPassSink;
  const u32 m_framebufferSource;
  std::map<std::pair<imageview, renderpass>, framebuffer> m_cache;
};

}  // namespace sge::vulkan
