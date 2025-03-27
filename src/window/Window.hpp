#pragma once
#include <memory>
#include <mutex>
#include <print>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>

#include "window/ClientApi.hpp"
#include "window/WindowApi.hpp"
#include "window/WindowContext.hpp"
#include "window/WindowControlBlock.hpp"

namespace strobe::window {

class Window {
 public:
  Window() : m_controlBlock(std::make_shared<details::WindowControlBlock>()) {
    m_controlBlock->title = "Strobe-Game-Engine";
  }

  void setWidth(unsigned int width) {
    std::lock_guard<std::mutex> lck{m_controlBlock->mutex};
    if (m_controlBlock->context.expired()) {
      m_controlBlock->width = width;
    } else {
      m_controlBlock->context.lock()->setWidth(width);
    }
  }
  unsigned int getWidth() const {
    std::lock_guard<std::mutex> lck{m_controlBlock->mutex};
    return m_controlBlock->width;
  }

  void setHeight(unsigned int height) {
    std::lock_guard<std::mutex> lck{m_controlBlock->mutex};
    if (m_controlBlock->context.expired()) {
      m_controlBlock->width = height;
    } else {
      m_controlBlock->context.lock()->setWidth(height);
    }
  }
  unsigned int getHeight() const {
    std::lock_guard<std::mutex> lck{m_controlBlock->mutex};
    return m_controlBlock->height;
  }

  void setTitle(std::string_view title) {
    std::lock_guard<std::mutex> lck{m_controlBlock->mutex};
    if (m_controlBlock->context.expired()) {
      m_controlBlock->title = title;
    } else {
      m_controlBlock->context.lock()->setTitle(title);
    }
  }

  std::string getTitle() const {
    std::lock_guard<std::mutex> lck{m_controlBlock->mutex};
    return m_controlBlock->title;
  }

  void setResizable(bool resizable) {
    std::lock_guard<std::mutex> lck{m_controlBlock->mutex};
    if (m_controlBlock->context.expired()) {
      m_controlBlock->resizable = resizable;
    } else {
      m_controlBlock->context.lock()->setResizable(resizable);
    }
  }

  bool isResizable() const {
    std::lock_guard<std::mutex> lck{m_controlBlock->mutex};
    return m_controlBlock->resizable;
  }

  bool shouldClose() const {
    std::lock_guard<std::mutex> lck{m_controlBlock->mutex};
    return m_controlBlock->context.expired();
  }

  std::shared_ptr<WindowContext> createContext(WindowApi windowApi);

  std::shared_ptr<WindowContext> getContext() {
    std::lock_guard<std::mutex> lck{m_controlBlock->mutex};
    return m_controlBlock->context.lock();
  }

  void waitUntilReady() {
    std::unique_lock<std::mutex> lock{m_controlBlock->mutex};
    m_controlBlock->initalized.wait(
        lock, [this]() { return !this->m_controlBlock->context.expired(); });
  }

  vk::SurfaceKHR createVkSurface(vk::Instance instance) {
    auto context = getContext();
    assert(context != nullptr);
    return context->createSurface(instance);
  }

  void setClientApi(ClientApi clientApi) {
    std::lock_guard<std::mutex> lck{m_controlBlock->mutex};
    assert(!static_cast<bool>(m_controlBlock->clientApi));
    m_controlBlock->clientApi = clientApi;
  }

 private:
  std::shared_ptr<details::WindowControlBlock> m_controlBlock;
};

}  // namespace strobe::window
