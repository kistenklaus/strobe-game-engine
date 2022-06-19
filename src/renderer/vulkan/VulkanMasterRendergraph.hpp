#pragma once

#include "renderer/Rendergraph.hpp"
#include "renderer/vulkan/VulkanDrawTrianglePass.hpp"
#include "renderer/vulkan/VulkanFramebufferDestPass.hpp"
#include "renderer/vulkan/VulkanFramebufferSrcPass.hpp"
#include "renderer/vulkan/VulkanRendererBackend.hpp"

namespace sge::vulkan {

class VulkanRendererBackend;

class VulkanMasterRendergraph : public Rendergraph {
 public:
  VulkanMasterRendergraph(VulkanRendererBackend* renderer);
  void beginFrame() override;
  void endFrame() override;

 private:
  const u32 m_imageAvaiableSemaphoreId;
  const u32 m_renderingDoneSemaphoreId;
  const u32 m_swapchain_src_pass_id;
  const u32 m_swapchain_dest_pass_id;
  const u32 m_triangle_pass_id;
};

}  // namespace sge::vulkan
