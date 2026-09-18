#pragma once

#include "imgui.h"
#include "strobe/core/events/event_listener.hpp"
#include "strobe/core/events/event_listener_handle.hpp"
#include "strobe/platform/window.hpp"
#include "strobe/platform/window_events.hpp"

#include <atomic>
#include <mutex>

namespace strobe::imgui::platform {

class WindowState {
public:
  explicit WindowState(strobe::platform::Window *window, ImGuiIO *io,
                       std::mutex *ioMutex) noexcept
      : m_io(io), m_ioMutex(ioMutex), m_size(window->size()),
        m_framebufferSize(window->framebuffer_size()),
        m_shouldClose(window->should_close()),
        m_focusListener(window->add_window_focus_listener(
            EventListenerRef<strobe::platform::WindowFocusEvent>::
                fromMemberFunction<WindowState, &WindowState::on_focus>(this))),
        m_sizeListener(window->add_window_size_listener(
            EventListenerRef<strobe::platform::WindowSizeEvent>::
                fromMemberFunction<WindowState, &WindowState::on_size>(this))),
        m_framebufferSizeListener(window->add_framebuffer_size_listener(
            EventListenerRef<strobe::platform::FramebufferSizeEvent>::
                fromMemberFunction<WindowState,
                                   &WindowState::on_framebuffer_size>(this))),
        m_closeListener(window->add_window_close_listener(
            EventListenerRef<strobe::platform::WindowCloseEvent>::
                fromMemberFunction<WindowState, &WindowState::on_close>(
                    this))) {}

  ~WindowState() noexcept {
    m_focusListener.release();
    m_sizeListener.release();
    m_framebufferSizeListener.release();
    m_closeListener.release();
  }

  [[nodiscard]] uvec2 size() const noexcept {
    return m_size.load(std::memory_order_relaxed);
  }

  [[nodiscard]] uvec2 framebuffer_size() const noexcept {
    return m_framebufferSize.load(std::memory_order_relaxed);
  }

  [[nodiscard]] vec2 framebuffer_scale() const noexcept {
    const auto size = m_size.load(std::memory_order_relaxed);
    const auto framebufferSize =
        m_framebufferSize.load(std::memory_order_relaxed);

    if (size.x() == 0 || size.y() == 0) {
      return vec2{1.0f, 1.0f};
    }

    return vec2{
        static_cast<float>(framebufferSize.x()) / static_cast<float>(size.x()),
        static_cast<float>(framebufferSize.y()) / static_cast<float>(size.y()),
    };
  }

  [[nodiscard]] bool should_close() const noexcept {
    return m_shouldClose.load(std::memory_order_relaxed);
  }

private:
  void on_focus(const strobe::platform::WindowFocusEvent &event) noexcept {
    std::lock_guard lock{*m_ioMutex};
    m_io->AddFocusEvent(event.focused());
  }

  void on_size(const strobe::platform::WindowSizeEvent &event) noexcept {
    m_size.store(uvec2{static_cast<uint32_t>(event.width()),
                       static_cast<uint32_t>(event.height())},
                 std::memory_order_relaxed);
  }

  void on_framebuffer_size(
      const strobe::platform::FramebufferSizeEvent &event) noexcept {
    m_framebufferSize.store(uvec2{static_cast<uint32_t>(event.width()),
                                  static_cast<uint32_t>(event.height())},
                            std::memory_order_relaxed);
  }

  void on_close(const strobe::platform::WindowCloseEvent &) noexcept {
    m_shouldClose.store(true, std::memory_order_relaxed);
  }

private:
  ImGuiIO *m_io;
  std::mutex *m_ioMutex;

  std::atomic<uvec2> m_size;
  std::atomic<uvec2> m_framebufferSize;
  std::atomic<bool> m_shouldClose;

  EventListenerHandle m_focusListener;
  EventListenerHandle m_sizeListener;
  EventListenerHandle m_framebufferSizeListener;
  EventListenerHandle m_closeListener;
};

} // namespace strobe::imgui::platform
