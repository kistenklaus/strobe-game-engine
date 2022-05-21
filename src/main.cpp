#include "strb/apis/glfw.hpp"
#include "strb/apis/vulkan.hpp"
#include "strb/fileio.hpp"
#include "strb/stl.hpp"
#include "strb/window.hpp"

const strb::tuple3i APP_VERSION = {0, 0, 0};
const strb::tuple3i ENGINE_VERSION = {0, 0, 0};
const strb::tuple3i VULKAN_API_VERSION = {1, 1, 0};

static void recordCommandBuffer(strb::vulkan::CommandBuffer &commandBuffer,
                                strb::vulkan::RenderPass &renderPass,
                                strb::vulkan::Framebuffer &framebuffer,
                                strb::vulkan::Pipeline &pipeline,
                                uint32_t width, uint32_t height) {
  commandBuffer.begin();
  renderPass.begin(commandBuffer, framebuffer, 0, 0, width, height);
  pipeline.bind(commandBuffer);
  pipeline.executeDrawCall(commandBuffer, 3, 1, 0, 0);
  renderPass.end(commandBuffer);
  commandBuffer.end();
}

class WSizeCallback : public strb::window::WindowSizeCallback {
private:
  strb::vulkan::Device &device;
  strb::vulkan::Swapchain &swapchain;
  strb::vulkan::RenderPass &renderPass;
  strb::vulkan::Pipeline &pipeline;
  strb::vector<strb::vulkan::Framebuffer> &framebuffers;
  strb::vulkan::CommandPool &commandPool;
  strb::vector<strb::vulkan::CommandBuffer> &commandBuffers;
  strb::vector<strb::vulkan::Shader> &shaders;

public:
  WSizeCallback(strb::vulkan::Device &device,
                strb::vulkan::Swapchain &swapchain,
                strb::vulkan::RenderPass &renderPass,
                strb::vulkan::Pipeline &pipeline,
                strb::vector<strb::vulkan::Framebuffer> &framebuffers,
                strb::vulkan::CommandPool &commandPool,
                strb::vector<strb::vulkan::CommandBuffer> &commandBuffers,
                strb::vector<strb::vulkan::Shader> &shaders)
      : strb::window::WindowSizeCallback(), device(device),
        swapchain(swapchain), renderPass(renderPass), pipeline(pipeline),
        framebuffers(framebuffers), commandPool(commandPool),
        commandBuffers(commandBuffers), shaders(shaders) {}
  void callback(const uint32_t width, const uint32_t height) override {
    this->device.waitIdle();
    this->swapchain.recreate(width, height);
    this->renderPass.recreate();
    this->pipeline.recreate(width, height);
    this->commandPool.recreate();
    uint32_t i = 0;
    for (strb::vulkan::Framebuffer &framebuffer : framebuffers) {
      framebuffer.recreate(width, height, {this->swapchain.getImageView(i)});
      i++;
    }
    strb::vector<strb::vulkan::CommandBuffer> tmp =
        commandPool.allocate(framebuffers.size());
    this->commandBuffers.clear();
    for (strb::vulkan::CommandBuffer buffer : tmp) {
      this->commandBuffers.push_back(buffer);
    }
    for (unsigned int i = 0; i < this->commandBuffers.size(); i++) {
      recordCommandBuffer(this->commandBuffers[i], renderPass, framebuffers[i],
                          pipeline, width, height);
    }
  }
};

