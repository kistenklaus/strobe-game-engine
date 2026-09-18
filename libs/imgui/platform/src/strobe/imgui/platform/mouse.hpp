#pragma once

#include "imgui.h"
#include "strobe/core/events/event_listener.hpp"
#include "strobe/core/events/event_listener_handle.hpp"
#include "strobe/imgui/platform/window_state.hpp"
#include "strobe/platform/window.hpp"
#include "strobe/platform/window_events.hpp"

#include <cfloat>
#include <mutex>

namespace strobe::imgui::platform {

class Mouse {
public:
  explicit Mouse(strobe::platform::Window *window, WindowState *state,
                 ImGuiIO *io, std::mutex *ioMutex) noexcept
      : m_io(io), m_ioMutex(ioMutex), m_state(state),
        m_positionListener(window->add_cursor_position_listener(
            EventListenerRef<strobe::platform::CursorPositionEvent>::
                fromMemberFunction<Mouse, &Mouse::on_position>(this))),
        m_buttonListener(window->add_mouse_button_listener(
            EventListenerRef<strobe::platform::MouseButtonEvent>::
                fromMemberFunction<Mouse, &Mouse::on_button>(this))),
        m_scrollListener(window->add_scroll_listener(
            EventListenerRef<strobe::platform::ScrollEvent>::fromMemberFunction<
                Mouse, &Mouse::on_scroll>(this))),
        m_cursorEnterListener(window->add_cursor_enter_listener(
            EventListenerRef<strobe::platform::CursorEnterEvent>::
                fromMemberFunction<Mouse, &Mouse::on_cursor_enter>(this))) {}

  ~Mouse() noexcept {
    m_positionListener.release();
    m_buttonListener.release();
    m_scrollListener.release();
    m_cursorEnterListener.release();
  }

private:
  void
  on_position(const strobe::platform::CursorPositionEvent &event) noexcept {
    float x = static_cast<float>(event.x());
    float y = static_cast<float>(event.y());
    std::lock_guard lock{*m_ioMutex};
    if (m_io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
      const ivec2 windowPos = m_state->pos();
      x += static_cast<float>(windowPos.x());
      y += static_cast<float>(windowPos.y());
    }
    m_io->AddMousePosEvent(x, y);
  }

  void on_button(const strobe::platform::MouseButtonEvent &event) noexcept {
    if (event.action() == strobe::platform::Action::repeat) {
      return;
    }

    const bool down = event.action() != strobe::platform::Action::release;
    const int button = static_cast<int>(event.button());

    if (button < 0 || button >= ImGuiMouseButton_COUNT) {
      return;
    }

    std::lock_guard lck{*m_ioMutex};
    m_io->AddMouseButtonEvent(button, down);
  }

  void on_scroll(const strobe::platform::ScrollEvent &event) noexcept {
    std::lock_guard lck{*m_ioMutex};
    m_io->AddMouseWheelEvent(static_cast<float>(event.xoffset()),
                             static_cast<float>(event.yoffset()));
  }

  void
  on_cursor_enter(const strobe::platform::CursorEnterEvent &event) noexcept {
    if (event.entered()) {
      return;
    }

    std::lock_guard lck{*m_ioMutex};
    m_io->AddMousePosEvent(-FLT_MAX, -FLT_MAX);
  }

private:
  ImGuiIO *m_io;
  std::mutex *m_ioMutex;

  WindowState *m_state;

  EventListenerHandle m_positionListener;
  EventListenerHandle m_buttonListener;
  EventListenerHandle m_scrollListener;
  EventListenerHandle m_cursorEnterListener;
};

} // namespace strobe::imgui::platform
