#pragma once

#include "strobe/core/containers/string.hpp"
#include "strobe/core/events/event_dispatcher.hpp"
#include "strobe/core/lina/vec.hpp"
#include "strobe/platform/window_events.hpp"
#include "strobe/platform/window_events_utils.hpp"
#include <GLFW/glfw3.h>
#include <strobe/platform.hpp>

namespace strobe::platform {

struct WindowImpl {
public:
  explicit WindowImpl(uvec2 size, const char *title) {
    m_window = platform::run([&] {
      glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
      glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
      glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
      glfwWindowHint(GLFW_FLOATING, GLFW_FALSE);

      GLFWwindow *window =
          glfwCreateWindow(size.x(), size.y(), title, nullptr, nullptr);
      assert(window);

      glfwSetWindowUserPointer(window, this);

      glfwSetWindowPosCallback(window, glfw_window_position_callback);
      glfwSetWindowSizeCallback(window, glfw_window_size_callback);
      glfwSetWindowCloseCallback(window, glfw_window_close_callback);
      glfwSetWindowRefreshCallback(window, glfw_window_refresh_callback);
      glfwSetWindowFocusCallback(window, glfw_window_focus_callback);
      glfwSetWindowIconifyCallback(window, glfw_window_iconify_callback);
      glfwSetWindowMaximizeCallback(window, glfw_window_maximize_callback);
      glfwSetFramebufferSizeCallback(window, glfw_framebuffer_size_callback);
      glfwSetWindowContentScaleCallback(window,
                                        glfw_window_content_scale_callback);
      glfwSetKeyCallback(window, glfw_key_callback);
      glfwSetCharCallback(window, glfw_character_callback);
      glfwSetCharModsCallback(window, glfw_character_mods_callback);
      glfwSetCursorPosCallback(window, glfw_cursor_position_callback);
      glfwSetCursorEnterCallback(window, glfw_cursor_enter_callback);
      glfwSetMouseButtonCallback(window, glfw_mouse_button_callback);
      glfwSetScrollCallback(window, glfw_scroll_callback);
      glfwSetDropCallback(window, glfw_drop_callback);
      return window;
    });
  }
  WindowImpl(const WindowImpl &) = delete;
  WindowImpl(WindowImpl &&) = delete;
  WindowImpl &operator=(const WindowImpl &) = delete;
  WindowImpl &operator=(WindowImpl &&) = delete;

  ~WindowImpl() {
    platform::run([&] { glfwDestroyWindow(m_window); });
  }

  // Close flag
  bool should_close() const noexcept {
    GLFWwindow *window = m_window;

    return platform::run([window] noexcept {
      return glfwWindowShouldClose(window) != GLFW_FALSE;
    });
  }

  void should_close(bool value) noexcept {
    GLFWwindow *window = m_window;

    platform::run([window, value] noexcept {
      glfwSetWindowShouldClose(window, value ? GLFW_TRUE : GLFW_FALSE);
    });
  }

  // Metadata
  void title(const char *title) noexcept {
    GLFWwindow *window = m_window;

    platform::run(
        [window, title] noexcept { glfwSetWindowTitle(window, title); });
  }

  // Geometry
  ivec2 position() const noexcept {
    GLFWwindow *window = m_window;

    return platform::run([window] noexcept {
      int x;
      int y;

      glfwGetWindowPos(window, &x, &y);

      return ivec2{x, y};
    });
  }

  void position(ivec2 position) noexcept {
    GLFWwindow *window = m_window;

    platform::run([window, position] noexcept {
      glfwSetWindowPos(window, position.x(), position.y());
    });
  }

  uvec2 size() const noexcept {
    GLFWwindow *window = m_window;

    return platform::run([window] noexcept {
      int width;
      int height;

      glfwGetWindowSize(window, &width, &height);

      return uvec2{
          static_cast<uint32_t>(width),
          static_cast<uint32_t>(height),
      };
    });
  }

