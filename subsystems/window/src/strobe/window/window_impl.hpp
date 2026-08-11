#pragma once

#include <GLFW/glfw3.h>
#include <atomic>
#include <fmt/printf.h>
#include <strobe/core/lina/vec.hpp>
#include <strobe/platform/platform.hpp>

namespace strobe::window {

namespace details {

struct SyncWindowState {
  std::atomic<uvec2> size = uvec2{0, 0};
  std::atomic<uvec2> fb_size = uvec2{0, 0};
  std::atomic<bool> should_close = false;
};

struct WindowStateSnapshot {
  uvec2 size = uvec2{0, 0};
  uvec2 fb_size = uvec2{0, 0};
  bool should_close = false;

  WindowStateSnapshot &operator=(const SyncWindowState &other) noexcept {

    size = other.size.load(std::memory_order_relaxed);
    fb_size = other.fb_size.load(std::memory_order_relaxed);
    should_close = other.should_close.load(std::memory_order_relaxed);

    return *this;
  }
};

} // namespace details

class WindowImpl {
private:
public:
  WindowImpl(uvec2 size, const char *name) {
    fmt::println("window-impl constructor");
    m_window = Platform::run([&, this]() -> GLFWwindow * {
      glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
      glfwWindowHint(GLFW_RESIZABLE, m_resizable ? GLFW_TRUE : GLFW_FALSE);
      glfwWindowHint(GLFW_VISIBLE, m_visible ? GLFW_TRUE : GLFW_FALSE);
      glfwWindowHint(GLFW_FLOATING, m_floating ? GLFW_TRUE : GLFW_TRUE);

      GLFWwindow *window =
          glfwCreateWindow(size.x(), size.y(), name, nullptr, nullptr);
      assert(window != nullptr);

      glfwHideWindow(window);
      glfwSetWindowUserPointer(window, this);

      {
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        this->m_state.size.store(uvec2(width, height),
                                 std::memory_order_relaxed);
      }
      {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        this->m_state.fb_size.store(uvec2(width, height),
                                    std::memory_order_relaxed);
      }

      glfwSetWindowCloseCallback(window, close_callback);
      glfwSetWindowSizeCallback(window, resize_callback);
      glfwSetFramebufferSizeCallback(window, framebuffer_resize_callback);

      return window;
    });

    poll();
  }
  ~WindowImpl() noexcept {
    fmt::println("window-impl destructor");
    Platform::run([window = m_window]() { glfwDestroyWindow(window); });
  }

  WindowImpl(const WindowImpl &) = delete;
  WindowImpl(WindowImpl &&) = delete;

  void show() noexcept {
    Platform::run([window = m_window]() { glfwShowWindow(window); });
    m_visible = true;
  }

  void hide() noexcept {
    Platform::run([window = m_window]() { glfwHideWindow(window); });
    m_visible = false;
  }

  bool visible() const noexcept { return m_visible; }
  bool visible(bool visible) noexcept {
    const bool current = m_visible;
    if (current == visible) {
      return current;
    }
    Platform::run([window = m_window, visible]() noexcept {
      if (visible) {
        glfwShowWindow(window);
      } else {
        glfwHideWindow(window);
      }
    });
    m_visible = visible;
    return current;
  }

  void poll() noexcept { m_snapshot = m_state; }

  bool should_close() const noexcept { return m_snapshot.should_close; }

  uvec2 size() const noexcept { return m_snapshot.size; }

  uvec2 framebuffer_size() const noexcept { return m_snapshot.fb_size; }

  bool resizable() const noexcept { return m_resizable; }

  bool resizable(bool resizable) noexcept {
    const bool current = m_resizable;
    if (current == resizable) {
      return current;
    }
    Platform::run([window = m_window, resizable]() noexcept {
      glfwSetWindowAttrib(window, GLFW_RESIZABLE,
                          resizable ? GLFW_TRUE : GLFW_FALSE);
    });
    m_resizable = resizable;
    return current;
  }

  bool floating() const noexcept { return m_floating; }
  bool floating(bool floating) noexcept {
    const bool current = m_floating;
    if (current == floating) {
      return current;
    }
    Platform::run([window = m_window, floating]() noexcept {
      glfwSetWindowAttrib(window, GLFW_FLOATING,
                          floating ? GLFW_TRUE : GLFW_FALSE);
    });
    m_floating = floating;
    return current;
  }

  GLFWwindow* ptr() noexcept {
    return m_window;
  }

private:
  static void close_callback(GLFWwindow *window) {
    fmt::println("close callback");
    auto win = static_cast<WindowImpl *>(glfwGetWindowUserPointer(window));
    win->m_state.should_close.store(true, std::memory_order_relaxed);
  }

  static void resize_callback(GLFWwindow *window, int w, int h) {
    auto win = static_cast<WindowImpl *>(glfwGetWindowUserPointer(window));
    assert(w >= 0);
    assert(h >= 0);
    win->m_state.size.store(
        uvec2(static_cast<unsigned int>(w), static_cast<unsigned int>(h)),
        std::memory_order_relaxed);
  }

  static void framebuffer_resize_callback(GLFWwindow *window, int w, int h) {
    auto win = static_cast<WindowImpl *>(glfwGetWindowUserPointer(window));
    assert(w >= 0);
    assert(h >= 0);
    win->m_state.fb_size.store(
        uvec2(static_cast<unsigned int>(w), static_cast<unsigned int>(h)),
        std::memory_order_relaxed);
  }


private:
  details::SyncWindowState m_state;
  details::WindowStateSnapshot m_snapshot;

  GLFWwindow *m_window;

  // more state
  uint8_t m_resizable : 1 = false;
  uint8_t m_visible : 1 = false;
  uint8_t m_floating : 1 = true;
};

} // namespace strobe::window
