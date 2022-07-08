#pragma once
#include <vulkan/vulkan.h>

#include <cstring>
#include <limits>
#include <map>
#include <optional>
#include <vector>

#include "renderer/RenderPass.hpp"
#include "renderer/RendererBackend.hpp"
#include "renderer/vulkan/VAssertion.hpp"
#include "renderer/vulkan/handles.hpp"
#include "types/inttypes.hpp"
#include "types/sarray.hpp"
#include "window/Window.hpp"
#include "window/glfw/glfw.lib.hpp"

namespace sge::vulkan {

enum QueueFamilyType {
  QUEUE_FAMILY_GRAPHICS,
  QUEUE_FAMILY_TRANSFER,
  QUEUE_FAMILY_COMPUTE
};

enum ShaderType { SHADER_TYPE_VERTEX, SHADER_TYPE_FRAGMENT };

class VulkanMasterRendergraph;

class VRendererBackend : public sge::RendererBackend {
 public:
  VRendererBackend(std::string& application_name,
                   std::tuple<u32, u32, u32>& application_version,
                   std::string& engine_name,
                   std::tuple<u32, u32, u32>& engine_version, Window* window);
  ~VRendererBackend() override;
  void beginFrame() override;
  void renderFrame() override;
  void endFrame() override;

  void recreateSwapchain();
  imageview createImageView(VkImage image, const u32 width, const u32 height,
                            VkFormat format);
  void destroyImageView(imageview imageViewHandle);
  const std::pair<u32, u32> getImageViewDimensions(
      const imageview imageview_handle);

  shader_module createShaderModule(const std::string path,
                                   ShaderType shaderType);
  void destroyShaderModule(shader_module shaderModuleHandle);
  renderpass createRenderPass(const VkFormat colorFormat);
  void destroyRenderPass(renderpass renderPassHandle);
  void beginRenderPass(renderpass renderPassHandle,
                       framebuffer framebufferHandle, u32 renderAreaWidth,
                       u32 renderAreaHeight,
                       command_buffer commandBufferHandle);
  void endRenderPass(command_buffer commandBufferHandle);
  pipeline_layout createPipelineLayout();
  void destroyPipelineLayout(pipeline_layout pipelineLayoutHandle);
  pipeline createPipeline(renderpass renderPassHandle,
                          pipeline_layout pipelineLayoutHandle,
                          uint32_t viewportWidth, uint32_t viewportHeight,
                          std::optional<shader_module> vertexShaderHandle,
                          std::optional<shader_module> fragmentShaderHandle);
  void destroyPipeline(pipeline pipelineHandle);
  void bindPipeline(pipeline pipelineHandle,
                    command_buffer commandBufferHandle);
  framebuffer createFramebuffer(renderpass renderPassHandle,
                                imageview imageViewHandle, u32 width,
                                u32 height);
  void destroyFramebuffer(framebuffer framebufferHandle);
  const std::pair<u32, u32> getFramebufferDimensions(
      const framebuffer framebufferHandle);

  command_pool createCommandPool(QueueFamilyType queueFamily);
  void resetCommandPool(command_pool commandPoolHandle);
  void destroyCommandPool(command_pool commandPoolHandle);
  const std::vector<command_buffer> allocateCommandBuffers(
      command_pool commandPoolHandle, u32 count);
  void freeCommandBuffers(
      const std::vector<command_buffer>& commandBufferHandles);
  void resetCommandBuffer(const command_buffer commandBufferHandle,
                          const boolean releaseResources = true);

  semaphore createSemaphore();
  void beginCommandBuffer(command_buffer commandBufferHandle);
  void endCommandBuffer(command_buffer commandBufferHandle);
  void destroySemaphore(semaphore semaphoreHandle);
  void drawCall(u32 vertexCount, u32 instanceCount,
                command_buffer commandBufferHandle);
  /// returns true if the swapchain KHR is deprecated (suboptional)
  // and a recreation of the swapchain is required.
  boolean acquireNextSwapchainFrame(semaphore singalSemaphoreHandle);
  imageview getCurrentSwapchainImageView();
  u32 getCurrentSwapchainFrameIndex();
  u32 getSwapchainCount() { return m_swapchain.count(); }
  boolean presentQueue(queue queueId,
                       const std::vector<semaphore>& waitSemaphoreId);
  void submitCommandBuffers(
      queue queueHandle,
      const std::vector<command_buffer>& commandBufferHandles,
      const std::vector<semaphore>& waitSemaphoreHandles,
      const std::vector<semaphore>& signalSemaphoreHandles,
      const std::optional<fence> fenceHandle = std::nullopt);
  fence createFence();
  void destroyFence(const fence fenceHandle);
  void waitForFence(const fence fenceHandle);
  void resetFence(const fence fenceHandle);
  void waitDeviceIdle();
  queue getAnyGraphicsQueue();
  queue getAnyTransferQueue();
  queue getAnyComputeQueue();
  Window* getWindowPtr() { return mp_window; }

