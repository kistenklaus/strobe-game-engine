#include "./VksBackend.hpp"

#include <vulkan/vulkan_enums.hpp>

#include "renderer/vks/context/extensions/VksContextDebugUtilsExtension.hpp"
#include "renderer/vks/context/extensions/VksContextFavorDedicatedExtension.hpp"
#include "renderer/vks/context/extensions/VksContextRequireQueueFamiliesExtension.hpp"
#include "renderer/vks/context/extensions/VksContextWindowSupportExtension.hpp"
#include "renderer/vks/swapchain/VksSwapchain.hpp"

namespace strobe::renderer::vks {

static VkBool32 debugCallback(
    vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    vk::DebugUtilsMessageTypeFlagsEXT messageType,
    const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {
  switch (messageSeverity) {
    case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose:
      SPDLOG_TRACE("{}", pCallbackData->pMessage);
      break;
    case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:
      SPDLOG_INFO("{}", pCallbackData->pMessage);
      break;
    case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
      SPDLOG_WARN("{}", pCallbackData->pMessage);
      break;
    case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
      SPDLOG_ERROR("{}", pCallbackData->pMessage);
      break;
  }
  return VK_FALSE;
}

void VksBackend::createContext() {
  VksContextWindowSupportExtension windowSupport{m_window};
  VksContextRequireQueueFamiliesExtension queueFamilies({
      VksQueueSet{
          .queues =
              {
                  VksQueueRequest{
                      .flags = VksQueueFlags::Graphics |
                               VksQueueFlags::Present | VksQueueFlags::Transfer,
                      .avoid = VksQueueFlags::None,
                  },  //
                  VksQueueRequest{
                      .flags = VksQueueFlags::Transfer,
                      .avoid = VksQueueFlags::Graphics |
                               VksQueueFlags::Present | VksQueueFlags::Compute,
                  }  //
              },  //
      },          //
      VksQueueSet{
          .queues =
              {
                  VksQueueRequest{
                      .flags =
                          VksQueueFlags::Graphics | VksQueueFlags::Transfer,
                  },
                  VksQueueRequest{
                      .flags = VksQueueFlags::Present,
                  },
              },
      },
  });
  VksContextDebugUtilsExtension debugUtils{debugCallback, nullptr};
  VksContextFavorDedicatedExtension favorDedicated;

  VksContextExtension* extensions[] = {&windowSupport, &queueFamilies};
  VksContextExtension* optionalExtensions[] = {&debugUtils, &favorDedicated};

  auto attempt = VksContext::tryCreate(m_window, VK_API_VERSION_1_3, extensions,
                                       optionalExtensions);
  if (!attempt) {
    std::println("Failed to initalize VksContext");
    m_frameQueue->stop();
    m_thread.request_stop();
  }
  m_context = attempt.value();
}

void VksBackend::createSwapchain() {
  auto attempt = VksSwapchain::tryCreate(m_window, m_context);
  if (!attempt) {
    m_frameQueue->stop();
    m_thread.request_stop();
  }
  m_swapchain = attempt.value();
}

void VksBackend::main(std::stop_token stoken, std::promise<void> stoped) {
  m_window.waitUntilReady();

  createContext();
  createSwapchain();

  {
    while (!stoken.stop_requested()) {
      VksFrame* frame = m_frameQueue->acquireRenderFrame();
      if (frame == nullptr) {
        break;
      }
      for (auto drawCmd : frame->drawCommands) {
        m_resourcePools->text.unpin(drawCmd);
      }

      std::this_thread::sleep_for(100ms);
      m_frameQueue->releaseEmptyFrame();
    }
  }
  stoped.set_value();
  m_frameQueue->stop();

  m_swapchain.destroy(m_context);

  m_context.destroy();
}
void VksBackend::start() {
  std::promise<void> stoped;
  m_stoped = stoped.get_future();
  m_thread = std::jthread(&VksBackend::main, this, std::move(stoped));
}
std::future<void> VksBackend::stop() {
  m_thread.request_stop();
  m_frameQueue->stop();
  return std::move(m_stoped);
}
VksBackend::VksBackend(VksFrameQueue* frameQueue,
                       VksResourcePools* resourcePools,
                       strobe::window::Window window)
    : m_frameQueue(frameQueue),
      m_resourcePools(resourcePools),
      m_window(window) {}

}  // namespace strobe::renderer::vks
