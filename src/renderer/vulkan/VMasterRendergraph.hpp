#pragma once

#include "logging/log.hpp"
#include "renderer/Rendergraph.hpp"
#include "renderer/vulkan/VRendererBackend.hpp"

namespace sge::vulkan {

class VMasterRendergraph : public Rendergraph {
 public:
  VMasterRendergraph(VRendererBackend* renderer);
  void recreate() override;

 private:
  class MasterRendergraphWindowSizeCallback : public WindowSizeCallback {
   public:
    MasterRendergraphWindowSizeCallback(VMasterRendergraph* p_rendergraph)
        : mp_rendergraph(p_rendergraph) {}
    void callback(const uint32_t width, const uint32_t height) override {
      // TODO avoid recreating twice on window size change
      mp_rendergraph->scheduleRecreation();
    }

   private:
    VMasterRendergraph* mp_rendergraph;
  };
  MasterRendergraphWindowSizeCallback m_windowSizeCallback;
  // resources.
  semaphore m_imageAvaiableSemaphoreHandle;
  semaphore m_renderingDoneSemaphoreHandle;

  // passes
  u32 m_acquireSwapchainFramePass;
  u32 m_renderPassSourcePass;
  u32 m_poolMultiSrcPass;
  u32 m_resetPoolPass;
  u32 m_allocateCmdBufferPass;
  u32 m_beginCmdBuffPass;
  u32 m_pipelineLayoutSourcePass;
  u32 m_framebufferCachedSourcePass;
  u32 m_trianglePass;
  u32 m_endCmdBuffPass;
  u32 m_acquireQueuePass;
  u32 m_submitPass;
  u32 m_presentQueuePass;
  u32 m_wrapWithVectorPass;
  u32 m_vectorAtIndexPass;
  u32 m_fenceMultiSrcPass;
  u32 m_selectFencePass;
  u32 m_resetCmdBuffPass;
  u32 m_selectCmdBuffPass;
  u32 m_cmdbufferMultiSrcPass;
  u32 m_poolSrcPass;
};

}  // namespace sge::vulkan
