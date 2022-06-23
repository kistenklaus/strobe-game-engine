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
class VulkanMasterRendergraph;

class VulkanRendererBackend : public sge::RendererBackend {
 public:
  VulkanRendererBackend(std::string application_name,
                        std::tuple<int, int, int> application_version,
                        std::string engine_name,
                        std::tuple<int, int, int> engine_version,
                        const Window& window);
  ~VulkanRendererBackend() override;
  void beginFrame() override;
  void renderFrame() override;
  void endFrame() override;

  uint32_t createImageView(VkImage image, VkFormat format);
  void destroyImageView(uint32_t imageViewId);

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
  uint32_t createCommandPool(uint32_t queueFamilyIndex);
  void destroyCommandPool(uint32_t commandPoolId);
  const std::vector<uint32_t> allocateCommandBuffers(uint32_t commandPoolId,
                                                     uint32_t count);
  void freeCommandBuffers(const std::vector<uint32_t>& commandBufferIds);
  uint32_t createSemaphore();
  void beginCommandBuffer(uint32_t commandBufferId);
  void endCommandBuffer(uint32_t commandBufferId);
  void destroySemaphore(uint32_t semaphoreId);
  void drawCall(uint32_t vertexCount, uint32_t instanceCount,
                uint32_t commandBufferId);
  void acquireNextSwapchainFrame(u32 singalSem);
  void presentQueue(u32 queueId, const std::vector<u32>& waitSemaphoreId);
  u32 getAnyGraphicsQueue();
  u32 getAnyTransferQueue();
  u32 getAnyComputeQueue();

 private:
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

 private:
  std::unique_ptr<VulkanMasterRendergraph> m_rendergraph;
  VkInstance m_instance;
  VkDevice m_device;
  VkSurfaceKHR m_surface;
  VkSwapchainKHR m_swapchain;
  u32 m_swapchainFrameIndex;
  std::vector<uint32_t> m_swapchainImageViews;

  std::vector<std::pair<u32, u32>> m_queueIds;
  std::vector<VkQueue> m_graphics_queues;
  std::vector<VkQueue> m_transfer_queues;
  std::vector<VkQueue> m_compute_queues;

  std::vector<VkImageView> m_imageViews;
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
  std::vector<VkCommandPool> m_commandPools;
  std::vector<uint32_t> m_emptyCommandPoolIds;
  std::vector<VkCommandBuffer> m_commandBuffers;
  std::vector<uint32_t> m_emptyCommandBufferIds;
  std::vector<uint32_t> m_commandBufferIdToPoolId;
  std::vector<VkSemaphore> m_semaphores;
  std::vector<uint32_t> m_emptySemaphoreIds;

  static const VkFormat SURFACE_COLOR_FORMAT = VK_FORMAT_B8G8R8A8_UNORM;
  static const u32 MAX_SWAPCHAIN_IMAGES = 3;
};

}  // namespace sge::vulkan
