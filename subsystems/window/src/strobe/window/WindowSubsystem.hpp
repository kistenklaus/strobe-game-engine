#pragma once

#include <GL/gl.h>
#include <GL/glext.h>

#include <print>
#include <string_view>
#include <strobe/lina.hpp>
#include <strobe/memory.hpp>

#include "strobe/core/events/event_listener.hpp"
#include "strobe/core/events/event_listener_handle.hpp"
#include "strobe/window/WindowImpl.hpp"

namespace strobe {

template <strobe::Allocator A>
class WindowSubsystem {
 public:
  WindowSubsystem(uvec2 size, std::string_view title, const A& allocator = {})
      : m_allocator(allocator),
        m_window(size, title, PolyAllocatorReference(&m_allocator)) {}

  ~WindowSubsystem() = default;

  // Ensure object identity
  WindowSubsystem(const WindowSubsystem&) = delete;
  WindowSubsystem& operator=(const WindowSubsystem&) = delete;
  WindowSubsystem(WindowSubsystem&&) = delete;
  WindowSubsystem& operator=(WindowSubsystem&&) = delete;

  void close() { m_window.close(); }

  bool closed() const { return m_window.closed(); }

  uvec2 getFramebufferSize() const { return m_window.getFramebufferSize(); }

  void setTitle(std::string_view title) { m_window.setTitle(title); }

  std::string getTitle() const { return m_window.getTitle(); }

  void setResizable(bool resizable) { return m_window.setResizable(resizable); }

  bool isResizable() const { return m_window.isResizable(); }

  EventListenerHandle addKeyboardListener(
      EventListenerRef<window::KeyboardEvent> listener) {
    return m_window.addKeyboardEventListener(listener);
  }

  EventListenerHandle addCharEventListener(
      EventListenerRef<window::CharEvent> listener) {
    return m_window.addCharEventListener(listener);
  }

  EventListenerHandle addResizeListener(
      EventListenerRef<window::ResizeEvent> listener) {
    return m_window.addResizeListener(listener);
  }

  EventListenerHandle addFramebufferSizeListener(
      EventListenerRef<window::ResizeEvent> listener) {
    return m_window.addFramebufferSizeListener(listener);
  }

  EventListenerHandle addMouseButtonEventListener(
      EventListenerRef<window::MouseButtonEvent> listener) {
    return m_window.addMouseButtonEventListener(listener);
  }

  EventListenerHandle addMouseMoveEventListener(
      EventListenerRef<window::MouseMoveEvent> listener) {
    return m_window.addMouseMoveEventListener(listener);
  }

  EventListenerHandle addMouseScrollEventListener(
      EventListenerRef<window::MouseScrollEvent> listener) {
    return m_window.addMouseScrollEventListener(listener);
  }

  EventListenerHandle addShutdownEventListener(
      EventListenerRef<window::ShutdownEvent> listener) {
    return m_window.addShutdownEventListener(listener);
  }

  // bool setKeyboardCallback() const { return m_window.

 private:
  [[no_unique_address]] MemoryResource<A> m_allocator;
  window::WindowImpl m_window;
};

}  // namespace strobe
