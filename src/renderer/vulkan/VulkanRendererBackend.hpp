#pragma once
#include <vulkan/vulkan.h>

#include <cstring>
#include <limits>
#include <optional>
#include <vector>

#include "renderer/RendererBackend.hpp"
#include "renderer/vulkan/VulkanAssertion.hpp"
#include "renderer/vulkan/VulkanMasterRendergraph.hpp"
#include "types/inttypes.hpp"
#include "window/Window.hpp"
#include "window/glfw/glfw.lib.hpp"

namespace sge::vulkan {

enum QueueFamilyType {
  QUEUE_FAMILY_GRAPHICS,
  QUEUE_FAMILY_TRANSFER,
  QUEUE_FAMILY_COMPUTE
};

class VulkanMasterRendergraph;

class VulkanRendererBackend : public sge::RendererBackend {
 public:
  VulkanRendererBackend(std::string application_name,
                        std::tuple<int, int, int> application_version,
                        std::string engine_name,
                        std::tuple<int, int, int> engine_version,
                        Window* window);
  ~VulkanRendererBackend() override;
  void beginFrame() override;
  void renderFrame() override;
  void endFrame() override;

  void recreateSwapchain();
  uint32_t createImageView(VkImage image, const u32 width, const u32 height,
                           VkFormat format);
  void destroyImageView(uint32_t imageViewId);
  const std::pair<u32, u32> getImageViewDimensions(const u32 imageViewId);

  uint32_t createShaderModule(const std::vector<char> source_code);
  void destroyShaderModule(uint32_t shader_module_id);
  uint32_t createRenderPass(const VkFormat color_format);
  void destroyRenderPass(uint32_t renderPassId);
  void beginRenderPass(uint32_t renderPassId, uint32_t framebufferId,
                       uint32_t renderAreaWidth, uint32_t renderAreaHeight,
                       uint32_t commandBufferId);
  void endRenderPass(uint32_t commandBufferId);
  uint32_t createPipelineLayout();
  void destroyPipelineLayout(uint32_t pipelineLayoutId);
  uint32_t createPipeline(uint32_t renderPassId, uint32_t pipelineLayoutId,
                          uint32_t viewportWidth, uint32_t viewportHeight,
                          std::optional<uint32_t> vertexShaderId,
                          std::optional<uint32_t> fragmentShaderId);
  void destroyPipeline(uint32_t pipelineId);
  void bindPipeline(uint32_t pipelineId, uint32_t commandBufferId);
  uint32_t createFramebuffer(uint32_t renderPassId, uint32_t imageViewId,
                             uint32_t width, uint32_t height);
  void destroyFramebuffer(uint32_t framebufferId);
  const std::pair<u32, u32> getFramebufferDimensions(const u32 framebufferId);

  uint32_t createCommandPool(QueueFamilyType queueFamily);
  void resetCommandPool(u32 commandPoolId);
  void destroyCommandPool(uint32_t commandPoolId);
  const std::vector<uint32_t> allocateCommandBuffers(uint32_t commandPoolId,
                                                     uint32_t count);
  void freeCommandBuffers(const std::vector<uint32_t>& commandBufferIds);
  void resetCommandBuffer(const u32 commandBufferId,
                          const boolean releaseResources = true);

  uint32_t createSemaphore();
  void beginCommandBuffer(uint32_t commandBufferId);
  void endCommandBuffer(uint32_t commandBufferId);
  void destroySemaphore(uint32_t semaphoreId);
  void drawCall(uint32_t vertexCount, uint32_t instanceCount,
                uint32_t commandBufferId);
  /// returns true if the swapchain KHR is deprecated (suboptional)
  // and a recreation of the swapchain is required.
  boolean acquireNextSwapchainFrame(u32 singalSem);
  u32 getCurrentSwapchainImageView();
  u32 getCurrentSwapchainFrameIndex();
  u32 getSwapchainCount() { return m_swapchainImageViews.size(); }
  boolean presentQueue(u32 queueId, const std::vector<u32>& waitSemaphoreId);
  void submitCommandBuffers(u32 queueId,
                            const std::vector<u32>& commandBufferIds,
                            const std::vector<u32>& waitSemaphoreIds,
                            const std::vector<u32>& signalSemaphoreIds,
                            const std::optional<u32> fence = std::nullopt);
  u32 createFence();
  void destroyFence(const u32 fenceId);
  void waitForFence(const u32 fenceId);
  void resetFence(const u32 fenceId);
  void waitDeviceIdle();
  u32 getAnyGraphicsQueue();
  u32 getAnyTransferQueue();
  u32 getAnyComputeQueue();
  Window* getWindowPtr() { return mp_window; }

