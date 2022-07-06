#include "renderer/vulkan/VulkanMasterRendergraph.hpp"

#include <vulkan/vulkan.h>

#include "logging/print.hpp"
#include "renderer/passes/VectorAtIndexPass.hpp"
#include "renderer/passes/WrapWithVectorPass.hpp"
#include "renderer/vulkan/VulkanAssertion.hpp"
#include "renderer/vulkan/passes/VulkanAcquireQueuePass.hpp"
#include "renderer/vulkan/passes/VulkanAcquireSwapchainFrame.hpp"
#include "renderer/vulkan/passes/VulkanAllocateCmdBuffPass.hpp"
#include "renderer/vulkan/passes/VulkanBeginCmdBuffPass.hpp"
#include "renderer/vulkan/passes/VulkanCmdBuffMSrcPass.hpp"
#include "renderer/vulkan/passes/VulkanCmdPoolMultiSrcPass.hpp"
#include "renderer/vulkan/passes/VulkanCmdPoolSrcPass.hpp"
#include "renderer/vulkan/passes/VulkanEndCmdBuffPass.hpp"
#include "renderer/vulkan/passes/VulkanFenceMultiSrcPass.hpp"
#include "renderer/vulkan/passes/VulkanFramebufferCachedSourcePass.hpp"
#include "renderer/vulkan/passes/VulkanPipelineLayoutSourcePass.hpp"
#include "renderer/vulkan/passes/VulkanPresentQueuePass.hpp"
#include "renderer/vulkan/passes/VulkanRenderPassSourcePass.hpp"
#include "renderer/vulkan/passes/VulkanResetCmdBuffPass.hpp"
#include "renderer/vulkan/passes/VulkanResetCmdPoolPass.hpp"
#include "renderer/vulkan/passes/VulkanSubmitCmdBufferPass.hpp"
#include "renderer/vulkan/passes/VulkanTriangleRenderPass.hpp"

namespace sge::vulkan {

VulkanMasterRendergraph::VulkanMasterRendergraph(VRendererBackend* renderer)
    : Rendergraph(reinterpret_cast<RendererBackend*>(renderer),
                  "master-rendergraph"),
      m_imageAvaiableSemaphoreId(m_vrenderer->createSemaphore()),
      m_renderingDoneSemaphoreId(m_vrenderer->createSemaphore()),
      m_acquireSwapchainFramePass(createPass<VulkanAcquireSwapchainFrame>(
          renderer, "acq-swapchain-frame")),
      m_renderPassSourcePass(createPass<VulkanRenderPassSourcePass>(
          renderer, "renderpass-src", VK_FORMAT_B8G8R8A8_UNORM)),
      // m_poolMultiSrcPass(createPass<VulkanCmdPoolMultiSrcPass>(
      //     renderer, "cmdpool-multi-src", QUEUE_FAMILY_GRAPHICS,
      //     m_vrenderer->getSwapchainCount())),
      // m_resetPoolPass(
      //     createPass<VulkanResetCmdPoolPass>(renderer, "reset-cmdpool")),
      // m_allocateCmdBufferPass(
      //     createPass<VulkanAllocateCmdBuffPass>(renderer,
      //     "alloc-cmdbuffer")),
      m_beginCmdBuffPass(
          createPass<VulkanBeginCmdBuffPass>(renderer, "begin-cmdbuffer")),
      m_pipelineLayoutSourcePass(createPass<VulkanPipelineLayoutSourcePass>(
          renderer, "pipelinelayout-src")),
      m_framebufferCachedSourcePass(
          createPass<VulkanFramebufferCachedSourcePass>(
              renderer, "framebuffer-cached-src")),
      m_trianglePass(
          createPass<VulkanTriangleRenderPass>(renderer, "triangle-pass")),
      m_endCmdBuffPass(
          createPass<VulkanEndCmdBuffPass>(renderer, "end-cmdbuffer")),
      m_acquireQueuePass(createPass<VulkanAcquireQueuePass>(
          renderer, "acq-gfxqueue", GRAPHICS_QUEUE)),
      m_submitPass(
          createPass<VulkanSubmitCmdBufferPass>(renderer, "submit-cmdbuffer")),
      m_presentQueuePass(
          createPass<VulkanPresentQueuePass>(renderer, "present-queue")),
      // m_wrapWithVectorPass(
      //     createPass<WrapWithVectorPass<u32>>(renderer, "wrap-with-vector")),
      // m_vectorAtIndexPass(
      //     createPass<VectorAtIndexPass<u32>>(renderer, "select-pool")),
      m_fenceMultiSrcPass(createPass<VulkanFenceMultiSrcPass>(
          renderer, "fence-multi-src", m_vrenderer->getSwapchainCount())),
      m_selectFencePass(
          createPass<VectorAtIndexPass<u32>>(renderer, "select-fence")),
      m_poolSrcPass(createPass<VulkanCmdPoolSrcPass>(renderer, "cmdpool-src",
                                                     QUEUE_FAMILY_GRAPHICS)),
      m_cmdbufferMultiSrcPass(createPass<VulkanCmdBuffMSrcPass>(
          renderer, "cmdbuffer-multi-src", m_vrenderer->getSwapchainCount())),
      m_selectCmdBuffPass(
          createPass<VectorAtIndexPass<u32>>(renderer, "select-cmdbuffer")),
      m_resetCmdBuffPass(
          createPass<VulkanResetCmdBuffPass>(renderer, "reset-cmdbuffer")),
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

void VulkanMasterRendergraph::recreate() {
  m_vrenderer->recreateSwapchain();
  Rendergraph::recreate();
}

}  // namespace sge::vulkan
