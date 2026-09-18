#pragma once

#include "strobe/core/events/event_listener.hpp"
#include "strobe/core/events/event_listener_handle.hpp"
#include "strobe/core/lina/vec.hpp"
#include "strobe/platform/window_events.hpp"

#include <GLFW/glfw3.h>
#include <cstdint>

namespace strobe::platform {

class Window {
public:
  Window(uvec2 size, const char *name) noexcept;
  Window(const Window &) = delete;
  Window(Window &&other) noexcept;
  Window &operator=(const Window &) = delete;
  Window &operator=(Window &&other) noexcept;
  ~Window() noexcept;

  bool should_close() const noexcept;
  void should_close(bool value) noexcept;
  void title(const char *value) noexcept;
  ivec2 position() const noexcept;
  void position(ivec2 value) noexcept;
  uvec2 size() const noexcept;
  void size(uvec2 value) noexcept;
  uvec2 framebuffer_size() const noexcept;
  vec2 content_scale() const noexcept;
  bool visible() const noexcept;
  void visible(bool value) noexcept;
  bool focused() const noexcept;
  bool minimized() const noexcept;
  bool maximized() const noexcept;
  bool hovered() const noexcept;
  void minimize() noexcept;
  void maximize() noexcept;
  void restore() noexcept;
  void focus() noexcept;
  void request_attention() noexcept;
  bool resizable() const noexcept;
  void resizable(bool value) noexcept;
  bool decorated() const noexcept;
  void decorated(bool value) noexcept;
  bool floating() const noexcept;
  void floating(bool value) noexcept;
  bool auto_iconify() const noexcept;
  void auto_iconify(bool value) noexcept;
  bool focus_on_show() const noexcept;
  void focus_on_show(bool value) noexcept;
  bool mouse_passthrough() const noexcept;
  void mouse_passthrough(bool value) noexcept;
  float opacity() const noexcept;
  void opacity(float value) noexcept;
  bool transparent_framebuffer() const noexcept;

  EventListenerHandle add_window_position_listener(
      const EventListenerRef<WindowPositionEvent> &listener,
      std::uint8_t layer = 0);

  EventListenerHandle
  add_window_size_listener(const EventListenerRef<WindowSizeEvent> &listener,
                           std::uint8_t layer = 0);

  EventListenerHandle
  add_window_close_listener(const EventListenerRef<WindowCloseEvent> &listener,
                            std::uint8_t layer = 0);

  EventListenerHandle add_window_refresh_listener(
      const EventListenerRef<WindowRefreshEvent> &listener,
      std::uint8_t layer = 0);

  EventListenerHandle
  add_window_focus_listener(const EventListenerRef<WindowFocusEvent> &listener,
                            std::uint8_t layer = 0);

  EventListenerHandle add_window_iconify_listener(
      const EventListenerRef<WindowIconifyEvent> &listener,
      std::uint8_t layer = 0);

  EventListenerHandle add_window_maximize_listener(
      const EventListenerRef<WindowMaximizeEvent> &listener,
      std::uint8_t layer = 0);

  EventListenerHandle add_framebuffer_size_listener(
      const EventListenerRef<FramebufferSizeEvent> &listener,
      std::uint8_t layer = 0);

  EventListenerHandle add_window_content_scale_listener(
      const EventListenerRef<WindowContentScaleEvent> &listener,
      std::uint8_t layer = 0);

  EventListenerHandle
  add_key_listener(const EventListenerRef<KeyEvent> &listener,
                   std::uint8_t layer = 0);

  EventListenerHandle
  add_character_listener(const EventListenerRef<CharacterEvent> &listener,
                         std::uint8_t layer = 0);

  EventListenerHandle add_character_mods_listener(
      const EventListenerRef<CharacterModsEvent> &listener,
      std::uint8_t layer = 0);

  EventListenerHandle add_cursor_position_listener(
      const EventListenerRef<CursorPositionEvent> &listener,
      std::uint8_t layer = 0);

  EventListenerHandle
  add_cursor_enter_listener(const EventListenerRef<CursorEnterEvent> &listener,
                            std::uint8_t layer = 0);

  EventListenerHandle
  add_mouse_button_listener(const EventListenerRef<MouseButtonEvent> &listener,
                            std::uint8_t layer = 0);

  EventListenerHandle
  add_scroll_listener(const EventListenerRef<ScrollEvent> &listener,
                      std::uint8_t layer = 0);

  EventListenerHandle
  add_drop_listener(const EventListenerRef<DropEvent> &listener,
                    std::uint8_t layer = 0);

  GLFWwindow *window_ptr() const noexcept;

private:
  void *m_internals = nullptr;
};

} // namespace strobe::platform
