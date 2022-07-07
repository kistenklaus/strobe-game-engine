#include "renderer/vulkan/passes/VFramebufferCachedSourcePass.hpp"

#include "logging//print.hpp"

namespace sge::vulkan {

VFramebufferCachedSourcePass::VFramebufferCachedSourcePass(
    VRendererBackend* renderer, const std::string name)
    : RenderPass(renderer, name),
      m_imageViewSink(sink<imageview>("imageview")),
      m_renderPassSink(sink<renderpass>("renderpass")),
      m_framebufferSource(source<framebuffer>("framebuffer")) {
  registerSink(&m_imageViewSink);
  registerSink(&m_renderPassSink);
  registerSource(&m_framebufferSource);
}

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
  std::pair<imageview, renderpass> key =
      std::make_pair(*m_imageViewSink, *m_renderPassSink);

  if (m_cache.contains(key)) {
    m_framebufferSource.set(&m_cache[key]);
  } else {
    const std::pair<u32, u32> dim =
        m_vrenderer->getImageViewDimensions(*m_imageViewSink);
    m_cache[key] = m_vrenderer->createFramebuffer(
        *m_renderPassSink, *m_imageViewSink, dim.first, dim.second);
    m_framebufferSource.set(&m_cache[key]);
  }
  // TODO: implement dynamic deallocation
}

}  // namespace sge::vulkan
