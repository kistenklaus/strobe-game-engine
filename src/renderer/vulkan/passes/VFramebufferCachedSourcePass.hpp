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
  sink<imageview> m_imageViewSink;
  sink<renderpass> m_renderPassSink;
  source<framebuffer> m_framebufferSource;
  std::map<std::pair<imageview, renderpass>, framebuffer> m_cache;
};

}  // namespace sge::vulkan
