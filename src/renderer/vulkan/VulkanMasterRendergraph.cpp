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
#include "renderer/vulkan/passes/VulkanCmdPoolMultiSrcPass.hpp"
#include "renderer/vulkan/passes/VulkanCmdPoolSrcPass.hpp"
#include "renderer/vulkan/passes/VulkanEndCmdBuffPass.hpp"
#include "renderer/vulkan/passes/VulkanFenceMultiSrcPass.hpp"
#include "renderer/vulkan/passes/VulkanFramebufferCachedSourcePass.hpp"
#include "renderer/vulkan/passes/VulkanPipelineLayoutSourcePass.hpp"
#include "renderer/vulkan/passes/VulkanPresentQueuePass.hpp"
#include "renderer/vulkan/passes/VulkanRenderPassSourcePass.hpp"
#include "renderer/vulkan/passes/VulkanResetCmdPoolPass.hpp"
#include "renderer/vulkan/passes/VulkanSubmitCmdBufferPass.hpp"
#include "renderer/vulkan/passes/VulkanTriangleRenderPass.hpp"

namespace sge::vulkan {

VulkanMasterRendergraph::VulkanMasterRendergraph(
    VulkanRendererBackend* renderer)
    : Rendergraph(reinterpret_cast<RendererBackend*>(renderer),
                  "master-rendergraph"),
      m_imageAvaiableSemaphoreId(m_vrenderer->createSemaphore()),
      m_renderingDoneSemaphoreId(m_vrenderer->createSemaphore()),
      m_acquireSwapchainFramePass(createPass<VulkanAcquireSwapchainFrame>(
          renderer, "acq-swapchain-frame")),
      m_renderPassSourcePass(createPass<VulkanRenderPassSourcePass>(
          renderer, "renderpass-src", VK_FORMAT_B8G8R8A8_UNORM)),
      m_poolMultiSrcPass(createPass<VulkanCmdPoolMultiSrcPass>(
          renderer, "cmdpool-multi-src", QUEUE_FAMILY_GRAPHICS,
          m_vrenderer->getSwapchainCount())),
      m_resetPoolPass(
          createPass<VulkanResetCmdPoolPass>(renderer, "reset-cmdpool")),
      m_allocateCmdBufferPass(
          createPass<VulkanAllocateCmdBuffPass>(renderer, "alloc-cmdbuffer")),
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
      m_wrapWithVectorPass(
          createPass<WrapWithVectorPass<u32>>(renderer, "wrap-with-vector")),
      m_vectorAtIndexPass(
          createPass<VectorAtIndexPass<u32>>(renderer, "select-pool")),
      m_fenceMultiSrcPass(createPass<VulkanFenceMultiSrcPass>(
          renderer, "fence-multi-src", m_vrenderer->getSwapchainCount())),
      m_selectFencePass(
          createPass<VectorAtIndexPass<u32>>(renderer, "select-fence")),
      m_windowSizeCallback(MasterRendergraphWindowSizeCallback(this)) {
  markPassAsRoot(m_presentQueuePass);

  // cmd-buffer route
  addLinkage("cmdpool-multi-src@cmdpools", "select-pool@vector");
  addLinkage("select-pool@value", "reset-cmdpool@cmdpool");
  addLinkage("reset-cmdpool@cmdpool", "alloc-cmdbuffer@cmdpool");
  addLinkage("alloc-cmdbuffer@cmdbuffer", "begin-cmdbuffer@cmdbuffer");
  addLinkage("begin-cmdbuffer@cmdbuffer", "triangle-pass@cmdbuffer");
  addLinkage("triangle-pass@cmdbuffer", "end-cmdbuffer@cmdbuffer");
  addLinkage("end-cmdbuffer@cmdbuffer", "wrap-with-vector@value");
  addLinkage("wrap-with-vector@vector", "submit-cmdbuffer@cmdbuffer");

  // swapchain framebuffer route
  addLinkage("acq-swapchain-frame@imageview",
             "framebuffer-cached-src@imageview");
  addLinkage("framebuffer-cached-src@framebuffer", "triangle-pass@framebuffer");

  // swapchain frame-index route.
  addLinkage("acq-swapchain-frame@frameindex", "select-pool@index");

  // renderpass route
  addLinkage("renderpass-src@renderpass", "framebuffer-cached-src@renderpass");
  addLinkage("renderpass-src@renderpass", "triangle-pass@renderpass");

  // pipeline layout route.
  addLinkage("pipelinelayout-src@pipelinelayout",
             "triangle-pass@pipelinelayout");

  // queue route.
  addLinkage("acq-gfxqueue@queue", "submit-cmdbuffer@queue");
  addLinkage("acq-gfxqueue@queue", "present-queue@queue");

  // semaphores.
  addLinkage("acq-swapchain-frame@signal", "submit-cmdbuffer@wait");
  addLinkage("submit-cmdbuffer@signal", "present-queue@wait");

  addLinkage("fence-multi-src@fences", "select-fence@vector");
  addLinkage("acq-swapchain-frame@frameindex", "select-fence@index");
  addLinkage("select-fence@value", "reset-cmdpool@fence");
  addLinkage("reset-cmdpool@fence", "submit-cmdbuffer@fence");

  m_vrenderer->getWindowPtr()->addWindowSizeCallback(&m_windowSizeCallback);
}

void VulkanMasterRendergraph::recreate() {
  m_vrenderer->recreateSwapchain();
  Rendergraph::recreate();
}

}  // namespace sge::vulkan
