#include "strb/core/window/Window.hpp"

namespace strb::window {

Window::Window(const uint32_t width, const uint32_t height,
               const strb::string title, const ClientApi clientApi,
               const WindowApi windowApi, Mouse *mouse, Keyboard *keyboard)
    : clientApi(clientApi),
      windowApi(windowApi),
      title(title),
      mouse(mouse),
      keyboard(keyboard),
      width(width),
      height(height) {}

void Window::setWidthInternal(const uint32_t width) {
  this->width = width;
  for (WindowSizeCallback *sizeCallback : windowSizeCallbacks) {
    sizeCallback->callback(this->width, this->height);
  }
}
void Window::setHeightInternal(const uint32_t height) {
  this->height = height;
  for (WindowSizeCallback *sizeCallback : windowSizeCallbacks) {
    sizeCallback->callback(this->width, this->height);
  }
}
void Window::setSizeInternal(const uint32_t width, const uint32_t height,
                             const strb::boolean envokeCallacks) {
  this->width = width;
  this->height = height;
  if (envokeCallacks) {
    for (WindowSizeCallback *sizeCallback : windowSizeCallbacks) {
      sizeCallback->callback(this->width, this->height);
    }
  }
}

}  // namespace strb::window
