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

 private:
  const u32 m_imageAvaiableSemaphoreId;
  const u32 m_renderingDoneSemaphoreId;

  const u32 m_acquireSwapchainFramePass;
  const u32 m_acquireQueuePass;
  const u32 m_submitPass;
  const u32 m_wrapWithVectorPass;
  const u32 m_presentQueuePass;
};

}  // namespace sge::vulkan
