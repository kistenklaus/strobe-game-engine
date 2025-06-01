#pragma once

#include <GLFW/glfw3.h>

#include <atomic>
#include <mutex>
#include <stdexcept>

#include "GlfwWindowContext.hpp"
#include "strobe/core/containers/vector.hpp"
#include "strobe/core/events/event_dispatcher.hpp"
#include "strobe/core/events/event_listener_handle.hpp"
#include "strobe/core/memory/ReferenceCounter.hpp"
#include "strobe/window/CharEvent.hpp"
#include "strobe/window/KeyboardEvent.hpp"
#include "strobe/window/MouseButtonEvent.hpp"
#include "strobe/window/MouseMoveEvent.hpp"
#include "strobe/window/MouseScrollEvent.hpp"
#include "strobe/window/ResizeEvent.hpp"

namespace strobe::window {

class Window;

class GlfwWindow {
 public:
  friend class Window;
  GlfwWindow(GlfwWindowContext* context, uvec2 size, std::string_view title,
             bool resizable, bool closeOnCallback)
      : m_context(context),
        m_referenceCounter(),
        m_window(nullptr),
        m_closed(false),
        m_alive(true),
        m_closeOnCallback(closeOnCallback),
        m_attribs(Attributes{
            .mutex = {},
            .size = size,
            .framebufferSize = uvec2(0, 0),
            .ctitle = Vector<char, PolyAllocatorReference>(
                title.size() + 1, context->getAllocator()),
        }),
        m_mouseButtonEventDispatcher(context->getAllocator()),
        m_mouseMoveEventDispatcher(context->getAllocator()),
        m_mouseScrollEventDispatcher(context->getAllocator()),
        m_keyboardEventDispatcher(context->getAllocator()),
        m_charEventDispatcher(context->getAllocator()),
        m_framebufferSizeEventDispatcher(context->getAllocator()),
        m_resizeEventDispatcher(context->getAllocator()) {
    std::memcpy(m_attribs.ctitle.data(), title.data(), title.size());
    m_attribs.ctitle[title.size()] = '\0';

    context->exec([this, resizable]() {
      glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
      glfwWindowHint(GLFW_RESIZABLE, resizable ? GLFW_TRUE : GLFW_FALSE);
      glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
      glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);

      this->m_window =
          glfwCreateWindow(this->m_attribs.size.x(), this->m_attribs.size.y(),
                           this->m_attribs.ctitle.data(), nullptr, nullptr);
      if (this->m_window == nullptr) {
        throw std::runtime_error("Failed to create GLFW window");
      }
      glfwSetWindowUserPointer(this->m_window, this);

      glfwSetMouseButtonCallback(this->m_window, mouse_button_callback);
      glfwSetCursorPosCallback(this->m_window, mouse_move_callback);
      glfwSetScrollCallback(this->m_window, mouse_scroll_callback);
      glfwSetKeyCallback(this->m_window, keyboard_callback);
      glfwSetCharCallback(this->m_window, char_callback);
      glfwSetFramebufferSizeCallback(this->m_window,
                                     framebuffer_resize_callback);
      glfwSetWindowSizeCallback(this->m_window, resize_callback);
      glfwSetWindowCloseCallback(this->m_window, close_callback);

      glfwShowWindow(this->m_window);
      {
        int width, height;
        glfwGetWindowSize(this->m_window, &width, &height);
        this->m_attribs.size = uvec2(width, height);
      }
      {
        int fw, fh;
        glfwGetFramebufferSize(this->m_window, &fw, &fh);
        this->m_attribs.framebufferSize = uvec2(fw, fh);
      }
    });
  }

  GlfwWindow(const GlfwWindow&) = delete;
  GlfwWindow& operator=(const GlfwWindow&) = delete;

  GlfwWindow(GlfwWindow&&) = delete;
  GlfwWindow& operator=(GlfwWindow&&) = delete;

  ~GlfwWindow() = default;

  EventListenerHandle addMouseButtonEventListener(
      EventListenerRef<MouseButtonEvent> listener) {
    return m_context->exec([listener, this]() {
      auto handle = this->m_mouseButtonEventDispatcher.addListener(listener);
      handle.deferr(this, [](void* userData, events::EventListenerId id) {
        auto win = static_cast<GlfwWindow*>(userData);
        win->m_context->exec([win, id]() {
          win->m_mouseButtonEventDispatcher.removeListener(id);
          release(win);
        });
      });
      return handle;
    });
  }

  EventListenerHandle addMouseMoveEventListener(
      EventListenerRef<MouseMoveEvent> listener) {
    return m_context->exec([listener, this]() {
      auto handle = this->m_mouseMoveEventDispatcher.addListener(listener);
      handle.deferr(this, [](void* userData, events::EventListenerId id) {
        auto win = static_cast<GlfwWindow*>(userData);
        win->m_context->exec([win, id]() {
          win->m_mouseMoveEventDispatcher.removeListener(id);
        });
      });
      return handle;
    });
  }

