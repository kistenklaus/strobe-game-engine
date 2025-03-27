#pragma once

#include <unistd.h>

#include <cassert>
#include <future>
#include <print>
#include <thread>

#include "renderer/vks/VksFrameQueue.hpp"
#include "renderer/vks/resources/ResourcePools.hpp"
#include "window/Window.hpp"

using namespace std::chrono_literals;

namespace strobe::renderer::vks {

class VksBackend {
 public:
  explicit VksBackend(VksFrameQueue* frameQueue, VksResourcePools* resourcePools, strobe::window::Window window)
      : m_frameQueue(frameQueue), m_resourcePools(resourcePools), m_window(window) {}

  void main(std::stop_token stoken, std::promise<void> stoped) {
    std::println("Create vkInstance, PhysicalDevice, Device ....");
    m_window.waitUntilReady();
    std::println("Create Surface & Swapchain");

    std::println("VksBackend: Started");
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
    stoped.set_value();
    m_frameQueue->stop();

    std::println("VksBackend: Stoped");
  }

  void start() {
    std::promise<void> stoped;
    m_stoped = stoped.get_future();
    m_thread = std::jthread(&VksBackend::main, this, std::move(stoped));
  }

  std::future<void> stop() {
    m_thread.request_stop();
    m_frameQueue->stop();
    return std::move(m_stoped);
  }

 private:
  VksFrameQueue* m_frameQueue;
  VksResourcePools* m_resourcePools;
  strobe::window::Window m_window;

  std::jthread m_thread;
  std::future<void> m_stoped;
};

}  // namespace strobe::renderer::vks
