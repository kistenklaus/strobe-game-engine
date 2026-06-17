#include "strobe/window/Window.hpp"

#include "GlfwWindow.hpp"
#include "strobe/core/memory/AllocatorTraits.hpp"

namespace strobe::window {

Window::Window(void* internals) : m_internals(internals) {}

Window::~Window() {
  if (m_internals != nullptr) {
    auto win = static_cast<GlfwWindow*>(m_internals);
    GlfwWindow::kill(win);
    m_internals = nullptr;
  }
}

Window::Window(Window&& o)
    : m_internals(std::exchange(o.m_internals, nullptr)) {}
Window& Window::operator=(Window&& o) {
  if (this == &o) {
    return *this;
  }
  if (m_internals != nullptr) {
    auto win = static_cast<GlfwWindow*>(m_internals);
    auto allocator = win->m_context->getAllocator();
    using ATraits = AllocatorTraits<decltype(allocator)>;
    win->~GlfwWindow();
    ATraits::deallocate(allocator, win);
    m_internals = nullptr;
  }
  m_internals = std::exchange(o.m_internals, nullptr);
  return *this;
}

EventListenerHandle Window::addMouseButtonEventListener(
    EventListenerRef<MouseButtonEvent> listener) {
  auto win = static_cast<GlfwWindow*>(m_internals);
  return win->addMouseButtonEventListener(listener);
}

EventListenerHandle Window::addMouseMoveEventListener(
    EventListenerRef<MouseMoveEvent> listener) {
  auto win = static_cast<GlfwWindow*>(m_internals);
  return win->addMouseMoveEventListener(listener);
}

EventListenerHandle Window::addMouseScrollEventListener(
    EventListenerRef<MouseScrollEvent> listener) {
  auto win = static_cast<GlfwWindow*>(m_internals);
  return win->addMouseScrollEventListener(listener);
}

EventListenerHandle Window::addKeyboardEventListener(
    EventListenerRef<KeyboardEvent> listener) {
  auto win = static_cast<GlfwWindow*>(m_internals);
  return win->addKeyboardEventListener(listener);
}

EventListenerHandle Window::addCharEventListener(
    EventListenerRef<CharEvent> listener) {
  auto win = static_cast<GlfwWindow*>(m_internals);
  return win->addCharEventListener(listener);
}

EventListenerHandle Window::addFramebufferResizeEventListener(
    EventListenerRef<ResizeEvent> listener) {
  auto win = static_cast<GlfwWindow*>(m_internals);
  return win->addFramebufferResizeEventListener(listener);
}

EventListenerHandle Window::addResizeEventListener(
    EventListenerRef<ResizeEvent> listener) {
  auto win = static_cast<GlfwWindow*>(m_internals);
  return win->addResizeEventListener(listener);
}

bool Window::isResizable() const {
  auto win = static_cast<GlfwWindow*>(m_internals);
  return win->isResizable();
}

void Window::setResizable(bool resizable) {
  auto win = static_cast<GlfwWindow*>(m_internals);
  win->setResizable(resizable);
}

uvec2 Window::size() const {
  auto win = static_cast<GlfwWindow*>(m_internals);
  return win->size();
}

void Window::setSize(uvec2 size) {
  auto win = static_cast<GlfwWindow*>(m_internals);
  win->setSize(size);
}

uvec2 Window::framebufferSize() const {
  auto win = static_cast<GlfwWindow*>(m_internals);
  return win->framebufferSize();
}

void Window::close() {
  auto win = static_cast<GlfwWindow*>(m_internals);
  win->close();
}
bool Window::closed() const {
  auto win = static_cast<GlfwWindow*>(m_internals);
  return win->closed();
}

}  // namespace strobe::window
