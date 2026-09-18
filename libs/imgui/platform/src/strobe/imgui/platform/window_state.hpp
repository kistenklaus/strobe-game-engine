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
        m_shouldClose(window->should_close()), m_focused(window->focused()),
        m_pos(window->position()), m_minimized(window->minimized()),
        m_positionChanged(false), m_sizeChanged(false),
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
                fromMemberFunction<WindowState, &WindowState::on_close>(this))),
        m_posListener(window->add_window_position_listener(
            EventListenerRef<strobe::platform::WindowPositionEvent>::
                fromMemberFunction<WindowState, &WindowState::on_pos>(this))),
        m_iconifyListener(window->add_window_iconify_listener(
            EventListenerRef<strobe::platform::WindowIconifyEvent>::
                fromMemberFunction<WindowState, &WindowState::on_iconify>(
                    this))) {}

  ~WindowState() noexcept {
    m_focusListener.release();
    m_sizeListener.release();
    m_framebufferSizeListener.release();
    m_closeListener.release();
    m_posListener.release();
    m_iconifyListener.release();
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

  [[nodiscard]] bool focused() const noexcept {
    return m_focused.load(std::memory_order_relaxed);
  }

  [[nodiscard]] ivec2 pos() const noexcept {
    return m_pos.load(std::memory_order_relaxed);
  }

  [[nodiscard]] bool minimized() const noexcept {
    return m_minimized.load(std::memory_order_relaxed);
  }

  bool consume_pos_changed() noexcept {
    return m_positionChanged.exchange(false, std::memory_order_acquire);
  }

  bool consume_size_changed() noexcept {
    return m_sizeChanged.exchange(false, std::memory_order_acquire);
  }

private:
  void on_focus(const strobe::platform::WindowFocusEvent &event) noexcept {
    m_focused.store(event.focused(), std::memory_order_relaxed);
    std::lock_guard lock{*m_ioMutex};
    m_io->AddFocusEvent(event.focused());
  }

  void on_size(const strobe::platform::WindowSizeEvent &event) noexcept {
    m_size.store(uvec2{static_cast<uint32_t>(event.width()),
                       static_cast<uint32_t>(event.height())},
                 std::memory_order_relaxed);
    m_sizeChanged.store(true, std::memory_order_release);
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

  void on_pos(const strobe::platform::WindowPositionEvent &event) noexcept {
    m_pos.store(ivec2{event.x(), event.y()}, std::memory_order_relaxed);
    m_positionChanged.store(true, std::memory_order_release);
  }

  void on_iconify(const strobe::platform::WindowIconifyEvent &event) noexcept {
    m_minimized.store(event.iconified(), std::memory_order_relaxed);
  }

private:
  ImGuiIO *m_io;
  std::mutex *m_ioMutex;

  std::atomic<uvec2> m_size;
  std::atomic<uvec2> m_framebufferSize;
  std::atomic<bool> m_shouldClose;
  std::atomic<bool> m_focused;
  std::atomic<ivec2> m_pos;
  std::atomic<bool> m_minimized;

  std::atomic<bool> m_positionChanged;
  std::atomic<bool> m_sizeChanged;

  EventListenerHandle m_focusListener;
  EventListenerHandle m_sizeListener;
  EventListenerHandle m_framebufferSizeListener;
  EventListenerHandle m_closeListener;
  EventListenerHandle m_posListener;
  EventListenerHandle m_iconifyListener;
};

} // namespace strobe::imgui::platform
