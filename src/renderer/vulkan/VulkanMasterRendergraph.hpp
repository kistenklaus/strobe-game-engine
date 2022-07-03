#pragma once

#include "logging/log.hpp"
#include "renderer/Rendergraph.hpp"
#include "renderer/vulkan/VulkanRendererBackend.hpp"

namespace sge::vulkan {

class VulkanRendererBackend;

class VulkanMasterRendergraph : public Rendergraph {
 public:
  VulkanMasterRendergraph(VulkanRendererBackend* renderer);

 private:
  // resources.
  const u32 m_imageAvaiableSemaphoreId;
  const u32 m_renderingDoneSemaphoreId;

  // passes
  const u32 m_acquireSwapchainFramePass;
  const u32 m_renderPassSourcePass;
  const u32 m_poolMultiSrcPass;
  const u32 m_resetPoolPass;
  const u32 m_allocateCmdBufferPass;
  const u32 m_beginCmdBuffPass;
  const u32 m_pipelineLayoutSourcePass;
  const u32 m_framebufferCachedSourcePass;
  const u32 m_trianglePass;
  const u32 m_endCmdBuffPass;
  const u32 m_acquireQueuePass;
  const u32 m_submitPass;
  const u32 m_presentQueuePass;
  const u32 m_wrapWithVectorPass;
  const u32 m_vectorAtIndexPass;
};

}  // namespace sge::vulkan