  void size(uvec2 size) noexcept {
    GLFWwindow *window = m_window;

    platform::run([window, size] noexcept {
      glfwSetWindowSize(window, static_cast<int>(size.x()),
                        static_cast<int>(size.y()));
    });
  }

  uvec2 framebuffer_size() const noexcept {
    GLFWwindow *window = m_window;

    return platform::run([window] noexcept {
      int width;
      int height;

      glfwGetFramebufferSize(window, &width, &height);

      return uvec2{
          static_cast<uint32_t>(width),
          static_cast<uint32_t>(height),
      };
    });
  }

  vec2 content_scale() const noexcept {
    GLFWwindow *window = m_window;

    return platform::run([window] noexcept {
      float x;
      float y;

      glfwGetWindowContentScale(window, &x, &y);

      return vec2{x, y};
    });
  }

  // State
  bool visible() const noexcept { return window_attribute(GLFW_VISIBLE); }

  void visible(bool value) noexcept {
    GLFWwindow *window = m_window;

    platform::run([window, value] noexcept {
      if (value) {
        glfwShowWindow(window);
      } else {
        glfwHideWindow(window);
      }
    });
  }

  bool focused() const noexcept { return window_attribute(GLFW_FOCUSED); }

  bool minimized() const noexcept { return window_attribute(GLFW_ICONIFIED); }

  bool maximized() const noexcept { return window_attribute(GLFW_MAXIMIZED); }

  bool hovered() const noexcept { return window_attribute(GLFW_HOVERED); }

  void minimize() noexcept {
    GLFWwindow *window = m_window;

    platform::run([window] noexcept { glfwIconifyWindow(window); });
  }

  void maximize() noexcept {
    GLFWwindow *window = m_window;

    platform::run([window] noexcept { glfwMaximizeWindow(window); });
  }

  void restore() noexcept {
    GLFWwindow *window = m_window;

    platform::run([window] noexcept { glfwRestoreWindow(window); });
  }

  void focus() noexcept {
    GLFWwindow *window = m_window;

    platform::run([window] noexcept { glfwFocusWindow(window); });
  }

  void request_attention() noexcept {
    GLFWwindow *window = m_window;

    platform::run([window] noexcept { glfwRequestWindowAttention(window); });
  }

  // Mutable attributes
  bool resizable() const noexcept { return window_attribute(GLFW_RESIZABLE); }

  void resizable(bool value) noexcept {
    set_window_attribute(GLFW_RESIZABLE, value);
  }

  bool decorated() const noexcept { return window_attribute(GLFW_DECORATED); }

  void decorated(bool value) noexcept {
    set_window_attribute(GLFW_DECORATED, value);
  }

  bool floating() const noexcept { return window_attribute(GLFW_FLOATING); }

  void floating(bool value) noexcept {
    set_window_attribute(GLFW_FLOATING, value);
  }

  bool auto_iconify() const noexcept {
    return window_attribute(GLFW_AUTO_ICONIFY);
  }

  void auto_iconify(bool value) noexcept {
    set_window_attribute(GLFW_AUTO_ICONIFY, value);
  }

  bool focus_on_show() const noexcept {
    return window_attribute(GLFW_FOCUS_ON_SHOW);
  }

  void focus_on_show(bool value) noexcept {
    set_window_attribute(GLFW_FOCUS_ON_SHOW, value);
  }

  bool mouse_passthrough() const noexcept {
    return window_attribute(GLFW_MOUSE_PASSTHROUGH);
  }

  void mouse_passthrough(bool value) noexcept {
    set_window_attribute(GLFW_MOUSE_PASSTHROUGH, value);
  }

  // Appearance
  float opacity() const noexcept {
    GLFWwindow *window = m_window;

    return platform::run(
        [window] noexcept { return glfwGetWindowOpacity(window); });
  }