 private:
  struct instance_t {
    VkInstance m_handle;
  };
  struct queue_t {
    VkQueue m_handle;
    QueueFamilyType m_type;
    u32 m_index;
  };
  struct device_t {
    VkDevice m_handle;
    VkPhysicalDevice m_physicalDevice;
    std::optional<u32> m_gfxQueueFamilyIndex;
    std::optional<u32> m_transferQueueFamilyIndex;
    std::optional<u32> m_computeQueueFamilyIndex;
  };
  struct surface_t {
    VkSurfaceKHR m_handle;
    VkSurfaceCapabilitiesKHR m_capabilities;
    std::vector<VkSurfaceFormatKHR> m_formats;
    std::vector<VkPresentModeKHR> m_presentModes;
  };
  struct imageview_t {
    VkImageView m_handle;
    u32 m_width;
    u32 m_height;
    u32 m_index;
  };
  struct swapchain_t {
    VkSwapchainKHR m_handle;
    u32 m_frameIndex;
    std::vector<imageview> m_imageViewsHandles;
    inline u32 count() { return m_imageViewsHandles.size(); }
  };
  struct pipeline_t {
    VkPipeline m_handle;
    u32 m_index;
  };
  struct pipeline_layout_t {
    VkPipelineLayout m_handle;
    u32 m_index;
  };
  struct renderpass_t {
    VkRenderPass m_handle;
    u32 m_index;
  };
  struct framebuffer_t {
    VkFramebuffer m_handle;
    imageview m_imageview;
    u32 m_width;
    u32 m_height;
    u32 m_index;
  };
  struct vertex_shader_input_layout_t {
    //
  };
  struct shader_module_t {
    VkShaderModule m_handle;
    ShaderType m_type;
    u32 m_refCount;
    u32 m_index;
    std::optional<vertex_shader_input_layout_t> m_layout;
  };
  struct semaphore_t {
    VkSemaphore m_handle;
    u32 m_index;
  };
  struct command_pool_t {
    VkCommandPool m_handle;
    u32 m_index;
  };
  struct command_buffer_t {
    VkCommandBuffer m_handle;
    command_pool m_origin_pool;
    u32 m_index;
  };
  struct fence_t {
    VkFence m_handle;
    u32 m_index;
  };

 private:
  void createSwapchain();
  void updateSwapchainSupport();
  imageview_t& getImageViewByHandle(const imageview imageViewId);
  pipeline_layout_t& getPipelineLayoutByHandle(
      const pipeline_layout pipelineLayoutId);
  pipeline_t& getPipelineByHandle(const pipeline pipelineId);
  renderpass_t& getRenderPassByHandle(const renderpass renderPassId);
  shader_module_t& getShaderByHandle(const shader_module shaderId);
  framebuffer_t& getFramebufferByHandle(const framebuffer framebufferId);
  command_pool_t& getCommandPoolByHandle(const command_pool commandPoolId);
  command_buffer_t& getCommandBufferByHandle(
      const command_buffer commandBufferId);
  semaphore_t& getSemaphoreByHandle(const semaphore semaphoreId);
  queue_t& getQueueByHandle(const queue queueId);
  fence_t& getFenceByHandle(const fence fenceId);
  instance_t createInstance(const std::string& applicationName,
                            const std::tuple<u32, u32, u32>& applicationVersion,
                            const std::string& engineName,
                            const std::tuple<u32, u32, u32>& engineVersion);
  device_t createDevice();
  surface_t createSurface();
  void destroyCommandPool(command_pool_t& commandPool);
  void destroySemaphore(semaphore_t& semaphore);
  void destroyPipeline(pipeline_t& pipeline);
  void destroyFramebuffer(framebuffer_t& framebuffer);
  void destroyPipelineLayout(pipeline_layout_t& pipelineLayout);
  void destroyShaderModule(shader_module_t& shaderModule);
  void destroyRenderPass(renderpass_t& renderpass);
  void destroyImageView(imageview_t& imageview);
  void destroySwapchain(swapchain_t& swapchain);
  void destroySurface(surface_t& surface);
  void destroyDevice(device_t& device);
  void destroyInstance(instance_t& instance);
  void destroyFence(fence_t& fence);
  void bindPipeline(pipeline_t& pipeline, command_buffer_t& commandBuffer);

 private:
  RenderPass* m_rootPass;

  Window* mp_window;
  instance_t m_instance;
  device_t m_device;
  surface_t m_surface;
  swapchain_t m_swapchain{VK_NULL_HANDLE};

  sarray<queue_t> m_queues;
  std::map<std::string, shader_module> m_loadedShaders;
  sarray<imageview_t> m_imageViews;
  sarray<shader_module_t> m_shaders;
  sarray<renderpass_t> m_renderpasses;
  sarray<pipeline_t> m_pipelines;
  sarray<pipeline_layout_t> m_pipelineLayouts;
  sarray<framebuffer_t> m_framebuffers;
  sarray<command_pool_t> m_commandPools;
  sarray<command_buffer_t> m_commandBuffers;
  sarray<semaphore_t> m_semaphores;
  sarray<fence_t> m_fences;

  static const VkFormat SURFACE_COLOR_FORMAT = VK_FORMAT_B8G8R8A8_UNORM;
  static const u32 MAX_SWAPCHAIN_IMAGES = 3;
};

}  // namespace sge::vulkan
