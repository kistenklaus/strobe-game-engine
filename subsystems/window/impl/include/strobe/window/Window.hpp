#pragma once

#include "strobe/core/events/event_listener_handle.hpp"
#include "strobe/window/CharEvent.hpp"
#include "strobe/window/KeyboardEvent.hpp"
#include "strobe/window/MouseButtonEvent.hpp"
#include "strobe/window/MouseMoveEvent.hpp"
#include "strobe/window/MouseScrollEvent.hpp"
#include "strobe/window/ResizeEvent.hpp"
namespace strobe::window {

class WindowContext;

class Window {
 public:
  friend class WindowContext;

  ~Window();

  Window(const Window&) = delete;
  Window& operator=(const Window&) = delete;
  Window(Window&&);
  Window& operator=(Window&&);

  EventListenerHandle addMouseButtonEventListener(EventListenerRef<MouseButtonEvent> listener);
  EventListenerHandle addMouseMoveEventListener(EventListenerRef<MouseMoveEvent> listener);
  EventListenerHandle addMouseScrollEventListener(EventListenerRef<MouseScrollEvent> listener);
  EventListenerHandle addKeyboardEventListener(EventListenerRef<KeyboardEvent> listener);
  EventListenerHandle addCharEventListener(EventListenerRef<CharEvent> listener);
  EventListenerHandle addFramebufferResizeEventListener(EventListenerRef<ResizeEvent> listener);
  EventListenerHandle addResizeEventListener(EventListenerRef<ResizeEvent> listener);

  bool isResizable() const;
  void setResizable(bool resizable);

  uvec2 size() const;
  void setSize(uvec2 size);

  uvec2 framebufferSize() const;

  void close();
  bool closed() const;

 private:
  Window(void* internals);
  void* m_internals;
};

}  // namespace strobe::window
