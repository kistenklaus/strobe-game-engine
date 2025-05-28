#include "./Window.hpp"

#include <print>
#include <stdexcept>
#include <utility>

#include "window/glfw/GlfwWindow.hpp"

namespace strobe::window {

std::shared_ptr<WindowContext> Window::createContext(WindowApi windowApi) {
  {
    std::lock_guard<std::mutex> lck{m_controlBlock->mutex};
    if (!m_controlBlock->context.expired()) {
      throw std::runtime_error(
          "Failed to create window context, there already exists a context");
    }
  }

  switch (windowApi) {
    case WindowApi::GLFW: {

      auto context = std::make_shared<GlfwWindow>(m_controlBlock->clientApi, m_controlBlock);

      {
        std::lock_guard<std::mutex> lck{m_controlBlock->mutex};
        m_controlBlock->context = context;
      }

      m_controlBlock->initalized.notify_all();
      return context;
    }
    default: {
      std::unreachable();
    }
  }
}

}  // namespace strobe::window
