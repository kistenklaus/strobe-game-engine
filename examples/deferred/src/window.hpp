#pragma once

#include "strobe/core/events/event_listener_handle.hpp"
#include "strobe/platform/window.hpp"
#include "strobe/platform/window_info.hpp"
#include <fmt/ostream.h>

class Window {
public:
  explicit Window(const strobe::platform::WindowInfo &info)
      : m_window(info),
        m_closeListenerHandle{m_window.add_window_close_listener(
            strobe::EventListenerRef<strobe::platform::WindowCloseEvent>::
                fromMemberFunction<Window, &Window::close_callback>(this))} {
    m_window.resizable(true);
  }
  Window(const Window &) = delete;
  Window(Window &&) = delete;
  Window &operator=(const Window &) = delete;
  Window &operator=(Window &&) = delete;

  bool should_close() const noexcept {
    return m_shouldClose.load(std::memory_order_acquire);
  }

  void show() noexcept { m_window.visible(true); }

  GLFWwindow *native() noexcept { return m_window.window_ptr(); }

private:
  void close_callback(const strobe::platform::WindowCloseEvent &) {
    m_shouldClose.store(true, std::memory_order_release);
  }

  strobe::platform::Window m_window;
  strobe::EventListenerHandle m_closeListenerHandle;
  std::atomic<bool> m_shouldClose;
};