  void opacity(float value) noexcept {
    GLFWwindow *window = m_window;

    platform::run(
        [window, value] noexcept { glfwSetWindowOpacity(window, value); });
  }

  bool transparent_framebuffer() const noexcept {
    return window_attribute(GLFW_TRANSPARENT_FRAMEBUFFER);
  }

  std::string clipboard() const noexcept {
    GLFWwindow *window = m_window;
    return platform::run(
        [window] -> std::string { return glfwGetClipboardString(window); });
  }

  void clipboard(std::string value) noexcept {
    GLFWwindow *window = m_window;
    platform::run(
        [window, &value] { glfwSetClipboardString(window, value.c_str()); });
  }

  EventListenerHandle add_window_position_listener(
      const EventListenerRef<WindowPositionEvent> &listener,
      std::uint8_t layer = 0) {
    return m_windowPosDispatcher.addListener(listener, layer);
  }

  EventListenerHandle
  add_window_size_listener(const EventListenerRef<WindowSizeEvent> &listener,
                           std::uint8_t layer = 0) {
    return m_windowSizeDispatcher.addListener(listener, layer);
  }

  EventListenerHandle
  add_window_close_listener(const EventListenerRef<WindowCloseEvent> &listener,
                            std::uint8_t layer = 0) {
    return m_windowCloseDispatcher.addListener(listener, layer);
  }

  EventListenerHandle add_window_refresh_listener(
      const EventListenerRef<WindowRefreshEvent> &listener,
      std::uint8_t layer = 0) {
    return m_windowRefreshDispatcher.addListener(listener, layer);
  }

  EventListenerHandle
  add_window_focus_listener(const EventListenerRef<WindowFocusEvent> &listener,
                            std::uint8_t layer = 0) {
    return m_windowFocusDispatcher.addListener(listener, layer);
  }

  EventListenerHandle add_window_iconify_listener(
      const EventListenerRef<WindowIconifyEvent> &listener,
      std::uint8_t layer = 0) {
    return m_windowIconifyDispatcher.addListener(listener, layer);
  }

  EventListenerHandle add_window_maximize_listener(
      const EventListenerRef<WindowMaximizeEvent> &listener,
      std::uint8_t layer = 0) {
    return m_windowMaximizeDispatcher.addListener(listener, layer);
  }

  EventListenerHandle add_framebuffer_size_listener(
      const EventListenerRef<FramebufferSizeEvent> &listener,
      std::uint8_t layer = 0) {
    return m_framebufferSizeDispatcher.addListener(listener, layer);
  }

  EventListenerHandle add_window_content_scale_listener(
      const EventListenerRef<WindowContentScaleEvent> &listener,
      std::uint8_t layer = 0) {
    return m_windowContentScaleDispatcher.addListener(listener, layer);
  }

  EventListenerHandle
  add_key_listener(const EventListenerRef<KeyEvent> &listener,
                   std::uint8_t layer = 0) {
    return m_keyDispatcher.addListener(listener, layer);
  }

  EventListenerHandle
  add_character_listener(const EventListenerRef<CharacterEvent> &listener,
                         std::uint8_t layer = 0) {
    return m_charDispatcher.addListener(listener, layer);
  }

  EventListenerHandle add_character_mods_listener(
      const EventListenerRef<CharacterModsEvent> &listener,
      std::uint8_t layer = 0) {
    return m_charModDispatcher.addListener(listener, layer);
  }

  EventListenerHandle add_cursor_position_listener(
      const EventListenerRef<CursorPositionEvent> &listener,
      std::uint8_t layer = 0) {
    return m_cursorPosDispatcher.addListener(listener, layer);
  }

  EventListenerHandle
  add_cursor_enter_listener(const EventListenerRef<CursorEnterEvent> &listener,
                            std::uint8_t layer = 0) {
    return m_cursorEnterDispatcher.addListener(listener, layer);
  }