 private:
  typedef struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
  } SwapChainSupportDetails;

 private:
  void createSwapchain();
  void updateSwapchainSupport();
  VkImageView& getImageViewById(const u32 imageViewId);
  VkPipelineLayout& getPipelineLayoutById(const u32 pipelineLayoutId);
  VkPipeline& getPipelineById(const u32 pipelineId);
  VkRenderPass& getRenderPassById(const u32 renderPassId);
  VkShaderModule& getShaderById(const u32 shaderId);
  VkFramebuffer& getFramebufferById(const u32 framebufferId);
  VkCommandPool& getCommandPoolById(const u32 commandPoolId);
  VkCommandBuffer& getCommandBufferById(const u32 commandBufferId);
  VkSemaphore& getSemaphoreById(const u32 semaphoreId);
  VkQueue& getQueueById(const u32 queueId);
  VkFence& getFenceById(const u32 fenceId);

 private:
  std::optional<u32> m_gfxQueueFamilyIndex;
  std::optional<u32> m_transferQueueFamilyIndex;
  std::optional<u32> m_computeQueueFamilyIndex;
  std::unique_ptr<VulkanMasterRendergraph> m_rendergraph;
  Window* mp_window;
  VkInstance m_instance;
  VkPhysicalDevice m_physicalDevice;
  VkDevice m_device;
  VkSurfaceKHR m_surface;
  SwapChainSupportDetails m_swapchainSupport;
  VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
  u32 m_swapchainFrameIndex;
  std::vector<uint32_t> m_swapchainImageViews;

  std::vector<std::pair<u32, u32>> m_queueIds;
  std::vector<VkQueue> m_graphics_queues;
  std::vector<VkQueue> m_transfer_queues;
  std::vector<VkQueue> m_compute_queues;

  std::vector<VkImageView> m_imageViews;
  std::vector<std::pair<u32, u32>> m_imageViewDimensions;
  std::vector<uint32_t> m_emptyImageViewIds;

  std::vector<VkShaderModule> m_shaders;
  std::vector<uint32_t> m_empty_shader_ids;
  std::vector<VkRenderPass> m_render_passes;
  std::vector<uint32_t> m_empty_render_pass_ids;
  std::vector<VkPipeline> m_pipelines;
  std::vector<uint32_t> m_empty_pipeline_ids;
  std::vector<VkPipelineLayout> m_pipelineLayouts;
  std::vector<uint32_t> m_emptyPipelineLayoutIds;
  std::vector<VkFramebuffer> m_framebuffers;
  std::vector<uint32_t> m_emptyFramebufferIds;
  std::vector<std::pair<u32, u32>> m_framebufferDimensions;
  std::vector<VkCommandPool> m_commandPools;
  std::vector<uint32_t> m_emptyCommandPoolIds;
  std::vector<VkCommandBuffer> m_commandBuffers;
  std::vector<uint32_t> m_emptyCommandBufferIds;
  std::vector<uint32_t> m_commandBufferIdToPoolId;
  std::vector<VkSemaphore> m_semaphores;
  std::vector<uint32_t> m_emptySemaphoreIds;

  std::vector<VkFence> m_fences;
  std::vector<u32> m_emptyFenceIds;

  static const VkFormat SURFACE_COLOR_FORMAT = VK_FORMAT_B8G8R8A8_UNORM;
  static const u32 MAX_SWAPCHAIN_IMAGES = 3;
};

}  // namespace sge::vulkan
