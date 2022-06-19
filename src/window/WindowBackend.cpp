#include "window/WindowBackend.hpp"

namespace sge {

WindowBackend::WindowBackend(const u32 width, const u32 height,
                             const std::string title,
                             const RendererBackendAPI render_backend_api,
                             std::unique_ptr<Mouse> mouse,
                             std::unique_ptr<Keyboard> keyboard)
    : m_renderer_backend_api(render_backend_api),
      m_title(title),
      m_mouse(std::move(mouse)),
      m_keyboard(std::move(keyboard)),
      m_width(width),
      m_height(height) {}

void WindowBackend::setWidthInternal(const u32 width) {
  m_width = width;
  for (WindowSizeCallback *sizeCallback : m_window_size_callbacks) {
    sizeCallback->callback(m_width, m_height);
  }
}
void WindowBackend::setHeightInternal(const u32 height) {
  m_height = height;
  for (WindowSizeCallback *sizeCallback : m_window_size_callbacks) {
    sizeCallback->callback(m_width, m_height);
  }
}
void WindowBackend::setSizeInternal(const u32 width, const u32 height,
                                    const boolean envokeCallacks) {
  m_width = width;
  m_height = height;
  if (envokeCallacks) {
    for (WindowSizeCallback *sizeCallback : m_window_size_callbacks) {
      sizeCallback->callback(m_width, m_height);
    }
  }
}

}  // namespace sge