  EventListenerHandle
  add_mouse_button_listener(const EventListenerRef<MouseButtonEvent> &listener,
                            std::uint8_t layer = 0) {
    return m_mouseButtonDispatcher.addListener(listener, layer);
  }

  EventListenerHandle
  add_scroll_listener(const EventListenerRef<ScrollEvent> &listener,
                      std::uint8_t layer = 0) {
    return m_scrollDispatcher.addListener(listener, layer);
  }

  EventListenerHandle
  add_drop_listener(const EventListenerRef<DropEvent> &listener,
                    std::uint8_t layer = 0) {
    return m_dropDispatcher.addListener(listener, layer);
  }

private:
  bool window_attribute(int attribute) const noexcept {
    GLFWwindow *window = m_window;

    return platform::run([window, attribute] noexcept {
      return glfwGetWindowAttrib(window, attribute) != GLFW_FALSE;
    });
  }

  void set_window_attribute(int attribute, bool value) noexcept {
    GLFWwindow *window = m_window;

    platform::run([window, attribute, value] noexcept {
      glfwSetWindowAttrib(window, attribute, value ? GLFW_TRUE : GLFW_FALSE);
    });
  }

  static void glfw_window_position_callback(GLFWwindow *window, int x,
                                            int y) noexcept {
    auto *self = static_cast<WindowImpl *>(glfwGetWindowUserPointer(window));

    self->m_windowPosDispatcher.dispatch({x, y});
  }

  static void glfw_window_size_callback(GLFWwindow *window, int width,
                                        int height) noexcept {
    auto *self = static_cast<WindowImpl *>(glfwGetWindowUserPointer(window));

    self->m_windowSizeDispatcher.dispatch({width, height});
  }

  static void glfw_window_close_callback(GLFWwindow *window) noexcept {
    auto *self = static_cast<WindowImpl *>(glfwGetWindowUserPointer(window));

    self->m_windowCloseDispatcher.dispatch({});
  }

  static void glfw_window_refresh_callback(GLFWwindow *window) noexcept {
    auto *self = static_cast<WindowImpl *>(glfwGetWindowUserPointer(window));

    self->m_windowRefreshDispatcher.dispatch({});
  }

  static void glfw_window_focus_callback(GLFWwindow *window,
                                         int focused) noexcept {
    auto *self = static_cast<WindowImpl *>(glfwGetWindowUserPointer(window));

    self->m_windowFocusDispatcher.dispatch(focused != GLFW_FALSE);
  }

  static void glfw_window_iconify_callback(GLFWwindow *window,
                                           int iconified) noexcept {
    auto *self = static_cast<WindowImpl *>(glfwGetWindowUserPointer(window));

    self->m_windowIconifyDispatcher.dispatch(iconified != GLFW_FALSE);
  }

  static void glfw_window_maximize_callback(GLFWwindow *window,
                                            int maximized) noexcept {
    auto *self = static_cast<WindowImpl *>(glfwGetWindowUserPointer(window));

    self->m_windowMaximizeDispatcher.dispatch(maximized != GLFW_FALSE);
  }

  static void glfw_framebuffer_size_callback(GLFWwindow *window, int width,
                                             int height) noexcept {
    auto *self = static_cast<WindowImpl *>(glfwGetWindowUserPointer(window));

    self->m_framebufferSizeDispatcher.dispatch({width, height});
  }

  static void glfw_window_content_scale_callback(GLFWwindow *window,
                                                 float xscale,
                                                 float yscale) noexcept {
    auto *self = static_cast<WindowImpl *>(glfwGetWindowUserPointer(window));

    self->m_windowContentScaleDispatcher.dispatch({xscale, yscale});
  }

  static void glfw_key_callback(GLFWwindow *window, int key, int scancode,
                                int action, int mods) noexcept {
    auto *self = static_cast<WindowImpl *>(glfwGetWindowUserPointer(window));

    self->m_keyDispatcher.dispatch({
        platform::detail::from_glfw_key(key),
        scancode,
        platform::detail::from_glfw_mods(mods),
        platform::detail::from_glfw_action(action),
    });
  }

