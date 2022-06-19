#include "window/Window.hpp"

#include "window/glfw/GlfwWindowBackend.hpp"

namespace sge {

Window::Window(const u32 width, const u32 height, const std::string title,
               const RendererBackendAPI renderer_backend_api,
               const WindowBackendAPI window_backend_api)
    : m_backend([&]() {
        switch (window_backend_api) {
          case GLFW_WINDOW_BACKEND:
            return std::make_unique<glfw::GlfwWindowBackend>(
                width, height, title, renderer_backend_api);
        }
        throw std::runtime_error("window backend is not supported");
      }()),
      m_window_backend_api(window_backend_api) {}

Window::~Window() { dispose(); }

boolean Window::shouldClose() { return m_backend->shouldClose(); }

void Window::close() { m_backend->close(); }

void Window::dispose() {
  if (m_backend) {
    m_backend->destroy();
    m_backend = nullptr;
  }
}

void Window::swapBuffers() { m_backend->swapBuffers(); }

void Window::pollEvents() { m_backend->pollEvents(); }

WindowBackendAPI Window::getBackendAPI() const { return m_window_backend_api; }

const WindowBackend& Window::getBackend() const { return *m_backend; }

const u32 Window::getWidth() const { return m_backend->getWidth(); }

const u32 Window::getHeight() const { return m_backend->getHeight(); }

}  // namespace sge
