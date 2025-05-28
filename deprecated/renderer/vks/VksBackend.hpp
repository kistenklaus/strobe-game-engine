#pragma once

#include <unistd.h>

#include <cassert>
#include <expected>
#include <future>
#include <print>
#include <thread>

#include "renderer/vks/VksFrameQueue.hpp"
#include "renderer/vks/context/VksContext.hpp"
#include "renderer/vks/context/VksQueueInfo.hpp"
#include "renderer/vks/resources/ResourcePools.hpp"
#include "renderer/vks/swapchain/VksSwapchain.hpp"
#include "spdlog/spdlog.h"
#include "window/Window.hpp"

using namespace std::chrono_literals;

namespace strobe::renderer::vks {

class VksBackend {
 public:
  explicit VksBackend(VksFrameQueue* frameQueue,
                      VksResourcePools* resourcePools,
                      strobe::window::Window window);

  void createContext();

  void createSwapchain();

  void main(std::stop_token stoken, std::promise<void> stoped);

  void start();

  std::future<void> stop();

 private:
  VksFrameQueue* m_frameQueue;
  VksResourcePools* m_resourcePools;
  strobe::window::Window m_window;

  std::jthread m_thread;
  std::future<void> m_stoped;

  VksContext m_context;
  VksSwapchain m_swapchain;
};

}  // namespace strobe::renderer::vks