  static void glfw_character_callback(GLFWwindow *window,
                                      unsigned int codepoint) noexcept {
    auto *self = static_cast<WindowImpl *>(glfwGetWindowUserPointer(window));

    self->m_charDispatcher.dispatch(static_cast<char32_t>(codepoint));
  }

  static void glfw_character_mods_callback(GLFWwindow *window,
                                           unsigned int codepoint,
                                           int mods) noexcept {
    auto *self = static_cast<WindowImpl *>(glfwGetWindowUserPointer(window));

    self->m_charModDispatcher.dispatch({
        static_cast<char32_t>(codepoint),
        platform::detail::from_glfw_mods(mods),
    });
  }

  static void glfw_cursor_position_callback(GLFWwindow *window, double xpos,
                                            double ypos) noexcept {
    auto *self = static_cast<WindowImpl *>(glfwGetWindowUserPointer(window));

    self->m_cursorPosDispatcher.dispatch({xpos, ypos});
  }

  static void glfw_cursor_enter_callback(GLFWwindow *window,
                                         int entered) noexcept {
    auto *self = static_cast<WindowImpl *>(glfwGetWindowUserPointer(window));

    self->m_cursorEnterDispatcher.dispatch(entered != GLFW_FALSE);
  }

  static void glfw_mouse_button_callback(GLFWwindow *window, int button,
                                         int action, int mods) noexcept {
    auto *self = static_cast<WindowImpl *>(glfwGetWindowUserPointer(window));

    self->m_mouseButtonDispatcher.dispatch({
        button,
        platform::detail::from_glfw_action(action),
        platform::detail::from_glfw_mods(mods),
    });
  }

  static void glfw_scroll_callback(GLFWwindow *window, double xoffset,
                                   double yoffset) noexcept {
    auto *self = static_cast<WindowImpl *>(glfwGetWindowUserPointer(window));

    self->m_scrollDispatcher.dispatch({xoffset, yoffset});
  }

  static void glfw_drop_callback(GLFWwindow *window, int count,
                                 const char **paths) noexcept {
    auto *self = static_cast<WindowImpl *>(glfwGetWindowUserPointer(window));

    self->m_dropDispatcher.dispatch({
        paths,
        static_cast<std::size_t>(count),
    });
  }

public:
  // TODO: make me private once GLFW is properly contained.
  GLFWwindow *m_window = nullptr;

private:
  EventDispatcher<WindowPositionEvent> m_windowPosDispatcher;
  EventDispatcher<WindowSizeEvent> m_windowSizeDispatcher;
  EventDispatcher<WindowCloseEvent> m_windowCloseDispatcher;
  EventDispatcher<WindowRefreshEvent> m_windowRefreshDispatcher;
  EventDispatcher<WindowFocusEvent> m_windowFocusDispatcher;
  EventDispatcher<WindowIconifyEvent> m_windowIconifyDispatcher;
  EventDispatcher<WindowMaximizeEvent> m_windowMaximizeDispatcher;
  EventDispatcher<FramebufferSizeEvent> m_framebufferSizeDispatcher;
  EventDispatcher<WindowContentScaleEvent> m_windowContentScaleDispatcher;
  EventDispatcher<KeyEvent> m_keyDispatcher;
  EventDispatcher<CharacterEvent> m_charDispatcher;
  EventDispatcher<CharacterModsEvent> m_charModDispatcher;
  EventDispatcher<CursorPositionEvent> m_cursorPosDispatcher;
  EventDispatcher<CursorEnterEvent> m_cursorEnterDispatcher;
  EventDispatcher<MouseButtonEvent> m_mouseButtonDispatcher;
  EventDispatcher<ScrollEvent> m_scrollDispatcher;
  EventDispatcher<DropEvent> m_dropDispatcher;
};

} // namespace strobe::platform