int main() {
#ifdef NDEBUG
  strb::logln("RELEASE-MODE");
#else
  strb::logln("DEBUG-MODE");
#endif
  strb::glfw::Window window =
      strb::glfw::Window(600, 400, "FLOATING", strb::window::RENDER_API_VULKAN);

  strb::vulkan::Instance instance = strb::vulkan::Instance(
      "app-name", "strobe-game-engine", APP_VERSION, ENGINE_VERSION,
      VULKAN_API_VERSION, window.windowApi, {VK_KHR_SURFACE_EXTENSION_NAME},
      {"VK_LAYER_KHRONOS_validation"});

  strb::vulkan::PhysicalDevice physicalDevice =
      strb::vulkan::PhysicalDevice(instance);
  strb::vulkan::Device device = strb::vulkan::Device(
      physicalDevice, {VK_KHR_SWAPCHAIN_EXTENSION_NAME}, 1, 0, 0);

  strb::vulkan::Surface surface = strb::vulkan::Surface(instance, window);

  strb::vulkan::Swapchain swapchain =
      strb::vulkan::Swapchain(device, physicalDevice, surface, 600, 400, 3,
                              strb::vulkan::Format::RGBA_UNORM);
  strb::vulkan::Shader vertexShader =
      strb::vulkan::Shader(device, strb::io::read("shader/shader.vert.spv"));
  strb::vulkan::Shader fragmentShader =
      strb::vulkan::Shader(device, strb::io::read("shader/shader.frag.spv"));

  strb::vulkan::RenderPass renderPass =
      strb::vulkan::RenderPass(device, strb::vulkan::Format::RGBA_UNORM);

  strb::vulkan::Pipeline pipeline =
      strb::vulkan::Pipeline(device, renderPass, window.getWidth(),
                             window.getHeight(), vertexShader, fragmentShader);

  strb::vector<strb::vulkan::Framebuffer> swapchainFramebuffers(
      swapchain.getImageCount());
  for (uint64_t i = 0; i < swapchain.getImageCount(); i++) {
    strb::vector<strb::vulkan::ImageView> attachments = {
        swapchain.getImageView(i)};
    swapchainFramebuffers[i] = strb::vulkan::Framebuffer(
        device, renderPass, window.getWidth(), window.getHeight(), attachments);
  }

  strb::vulkan::CommandPool commandPool =
      strb::vulkan::CommandPool(device, strb::vulkan::QueueFamily::GRAPHICS);
  strb::vector<strb::vulkan::CommandBuffer> commandBuffers =
      commandPool.allocate(swapchainFramebuffers.size());

  for (unsigned int i = 0; i < commandBuffers.size(); i++) {
    recordCommandBuffer(commandBuffers[i], renderPass, swapchainFramebuffers[i],
                        pipeline, window.getWidth(), window.getHeight());
  }
  strb::vulkan::Semaphore semaphoreImageAvaiable =
      strb::vulkan::Semaphore(device);
  strb::vulkan::Semaphore semaphoreRenderingDone =
      strb::vulkan::Semaphore(device);

  strb::vector<strb::vulkan::Shader> shaders = {vertexShader, fragmentShader};
  WSizeCallback sizeCallback = WSizeCallback(
      device, swapchain, renderPass, pipeline, swapchainFramebuffers,
      commandPool, commandBuffers, shaders);
  window.addWindowSizeCallback(&sizeCallback);

  // LOOP

  while (!window.shouldClose()) {
    const uint32_t frameIndex =
        swapchain.nextFrameIndex(semaphoreImageAvaiable);
    strb::vulkan::Queue &queue = device.getGraphicsQueue(0);
    queue.submit(commandBuffers[frameIndex], {semaphoreImageAvaiable},
                 {semaphoreRenderingDone});

    queue.present(swapchain, frameIndex, {semaphoreRenderingDone});
    window.pollEvents();
  }

  // CLEANUP

  window.removeWindowSizeCallback(&sizeCallback);

  device.waitIdle();

  semaphoreImageAvaiable.destroy();
  semaphoreRenderingDone.destroy();

  commandPool.free(commandBuffers);
  commandPool.destroy();

  for (strb::vulkan::Framebuffer &framebuffer : swapchainFramebuffers) {
    framebuffer.destroy();
  }

  renderPass.destroy();

  pipeline.destroy();

  vertexShader.destroy();
  fragmentShader.destroy();

  swapchain.destroy();
  surface.destroy();
  device.destroy();
  instance.destroy();

  window.destroy();
}
