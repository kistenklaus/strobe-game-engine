#include "renderer/vulkan/passes/VulkanFramebufferCachedSourcePass.hpp"

namespace sge::vulkan {

VulkanFramebufferCachedSourcePass::VulkanFramebufferCachedSourcePass(
    VulkanRendererBackend* renderer, const std::string name)
    : RenderPass(renderer, name),
      m_imageViewSink(registerSink<u32>("imageview")),
      m_renderPassSink(registerSink<u32>("renderpass")),
      m_framebufferSource(registerSource<u32>("framebuffer")) {}

VulkanFramebufferCachedSourcePass::~VulkanFramebufferCachedSourcePass() {
  for (auto const& [key, value] : m_cache) {
    m_vrenderer->destroyFramebuffer(value);
  }
}

void VulkanFramebufferCachedSourcePass::execute() {
  const u32* p_imageView = getSinkResource<u32>(m_imageViewSink);
  const u32* p_renderPass = getSinkResource<u32>(m_renderPassSink);
  std::pair<u32, u32> key = std::make_pair(*p_imageView, *p_renderPass);

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
