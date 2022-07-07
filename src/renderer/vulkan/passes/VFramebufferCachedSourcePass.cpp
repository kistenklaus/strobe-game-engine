#include "renderer/vulkan/passes/VFramebufferCachedSourcePass.hpp"

#include "logging//print.hpp"

namespace sge::vulkan {

VFramebufferCachedSourcePass::VFramebufferCachedSourcePass(
    VRendererBackend* renderer, const std::string name)
    : RenderPass(renderer, name),
      m_imageViewSink(registerSink<imageview>("imageview")),
      m_renderPassSink(registerSink<renderpass>("renderpass")),
      m_framebufferSource(registerSource<framebuffer>("framebuffer")) {}

void VFramebufferCachedSourcePass::dispose() {
  for (auto const& [key, value] : m_cache) {
    m_vrenderer->destroyFramebuffer(value);
  }
}

void VFramebufferCachedSourcePass::recreate() {
  for (const auto& [key, framebuffer] : m_cache) {
    m_vrenderer->destroyFramebuffer(framebuffer);
  }
  m_cache.clear();
}

void VFramebufferCachedSourcePass::execute() {
  const imageview* p_imageView = getSinkResource<imageview>(m_imageViewSink);
  const renderpass* p_renderPass =
      getSinkResource<renderpass>(m_renderPassSink);
  std::pair<imageview, renderpass> key =
      std::make_pair(*p_imageView, *p_renderPass);

  if (m_cache.contains(key)) {
    setSourceResource(m_framebufferSource, &m_cache[key]);
  } else {
    const std::pair<u32, u32> dim =
        m_vrenderer->getImageViewDimensions(*p_imageView);
    m_cache[key] = m_vrenderer->createFramebuffer(*p_renderPass, *p_imageView,
                                                  dim.first, dim.second);
    setSourceResource(m_framebufferSource, &m_cache[key]);
  }
  // TODO: implement dynamic deallocation
}

}  // namespace sge::vulkan