  EventListenerHandle addMouseScrollEventListener(
      EventListenerRef<MouseScrollEvent> listener) {
    if (!m_referenceCounter.inc()) {
      return EventListenerHandle();
    }
    return m_context->exec([listener, this]() {
      auto handle = this->m_mouseScrollEventDispatcher.addListener(listener);
      handle.deferr(this, [](void* userData, events::EventListenerId id) {
        auto win = static_cast<GlfwWindow*>(userData);
        win->m_context->exec([win, id]() {
          win->m_mouseScrollEventDispatcher.removeListener(id);
          release(win);
        });
      });
      return handle;
    });
  }

  EventListenerHandle addKeyboardEventListener(
      EventListenerRef<KeyboardEvent> listener) {
    if (!m_referenceCounter.inc()) {
      return EventListenerHandle();
    }
    return m_context->exec([listener, this]() {
      auto handle = this->m_keyboardEventDispatcher.addListener(listener);
      handle.deferr(this, [](void* userData, events::EventListenerId id) {
        auto win = static_cast<GlfwWindow*>(userData);
        win->m_context->exec([win, id]() {
          win->m_keyboardEventDispatcher.removeListener(id);
          release(win);
        });
      });
      return handle;
    });
  }

  EventListenerHandle addCharEventListener(
      EventListenerRef<CharEvent> listener) {
    if (!m_referenceCounter.inc()) {
      return EventListenerHandle();
    }
    return m_context->exec([listener, this]() {
      auto handle = this->m_charEventDispatcher.addListener(listener);
      handle.deferr(this, [](void* userData, events::EventListenerId id) {
        auto win = static_cast<GlfwWindow*>(userData);
        win->m_context->exec([win, id]() {
          win->m_charEventDispatcher.removeListener(id);
          release(win);
        });
      });
      return handle;
    });
  }

  EventListenerHandle addFramebufferResizeEventListener(
      EventListenerRef<ResizeEvent> listener) {
    if (!m_referenceCounter.inc()) {
      return EventListenerHandle();
    }
    return m_context->exec([listener, this]() {
      auto handle =
          this->m_framebufferSizeEventDispatcher.addListener(listener);
      handle.deferr(this, [](void* userData, events::EventListenerId id) {
        auto win = static_cast<GlfwWindow*>(userData);
        win->m_context->exec([win, id]() {
          win->m_framebufferSizeEventDispatcher.removeListener(id);
          release(win);
        });
      });
      return handle;
    });
  }

  EventListenerHandle addResizeEventListener(
      EventListenerRef<ResizeEvent> listener) {
    if (!m_referenceCounter.inc()) {
      return EventListenerHandle();
    }
    return m_context->exec([listener, this]() {
      auto handle = this->m_resizeEventDispatcher.addListener(listener);
      handle.deferr(this, [](void* userData, events::EventListenerId id) {
        auto win = static_cast<GlfwWindow*>(userData);
        win->m_context->exec([win, id]() {
          win->m_resizeEventDispatcher.removeListener(id);
          release(win);
        });
      });
      return handle;
    });
  }

  bool isResizable() const {
    return m_context->exec([this]() {
      return glfwGetWindowAttrib(this->m_window, GLFW_RESIZABLE) == GLFW_TRUE;
    });
  }

  void setResizable(bool resizable) {
    m_context->exec([resizable, this]() {
      glfwSetWindowAttrib(this->m_window, GLFW_RESIZABLE,
                          resizable ? GLFW_TRUE : GLFW_FALSE);
    });
  }

  uvec2 size() const {
    std::lock_guard lck{m_attribs.mutex};
    return m_attribs.size;
  }

  void setSize(uvec2 size) {
    {
      std::lock_guard lck{m_attribs.mutex};
      if (m_attribs.size == size) {
        return;
      }
    }
    m_context->exec([this, size]() {
      std::lock_guard lck{m_attribs.mutex};
      glfwSetWindowSize(this->m_window, size.x(), size.y());
      this->m_attribs.size = size;
    });
  }

  uvec2 framebufferSize() const {
    std::lock_guard lck{m_attribs.mutex};
    return m_attribs.framebufferSize;
  }

  void close() {
    {
      if (m_closed.load(std::memory_order_relaxed) == true) {
        return;
      }
    }
    m_context->exec([this]() {
      std::lock_guard lck{m_attribs.mutex};
      m_closed.store(true, std::memory_order_release);
      glfwHideWindow(this->m_window);
    });
  }

  bool closed() const { return m_closed.load(std::memory_order_relaxed); }

