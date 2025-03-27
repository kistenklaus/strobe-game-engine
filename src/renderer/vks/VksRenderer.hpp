#pragma once

#include <unistd.h>
#include <print>

#include "renderer/RenderContext.hpp"
#include "renderer/vks/VksBackend.hpp"
#include "renderer/vks/VksFrameQueue.hpp"
#include "renderer/vks/VksFrontend.hpp"
#include "renderer/vks/resources/ResourcePools.hpp"
#include "window/ClientApi.hpp"
#include "window/Window.hpp"
namespace strobe::renderer::vks {

class VksRenderer final : public RenderContext {
 public:
  VksRenderer(strobe::window::Window& window)
      : m_frameQueue(), m_frontend(&m_frameQueue, &m_resourcePools), m_backend(&m_frameQueue, &m_resourcePools, window) {
    window.setClientApi(window::ClientApiVulkan{});
  }

  void beginFrame() final override {
    m_frontend.beginFrame();
  }
  void endFrame() final override {
    m_frontend.endFrame();
  }

  ResourceHandle createText(std::string_view text) final override {
    return m_frontend.createText(text);
  }
  void drawText(ResourceHandle text) final override {
    m_frontend.drawText(text);
  }
  void destroyText(ResourceHandle text) final override {
    m_frontend.destroyText(text);
  }

  void start() final override {
    m_backend.start();
  }
  std::future<void> stop() final override {
    return m_backend.stop();
  }

 private:
  VksFrameQueue m_frameQueue;
  VksFrontend m_frontend;
  VksBackend m_backend;
  VksResourcePools m_resourcePools;


  std::jthread m_backendThread;
};

}  // namespace strobe::renderer::vks
