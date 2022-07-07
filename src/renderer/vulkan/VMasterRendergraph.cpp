#include "renderer/vulkan/VMasterRendergraph.hpp"

#include <vulkan/vulkan.h>

#include "logging/print.hpp"
#include "renderer/passes/VectorAtIndexPass.hpp"
#include "renderer/passes/WrapWithVectorPass.hpp"
#include "renderer/vulkan/VAssertion.hpp"
#include "renderer/vulkan/passes/VAcquireQueuePass.hpp"
#include "renderer/vulkan/passes/VAcquireSwapchainFrame.hpp"
#include "renderer/vulkan/passes/VAllocateCmdBuffPass.hpp"
#include "renderer/vulkan/passes/VBeginCmdBuffPass.hpp"
#include "renderer/vulkan/passes/VCmdBuffMSrcPass.hpp"
#include "renderer/vulkan/passes/VCmdPoolMultiSrcPass.hpp"
#include "renderer/vulkan/passes/VCmdPoolSrcPass.hpp"
#include "renderer/vulkan/passes/VEndCmdBuffPass.hpp"
#include "renderer/vulkan/passes/VFenceMultiSrcPass.hpp"
#include "renderer/vulkan/passes/VFramebufferCachedSourcePass.hpp"
#include "renderer/vulkan/passes/VPipelineLayoutSourcePass.hpp"
#include "renderer/vulkan/passes/VPresentQueuePass.hpp"
#include "renderer/vulkan/passes/VRenderPassSourcePass.hpp"
#include "renderer/vulkan/passes/VResetCmdBuffPass.hpp"
#include "renderer/vulkan/passes/VResetCmdPoolPass.hpp"
#include "renderer/vulkan/passes/VSubmitCmdBufferPass.hpp"
#include "renderer/vulkan/passes/VTriangleRenderPass.hpp"

namespace sge::vulkan {

VMasterRendergraph::VMasterRendergraph(VRendererBackend* renderer)
    : Rendergraph(reinterpret_cast<RendererBackend*>(renderer),
                  "master-rendergraph"),
      m_imageAvaiableSemaphoreHandle(m_vrenderer->createSemaphore()),
      m_renderingDoneSemaphoreHandle(m_vrenderer->createSemaphore()),
      m_acquireSwapchainFramePass(
          createPass<VAcquireSwapchainFrame>(renderer, "acq-swapchain-frame")),
      m_renderPassSourcePass(createPass<VRenderPassSourcePass>(
          renderer, "renderpass-src", VK_FORMAT_B8G8R8A8_UNORM)),
      // m_poolMultiSrcPass(createPass<VCmdPoolMultiSrcPass>(
      //     renderer, "cmdpool-multi-src", QUEUE_FAMILY_GRAPHICS,
      //     m_vrenderer->getSwapchainCount())),
      // m_resetPoolPass(
      //     createPass<VResetCmdPoolPass>(renderer, "reset-cmdpool")),
      // m_allocateCmdBufferPass(
      //     createPass<VAllocateCmdBuffPass>(renderer,
      //     "alloc-cmdbuffer")),
      m_beginCmdBuffPass(
          createPass<VBeginCmdBuffPass>(renderer, "begin-cmdbuffer")),
      m_pipelineLayoutSourcePass(createPass<VPipelineLayoutSourcePass>(
          renderer, "pipelinelayout-src")),
      m_framebufferCachedSourcePass(createPass<VFramebufferCachedSourcePass>(
          renderer, "framebuffer-cached-src")),
      m_trianglePass(
          createPass<VTriangleRenderPass>(renderer, "triangle-pass")),
      m_endCmdBuffPass(createPass<VEndCmdBuffPass>(renderer, "end-cmdbuffer")),
      m_acquireQueuePass(createPass<VAcquireQueuePass>(renderer, "acq-gfxqueue",
                                                       GRAPHICS_QUEUE)),
      m_submitPass(
          createPass<VSubmitCmdBufferPass>(renderer, "submit-cmdbuffer")),
      m_presentQueuePass(
          createPass<VPresentQueuePass>(renderer, "present-queue")),
      m_fenceMultiSrcPass(createPass<VFenceMultiSrcPass>(
          renderer, "fence-multi-src", m_vrenderer->getSwapchainCount())),
      m_selectFencePass(
          createPass<VectorAtIndexPass<fence>>(renderer, "select-fence")),
      m_poolSrcPass(createPass<VCmdPoolSrcPass>(renderer, "cmdpool-src",
                                                QUEUE_FAMILY_GRAPHICS)),
      m_cmdbufferMultiSrcPass(createPass<VCmdBuffMSrcPass>(
          renderer, "cmdbuffer-multi-src", m_vrenderer->getSwapchainCount())),
      m_selectCmdBuffPass(createPass<VectorAtIndexPass<command_buffer>>(
          renderer, "select-cmdbuffer")),
      m_resetCmdBuffPass(
          createPass<VResetCmdBuffPass>(renderer, "reset-cmdbuffer")),
      m_windowSizeCallback(MasterRendergraphWindowSizeCallback(this)) {
  markPassAsRoot(m_presentQueuePass);

  // cmd-buffer route
  //----------------
  // fence src.
  addLinkage("fence-multi-src@fences", "select-fence@vector");
  addLinkage("acq-swapchain-frame@frameindex", "select-fence@index");

  // framebuffer @ renderpass src
  addLinkage("renderpass-src@renderpass", "framebuffer-cached-src@renderpass");
  addLinkage("acq-swapchain-frame@imageview",
             "framebuffer-cached-src@imageview");

  // cmdbuffer src
  addLinkage("cmdpool-src@cmdpool", "cmdbuffer-multi-src@cmdpool");
  addLinkage("cmdbuffer-multi-src@cmdbuffers", "select-cmdbuffer@vector");
  addLinkage("acq-swapchain-frame@frameindex", "select-cmdbuffer@index");
  addLinkage("select-cmdbuffer@value", "reset-cmdbuffer@cmdbuffer");
  addLinkage("select-fence@value", "reset-cmdbuffer@fence");

  addLinkage("reset-cmdbuffer@cmdbuffer", "begin-cmdbuffer@cmdbuffer");

  addLinkage("begin-cmdbuffer@cmdbuffer", "triangle-pass@cmdbuffer");
  addLinkage("framebuffer-cached-src@framebuffer", "triangle-pass@framebuffer");
  addLinkage("pipelinelayout-src@pipelinelayout",
             "triangle-pass@pipelinelayout");
  addLinkage("renderpass-src@renderpass", "triangle-pass@renderpass");

  addLinkage("triangle-pass@cmdbuffer", "end-cmdbuffer@cmdbuffer");

  addLinkage("acq-swapchain-frame@signal", "submit-cmdbuffer@wait");
  addLinkage("reset-cmdbuffer@fence", "submit-cmdbuffer@fence");
  addLinkage("end-cmdbuffer@cmdbuffer", "submit-cmdbuffer@cmdbuffer");
  addLinkage("acq-gfxqueue@queue", "submit-cmdbuffer@queue");

  addLinkage("submit-cmdbuffer@signal", "present-queue@wait");
  addLinkage("acq-gfxqueue@queue", "present-queue@queue");

  m_vrenderer->getWindowPtr()->addWindowSizeCallback(&m_windowSizeCallback);
  Rendergraph::create();
}

void VMasterRendergraph::recreate() {
  m_vrenderer->recreateSwapchain();
  Rendergraph::recreate();
}

}  // namespace sge::vulkan