  static void kill(GlfwWindow* window) {
    window->close();
    // NOTE: Can maybe be relaxed but don't care.
    window->m_alive.store(false, std::memory_order_seq_cst);

    window->m_context->exec(
        [win = window->m_window]() { glfwDestroyWindow(win); });

    release(window);
  }

 private:
  static void release(GlfwWindow* window) {
    if (window->m_referenceCounter.dec()) {
      assert(window->m_referenceCounter.isZero());
      assert(window->m_mouseButtonEventDispatcher.empty());
      assert(window->m_mouseMoveEventDispatcher.empty());
      assert(window->m_mouseScrollEventDispatcher.empty());
      assert(window->m_keyboardEventDispatcher.empty());
      assert(window->m_charEventDispatcher.empty());
      assert(window->m_framebufferSizeEventDispatcher.empty());
      assert(window->m_resizeEventDispatcher.empty());
      auto allocator = window->m_context->getAllocator();
      using ATraits = AllocatorTraits<decltype(allocator)>;
      window->~GlfwWindow();
      ATraits::deallocate(allocator, window);
    }
  }

  static void mouse_button_callback(GLFWwindow* window, int button, int action,
                                    int mods) {
    auto win = static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));
    win->m_mouseButtonEventDispatcher.dispatch(MouseButtonEvent::Payload{
        .button = MouseButton(button),
        .action = Action(action),
        .mod = Mod(mods),
    });
  }

  static void mouse_move_callback(GLFWwindow* window, double x, double y) {
    auto win = static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));
    win->m_mouseMoveEventDispatcher.dispatch(vec2(x, y));
  }

  static void mouse_scroll_callback(GLFWwindow* window, double dx, double dy) {
    auto win = static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));
    win->m_mouseScrollEventDispatcher.dispatch(dvec2(dx, dy));
  }

  static void keyboard_callback(GLFWwindow* window, int key, int scancode,
                                int action, int mods) {
    auto win = static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));
    win->m_keyboardEventDispatcher.dispatch(KeyboardEvent::Payload{
        .key = Key(key),
        .action = Action(action),
        .mod = Mod(mods),
    });
  }

  static void char_callback(GLFWwindow* window, unsigned int codepoint) {
    auto win = static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));
    win->m_charEventDispatcher.dispatch(char32_t(codepoint));
  }

  static void framebuffer_resize_callback(GLFWwindow* window, int w, int h) {
    auto win = static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));
    {
      std::lock_guard lck{win->m_attribs.mutex};
      win->m_attribs.framebufferSize = uvec2(w, h);
    }
    win->m_framebufferSizeEventDispatcher.dispatch(uvec2(w, h));
  }
  friend void framebuffer_resize_callback(GLFWwindow*, int, int);

  static void resize_callback(GLFWwindow* window, int w, int h) {
    auto win = static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));
    {
      std::lock_guard lck{win->m_attribs.mutex};
      win->m_attribs.size = uvec2(w, h);
    }
    win->m_resizeEventDispatcher.dispatch(uvec2(w, h));
  }
  friend void resize_callback(GLFWwindow*, int, int);

  static void close_callback(GLFWwindow* window) {
    auto win = static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));
    if (win->m_closeOnCallback) {
      win->close();
    }
  }
  friend void close_callback(GLFWwindow*);

  GlfwWindowContext* m_context;
  // NOTE: This includes the strobe::winodw::Window instance itself
  // and all listeners. This way we ensure that any listener which tries
  // to unregister itself after the window was destructed still hit's valid
  // memory.
  memory::ReferenceCounter<std::size_t> m_referenceCounter;
  GLFWwindow* m_window;
  std::atomic<bool> m_closed;
  std::atomic<bool> m_alive;
  bool m_closeOnCallback;
  struct Attributes {
    mutable std::mutex mutex;
    uvec2 size;
    uvec2 framebufferSize;
    Vector<char, PolyAllocatorReference> ctitle;
  } m_attribs;

  EventDispatcher<MouseButtonEvent, PolyAllocatorReference>
      m_mouseButtonEventDispatcher;
  EventDispatcher<MouseMoveEvent, PolyAllocatorReference>
      m_mouseMoveEventDispatcher;
  EventDispatcher<MouseScrollEvent, PolyAllocatorReference>
      m_mouseScrollEventDispatcher;

  EventDispatcher<KeyboardEvent, PolyAllocatorReference>
      m_keyboardEventDispatcher;
  EventDispatcher<CharEvent, PolyAllocatorReference> m_charEventDispatcher;

  EventDispatcher<ResizeEvent, PolyAllocatorReference>
      m_framebufferSizeEventDispatcher;
  EventDispatcher<ResizeEvent, PolyAllocatorReference> m_resizeEventDispatcher;
};

}  // namespace strobe::window
