#include <GLFW/glfw3.h>

#include <atomic>
#include <cstring>
#include <future>
#include <mutex>
#include <print>
#include <stdexcept>
#include <string_view>
#include <strobe/events.hpp>
#include <strobe/sync.hpp>
#include <strobe/window/WindowImpl.hpp>
#include <thread>

#include "strobe/core/events/event_dispatcher.hpp"
#include "strobe/core/events/event_listener.hpp"
#include "strobe/core/events/event_listener_handle.hpp"
#include "strobe/core/memory/AllocatorTraits.hpp"
#include "strobe/core/memory/PolyAllocator.hpp"
#include "strobe/core/sync/mpsc.hpp"

namespace strobe::window {

static thread_local std::mutex s_glfwUseCountMutex;
static thread_local std::size_t s_glfwUseCount;

struct WindowControlBlock;

class DeferredJob {
 public:
  virtual ~DeferredJob() = default;

  virtual void operator()(WindowControlBlock*) = 0;
};

struct WindowControlBlock {
  GLFWwindow* window;
  std::jthread thread;

  PolyAllocatorReference allocator;

  std::atomic<bool> closed;

  std::mutex attribMutex;
  uvec2 size;
  uvec2 framebufferSize;
  Vector<char, PolyAllocatorReference> ctitle;
  bool resizable;


  EventDispatcher<MouseButtonEvent, PolyAllocatorReference>
      mouseButtonEventDispatcher;
  EventDispatcher<MouseMoveEvent, PolyAllocatorReference>
      mouseMoveEventDispatcher;
  EventDispatcher<MouseScrollEvent, PolyAllocatorReference>
      mouseScrollEventDispatcher;

  EventDispatcher<KeyboardEvent, PolyAllocatorReference>
      keyboardEventDispatcher;
  EventDispatcher<CharEvent, PolyAllocatorReference> charEventDispatcher;

  EventDispatcher<ResizeEvent, PolyAllocatorReference>
      framebufferSizeEventDispatcher;
  EventDispatcher<ResizeEvent, PolyAllocatorReference> resizeEventDispatcher;

  EventDispatcher<ShutdownEvent, PolyAllocatorReference>
      shutdownEventDispatcher;

  // once globally.
  strobe::mpsc::Sender<DeferredJob*> deferredJobTx;

  void enqueueJob(DeferredJob* job) {
    if (std::this_thread::get_id() == thread.get_id()) {
      (*job)(this);
    } else {
      while (!deferredJobTx.send(job)) {
        glfwPostEmptyEvent();
        std::this_thread::yield();
      }
      glfwPostEmptyEvent();
    }
  }

  explicit WindowControlBlock(PolyAllocatorReference allocator,
                              strobe::mpsc::Sender<DeferredJob*> deferredJobTx,
                              uvec2 size,
                              Vector<char, PolyAllocatorReference> ctitle)
      : window(nullptr),
        thread({}),
        allocator(allocator),
        mouseButtonEventDispatcher(allocator),
        mouseMoveEventDispatcher(allocator),
        mouseScrollEventDispatcher(allocator),
        keyboardEventDispatcher(allocator),
        charEventDispatcher(allocator),
        framebufferSizeEventDispatcher(allocator),
        resizeEventDispatcher(allocator),
        shutdownEventDispatcher(allocator),
        deferredJobTx(std::move(deferredJobTx)),
        size(size),
        ctitle(std::move(ctitle)) {}
};

static void mouse_button_callback(GLFWwindow* window, int button, int action,
                                  int mods) {
  auto cb =
      reinterpret_cast<WindowControlBlock*>(glfwGetWindowUserPointer(window));

  cb->mouseButtonEventDispatcher.dispatch(MouseButtonEvent::Payload{
      .button = MouseButton(button),
      .action = Action(action),
      .mod = Mod(mods),
  });
}

class DeferredUnregisterMouseButtonEventListener : public DeferredJob {
 public:
  DeferredUnregisterMouseButtonEventListener(events::EventListenerId id)
      : m_id(id) {}

  void wait(WindowControlBlock* cb) {
    auto fut = std::move(m_comp.get_future());
    cb->enqueueJob(this);
    fut.wait();
  }

  void operator()(WindowControlBlock* cb) override {
    assert(cb != nullptr);
    cb->mouseButtonEventDispatcher.removeListener(m_id);
    if (cb->mouseButtonEventDispatcher.empty()) {
      assert(cb->window != nullptr);
      glfwSetMouseButtonCallback(cb->window, nullptr);
    }
    m_comp.set_value();
  }

 private:
  events::EventListenerId m_id;
  std::promise<void> m_comp;
};

static void deferred_mouse_button_listener_handle_unregister(
    void* userData, events::EventListenerId id) {
  auto cb = reinterpret_cast<WindowControlBlock*>(userData);
  DeferredUnregisterMouseButtonEventListener job{id};
  job.wait(cb);
}

class DeferredRegisterMouseButtonEventListener : public DeferredJob {
 public:
  DeferredRegisterMouseButtonEventListener(
      EventListenerRef<MouseButtonEvent> listener)
      : m_listener(std::move(listener)) {}

  EventListenerHandle get(WindowControlBlock* cb) {
    auto fut = std::move(m_handle.get_future());
    cb->enqueueJob(this);
    return fut.get();
  }

  void operator()(WindowControlBlock* cb) override {
    assert(cb != nullptr);
    assert(cb->window != nullptr);
    auto handle = cb->mouseButtonEventDispatcher.addListener(m_listener);
    glfwSetMouseButtonCallback(cb->window, mouse_button_callback);
    handle.deferr(cb, deferred_mouse_button_listener_handle_unregister);
    m_handle.set_value(std::move(handle));
  }

 private:
  EventListenerRef<MouseButtonEvent> m_listener;
  std::promise<EventListenerHandle> m_handle;
};

static void mouse_move_callback(GLFWwindow* window, double x, double y) {
  auto cb =
      reinterpret_cast<WindowControlBlock*>(glfwGetWindowUserPointer(window));
  cb->mouseMoveEventDispatcher.dispatch(vec2(x, y));
}

class DeferredUnregisterMouseMoveEventListener : public DeferredJob {
 public:
  DeferredUnregisterMouseMoveEventListener(events::EventListenerId id)
      : m_id(id) {}

  void wait(WindowControlBlock* cb) {
    auto fut = std::move(m_comp.get_future());
    cb->enqueueJob(this);
    fut.wait();
  }

  void operator()(WindowControlBlock* cb) override {
    assert(cb != nullptr);
    cb->mouseMoveEventDispatcher.removeListener(m_id);
    if (cb->mouseMoveEventDispatcher.empty()) {
      assert(cb->window != nullptr);
      glfwSetCursorPosCallback(cb->window, nullptr);
    }
    m_comp.set_value();
  }

 private:
  events::EventListenerId m_id;
  std::promise<void> m_comp;
};

static void deferred_mouse_move_handle_unregister(void* userData,
                                                  events::EventListenerId id) {
  auto cb = reinterpret_cast<WindowControlBlock*>(userData);
  DeferredUnregisterMouseMoveEventListener job{id};
  job.wait(cb);
}

static void mouse_scroll_callback(GLFWwindow* window, double dx, double dy) {
  auto cb =
      reinterpret_cast<WindowControlBlock*>(glfwGetWindowUserPointer(window));
  cb->mouseScrollEventDispatcher.dispatch(dvec2(dx, dy));
}

class DeferredUnregisterMouseScrollEventListener : public DeferredJob {
 public:
  DeferredUnregisterMouseScrollEventListener(events::EventListenerId id)
      : m_id(id) {}

  void wait(WindowControlBlock* cb) {
    auto fut = std::move(m_comp.get_future());
    cb->enqueueJob(this);
    fut.wait();
  }

  void operator()(WindowControlBlock* cb) override {
    assert(cb != nullptr);
    cb->mouseScrollEventDispatcher.removeListener(m_id);
    if (cb->mouseScrollEventDispatcher.empty()) {
      assert(cb->window != nullptr);
      glfwSetScrollCallback(cb->window, nullptr);
    }
    m_comp.set_value();
  }

 private:
  events::EventListenerId m_id;
  std::promise<void> m_comp;
};

static void deferred_mouse_scroll_handle_unregister(
    void* userData, events::EventListenerId id) {
  auto cb = reinterpret_cast<WindowControlBlock*>(userData);
  DeferredUnregisterMouseScrollEventListener job{id};
  job.wait(cb);
}

class DeferredRegisterMouseScrollEventListener : public DeferredJob {
 public:
  DeferredRegisterMouseScrollEventListener(
      EventListenerRef<MouseScrollEvent> listener)
      : m_listener(std::move(listener)) {}

  EventListenerHandle get(WindowControlBlock* cb) {
    auto fut = std::move(m_handle.get_future());
    cb->enqueueJob(this);
    return fut.get();
  }

  void operator()(WindowControlBlock* cb) override {
    assert(cb != nullptr);
    assert(cb->window != nullptr);

    auto handle = cb->mouseScrollEventDispatcher.addListener(m_listener);
    glfwSetScrollCallback(cb->window, mouse_scroll_callback);
    handle.deferr(cb, deferred_mouse_scroll_handle_unregister);
    m_handle.set_value(std::move(handle));
  }

 private:
  EventListenerRef<MouseScrollEvent> m_listener;
  std::promise<EventListenerHandle> m_handle;
};

static void char_callback(GLFWwindow* window, unsigned int codepoint) {
  auto cb =
      reinterpret_cast<WindowControlBlock*>(glfwGetWindowUserPointer(window));
  cb->charEventDispatcher.dispatch(char32_t(codepoint));
}

class DeferredUnregisterCharEventListener : public DeferredJob {
 public:
  DeferredUnregisterCharEventListener(events::EventListenerId id) : m_id(id) {}

  void wait(WindowControlBlock* cb) {
    auto fut = std::move(m_comp.get_future());
    cb->enqueueJob(this);
    fut.wait();
  }

  void operator()(WindowControlBlock* cb) override {
    assert(cb != nullptr);
    cb->charEventDispatcher.removeListener(m_id);
    if (cb->charEventDispatcher.empty()) {
      assert(cb->window != nullptr);
      glfwSetCharCallback(cb->window, nullptr);
    }
    m_comp.set_value();
  }

 private:
  events::EventListenerId m_id;
  std::promise<void> m_comp;
};

static void deferred_char_event_handle_unregister(void* userData,
                                                  events::EventListenerId id) {
  auto cb = reinterpret_cast<WindowControlBlock*>(userData);
  DeferredUnregisterCharEventListener job{id};
  job.wait(cb);
}

class DeferredRegisterCharEventListener : public DeferredJob {
 public:
  DeferredRegisterCharEventListener(EventListenerRef<CharEvent> listener)
      : m_listener(std::move(listener)) {}

  EventListenerHandle get(WindowControlBlock* cb) {
    auto fut = std::move(m_handle.get_future());
    cb->enqueueJob(this);
    return fut.get();
  }

  void operator()(WindowControlBlock* cb) override {
    assert(cb != nullptr);
    assert(cb->window != nullptr);

    auto handle = cb->charEventDispatcher.addListener(m_listener);
    glfwSetCharCallback(cb->window, char_callback);
    handle.deferr(cb, deferred_char_event_handle_unregister);
    m_handle.set_value(std::move(handle));
  }

 private:
  EventListenerRef<CharEvent> m_listener;
  std::promise<EventListenerHandle> m_handle;
};

static void keyboard_callback(GLFWwindow* window, int key, int scancode,
                              int action, int mods) {
  auto cb =
      reinterpret_cast<WindowControlBlock*>(glfwGetWindowUserPointer(window));

  cb->keyboardEventDispatcher.dispatch(KeyboardEvent::Payload{
      .key = Key(key),
      .action = Action(action),
      .mod = Mod(mods),
  });
}

class DeferredUnregisterKeyEventListener : public DeferredJob {
 public:
  DeferredUnregisterKeyEventListener(events::EventListenerId id) : m_id(id) {}

  void wait(WindowControlBlock* cb) {
    auto fut = std::move(m_comp.get_future());
    cb->enqueueJob(this);
    fut.wait();
  }

  void operator()(WindowControlBlock* cb) override {
    assert(cb != nullptr);
    cb->keyboardEventDispatcher.removeListener(m_id);
    if (cb->keyboardEventDispatcher.empty()) {
      assert(cb->window != nullptr);
      glfwSetKeyCallback(cb->window, nullptr);
    }
    m_comp.set_value();
  }

 private:
  events::EventListenerId m_id;
  std::promise<void> m_comp;
};

static void deferred_keyboard_handle_unregister(void* userData,
                                                events::EventListenerId id) {
  auto cb = reinterpret_cast<WindowControlBlock*>(userData);
  DeferredUnregisterKeyEventListener job{id};
  job.wait(cb);
}

class DeferredRegisterKeyEventListener : public DeferredJob {
 public:
  DeferredRegisterKeyEventListener(EventListenerRef<KeyboardEvent> listener)
      : m_listener(std::move(listener)) {}

  EventListenerHandle get(WindowControlBlock* cb) {
    auto fut = std::move(m_handle.get_future());
    cb->enqueueJob(this);
    return fut.get();
  }

  void operator()(WindowControlBlock* cb) override {
    assert(cb != nullptr);
    assert(cb->window != nullptr);
    auto handle = cb->keyboardEventDispatcher.addListener(m_listener);

    glfwSetKeyCallback(cb->window, keyboard_callback);
    handle.deferr(cb, deferred_keyboard_handle_unregister);

    m_handle.set_value(std::move(handle));
  }

 private:
  EventListenerRef<KeyboardEvent> m_listener;
  std::promise<EventListenerHandle> m_handle;
};

class DeferredRegisterMouseMoveEventListener : public DeferredJob {
 public:
  DeferredRegisterMouseMoveEventListener(
      EventListenerRef<MouseMoveEvent> listener)
      : m_listener(std::move(listener)) {}

  EventListenerHandle get(WindowControlBlock* cb) {
    auto fut = std::move(m_handle.get_future());
    cb->enqueueJob(this);
    return fut.get();
  }

  void operator()(WindowControlBlock* cb) override {
    assert(cb != nullptr);
    assert(cb->window != nullptr);
    auto handle = cb->mouseMoveEventDispatcher.addListener(m_listener);
    glfwSetCursorPosCallback(cb->window, mouse_move_callback);
    handle.deferr(cb, deferred_mouse_move_handle_unregister);
    m_handle.set_value(std::move(handle));
  }

 private:
  EventListenerRef<MouseMoveEvent> m_listener;
  std::promise<EventListenerHandle> m_handle;
};

void resize_callback(GLFWwindow* window, int width, int height) {
  auto cb =
      reinterpret_cast<WindowControlBlock*>(glfwGetWindowUserPointer(window));

  {
    std::lock_guard lck{cb->attribMutex};
    cb->size = uvec2(width, height);
  }

  cb->resizeEventDispatcher.dispatch(uvec2(width, height));
}

class DeferredUnregisterResizeEventListener : public DeferredJob {
 public:
  DeferredUnregisterResizeEventListener(events::EventListenerId id)
      : m_id(id) {}

  void wait(WindowControlBlock* cb) {
    auto fut = std::move(m_comp.get_future());
    cb->enqueueJob(this);
    fut.wait();
  }

  void operator()(WindowControlBlock* cb) override {
    cb->resizeEventDispatcher.removeListener(m_id);
    m_comp.set_value();
  }

 private:
  events::EventListenerId m_id;
  std::promise<void> m_comp;
};

static void deferred_resize_handle_unregister(void* userData,
                                              events::EventListenerId id) {
  auto cb = reinterpret_cast<WindowControlBlock*>(userData);
  DeferredUnregisterResizeEventListener job{id};
  job.wait(cb);
}

class DeferredRegisterResizeEventListener : public DeferredJob {
 public:
  DeferredRegisterResizeEventListener(EventListenerRef<ResizeEvent> listener)
      : m_listener(std::move(listener)) {}

  EventListenerHandle get(WindowControlBlock* cb) {
    auto fut = std::move(m_handle.get_future());
    cb->enqueueJob(this);
    return fut.get();
  }

  void operator()(WindowControlBlock* cb) {
    auto handle = cb->resizeEventDispatcher.addListener(m_listener);
    handle.deferr(cb, deferred_resize_handle_unregister);
    m_handle.set_value(std::move(handle));
  }

 private:
  EventListenerRef<ResizeEvent> m_listener;
  std::promise<EventListenerHandle> m_handle;
};

static void framebuffer_size_callback(GLFWwindow* window, int width,
                                      int height) {
  auto cb =
      reinterpret_cast<WindowControlBlock*>(glfwGetWindowUserPointer(window));
  cb->framebufferSizeEventDispatcher.dispatch(uvec2(width, height));
}

class DeferredUnregisterFramebufferSizeListener : public DeferredJob {
 public:
  DeferredUnregisterFramebufferSizeListener(events::EventListenerId id)
      : m_id(std::move(id)) {}

  void wait(WindowControlBlock* cb) {
    auto fut = std::move(m_comp.get_future());
    cb->enqueueJob(this);
    fut.wait();
  }

  void operator()(WindowControlBlock* cb) override {
    cb->framebufferSizeEventDispatcher.removeListener(m_id);
    m_comp.set_value();
  }

 private:
  events::EventListenerId m_id;
  std::promise<void> m_comp;
};

void deferred_framebuffer_size_handle_unregister(void* userData,
                                                 events::EventListenerId id) {
  auto cb = reinterpret_cast<WindowControlBlock*>(userData);
  DeferredUnregisterFramebufferSizeListener job{id};
  job.wait(cb);
}

class DeferredRegisterFramebufferSizeListener : public DeferredJob {
 public:
  DeferredRegisterFramebufferSizeListener(
      EventListenerRef<ResizeEvent> listener)
      : m_listener(std::move(listener)) {}

  EventListenerHandle get(WindowControlBlock* cb) {
    auto fut = std::move(m_handle.get_future());
    cb->enqueueJob(this);
    return fut.get();
  }

  void operator()(WindowControlBlock* cb) override {
    auto handle = cb->framebufferSizeEventDispatcher.addListener(m_listener);
    handle.deferr(cb, deferred_framebuffer_size_handle_unregister);
    m_handle.set_value(std::move(handle));
  }

 private:
  EventListenerRef<ResizeEvent> m_listener;
  std::promise<EventListenerHandle> m_handle;
};

class DeferredSetAttrib : public DeferredJob {
 public:
  DeferredSetAttrib(int attrib, int value) : m_attrib(attrib), m_value(value) {}

  void wait(WindowControlBlock* cb) {
    auto fut = std::move(m_comp.get_future());
    cb->enqueueJob(this);
    fut.wait();
  }

  void operator()(WindowControlBlock* cb) override {
    assert(cb != nullptr);
    assert(cb->window != nullptr);
    if (m_attrib == GLFW_RESIZABLE) {
      std::lock_guard lck{cb->attribMutex};
      cb->resizable = m_value == GLFW_TRUE;
    }
    if (m_attrib == GLFW_VISIBLE) { // <- a bit of a hack
      if (m_value == GLFW_TRUE) {
        glfwShowWindow(cb->window);
      } else {
        glfwHideWindow(cb->window);
      }
    } else {
      glfwSetWindowAttrib(cb->window, m_attrib, m_value);
    }

    m_comp.set_value();
  }

 private:
  int m_attrib;
  int m_value;
  std::promise<void> m_comp;
};

class DeferredSetTitle : public DeferredJob {
 public:
  DeferredSetTitle(std::string_view title) : m_newTitle(title) {}
  void wait(WindowControlBlock* cb) {
    auto fut = std::move(m_comp.get_future());
    cb->enqueueJob(this);
    fut.wait();
  }

  void operator()(WindowControlBlock* cb) override {
    assert(cb != nullptr);
    assert(cb->window != nullptr);
    std::lock_guard lck{cb->attribMutex};
    cb->ctitle.resize(m_newTitle.size() + 1);
    std::memcpy(cb->ctitle.data(), m_newTitle.data(),
                m_newTitle.size() * sizeof(char));
    cb->ctitle[m_newTitle.size()] = '\0';
    glfwSetWindowTitle(cb->window, cb->ctitle.data());
    m_comp.set_value();
  }

 private:
  std::string_view m_newTitle;
  std::promise<void> m_comp;
};

class DeferredSetSize : public DeferredJob {
 public:
  DeferredSetSize(uvec2 newSize) : m_newSize(newSize) {}

  void wait(WindowControlBlock* cb) {
    auto fut = std::move(m_comp.get_future());
    cb->enqueueJob(this);
    fut.wait();
  }

  void operator()(WindowControlBlock* cb) override {
    assert(cb != nullptr);
    assert(cb->window != nullptr);
    std::lock_guard lck{cb->attribMutex};
    cb->size = m_newSize;
    glfwSetWindowSize(cb->window, cb->size.x(), cb->size.y());
  }

 private:
  uvec2 m_newSize;
  std::promise<void> m_comp;
};

enum class WindowCreateErno {
  OK,
  FAILED_TO_INITALIZE_GLFW,
  FAILED_TO_CREATE_WINDOW,
};

static void window_close_callback(GLFWwindow* window) {
  auto cb =
      reinterpret_cast<WindowControlBlock*>(glfwGetWindowUserPointer(window));
  assert(cb != nullptr);
  bool closed = false;
  if (cb->closed.compare_exchange_strong(closed, true)) {
    assert(cb->window != nullptr);
    std::println("CLOSED-BY-CALLBACK");
    glfwHideWindow(cb->window);
    cb->shutdownEventDispatcher.dispatch(ShutdownEvent::State::CLOSED);
  }
}

class DeferredUnregisterShutdownEventListener : public DeferredJob {
 public:
  DeferredUnregisterShutdownEventListener(events::EventListenerId id)
      : m_id(id) {}

  void wait(WindowControlBlock* cb) {
    auto fut = std::move(m_comp.get_future());
    cb->enqueueJob(this);
    fut.wait();
  }

  void operator()(WindowControlBlock* cb) {
    cb->shutdownEventDispatcher.removeListener(m_id);
    m_comp.set_value();
  }

 private:
  events::EventListenerId m_id;
  std::promise<void> m_comp;
};

static void deferred_shutdown_event_handle_unregister(
    void* userData, events::EventListenerId id) {
  auto cb = reinterpret_cast<WindowControlBlock*>(userData);
  DeferredUnregisterShutdownEventListener job{id};
  job.wait(cb);
}

class DeferredRegisterShutdownEventListener : public DeferredJob {
 public:
  DeferredRegisterShutdownEventListener(
      EventListenerRef<ShutdownEvent> listener)
      : m_listener(std::move(listener)) {}

  EventListenerHandle get(WindowControlBlock* cb) {
    auto fut = std::move(m_handle.get_future());
    cb->enqueueJob(this);
    return fut.get();
  }

  void operator()(WindowControlBlock* cb) override {
    auto handle = cb->shutdownEventDispatcher.addListener(m_listener);
    handle.deferr(cb, deferred_shutdown_event_handle_unregister);
    m_handle.set_value(std::move(handle));
  }

 private:
  EventListenerRef<ShutdownEvent> m_listener;
  std::promise<EventListenerHandle> m_handle;
};

static void windowThread(std::stop_token stop_token, WindowControlBlock* self,
                         mpsc::Receiver<DeferredJob*> deferredRx,
                         std::promise<WindowCreateErno> asyncInitReturn) {
  using ATraits = AllocatorTraits<PolyAllocatorReference>;
  {
    std::lock_guard<std::mutex> lck{s_glfwUseCountMutex};
    if (s_glfwUseCount == 0) {
      if (!glfwInit()) {
        asyncInitReturn.set_value_at_thread_exit(
            WindowCreateErno::FAILED_TO_INITALIZE_GLFW);
        return;
      }
    }
    s_glfwUseCount++;
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
  glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);

  self->window = glfwCreateWindow(self->size.x(), self->size.y(),
                                  self->ctitle.data(), nullptr, nullptr);
  self->closed = false;

  if (self->window == nullptr) {
    glfwTerminate();
    asyncInitReturn.set_value_at_thread_exit(
        WindowCreateErno::FAILED_TO_CREATE_WINDOW);
    return;
  }
  glfwSwapInterval(0);
  glfwSetWindowUserPointer(self->window, self);

  glfwShowWindow(self->window);

  self->resizable =
      glfwGetWindowAttrib(self->window, GLFW_RESIZABLE) == GLFW_TRUE;
  {
    int width, height;
    glfwGetWindowSize(self->window, &width, &height);
    self->size = uvec2(width, height);
  }
  {
    int fw, fh;
    glfwGetFramebufferSize(self->window, &fw, &fh);
    self->framebufferSize = uvec2(fw, fh);
  }

  glfwSetWindowCloseCallback(self->window, window_close_callback);
  glfwSetWindowSizeCallback(self->window, resize_callback);
  glfwSetFramebufferSizeCallback(self->window, framebuffer_size_callback);

  asyncInitReturn.set_value(WindowCreateErno::OK);

  while (!stop_token.stop_requested()) {
    glfwWaitEvents();
    std::optional<DeferredJob*> job;
    while ((job = deferredRx.recv()).has_value()) {
      (**job)(self);
    }
  }

  bool closed = false;
  if (self->closed.compare_exchange_weak(closed, true)) {
    std::println("CLOSED-BY-LOOP");
    assert(self->window != nullptr);
    glfwHideWindow(self->window);
    self->shutdownEventDispatcher.dispatch(ShutdownEvent::State::CLOSED);
  }
  // clear rx queue, jobs do not have to be executed,
  // because we are shutting down anyway, just drop
  // pending jobs.
  while (deferredRx.recv().has_value()) {
  }

  // Destroy glfw.
  glfwDestroyWindow(std::exchange(self->window, nullptr));
  std::lock_guard<std::mutex> lck{s_glfwUseCountMutex};
  s_glfwUseCount--;
  if (s_glfwUseCount == 0) {
    glfwTerminate();
  }

  self->shutdownEventDispatcher.dispatch(ShutdownEvent::State::EXITED);
}

WindowImpl::WindowImpl(uvec2 size, std::string_view title,
                       PolyAllocatorReference allocator) {
  using ATraits = AllocatorTraits<PolyAllocatorReference>;
  WindowControlBlock* self = ATraits::allocate<WindowControlBlock>(allocator);
  m_internals = self;

  Vector<char, PolyAllocatorReference> ctitle(allocator);
  ctitle.resize(title.size() + 1);
  std::memcpy(ctitle.data(), title.data(), title.size());
  ctitle[title.size()] = '\0';

  // mpsc::channel<typename T, std::size_t Capacity>()
  auto [tx, rx] = mpsc::channel<DeferredJob*, 10>(allocator);
  new (self)
      WindowControlBlock(allocator, std::move(tx), size, std::move(ctitle));

  std::promise<WindowCreateErno> asyncReturn;
  auto asyncErno = std::move(asyncReturn.get_future());
  self->thread =
      std::jthread(windowThread, self, std::move(rx), std::move(asyncReturn));

  WindowCreateErno erno = asyncErno.get();
  if (erno != WindowCreateErno::OK) {
    self->~WindowControlBlock();  // implicity joins the thread.
    ATraits::deallocate<WindowControlBlock>(allocator, self);
    m_internals = nullptr;
    if (erno == WindowCreateErno::FAILED_TO_INITALIZE_GLFW) {
      throw std::runtime_error("Failed to initalize glfw");
    } else if (erno == WindowCreateErno::FAILED_TO_CREATE_WINDOW) {
      throw std::runtime_error("Failed to create glfw window");
    } else {
      throw std::runtime_error(
          "Failed to initalize the WindowImpl, with a unknown error.");
    }
  }
}

WindowImpl::~WindowImpl() {
  if (m_internals != nullptr) {
    close();

    auto self = reinterpret_cast<WindowControlBlock*>(m_internals);
    std::jthread thread = std::move(self->thread);
    thread.request_stop();
    glfwPostEmptyEvent();
    thread.join();

    // NOTE: Please just remove all listeners before destructing the window
    // subsystem. This is the only proper way to ensure that we dont have any
    // dangling pointers after shutdown.
    assert(self->mouseButtonEventDispatcher.empty());
    assert(self->mouseMoveEventDispatcher.empty());
    assert(self->mouseScrollEventDispatcher.empty());
    assert(self->keyboardEventDispatcher.empty());
    assert(self->charEventDispatcher.empty());
    assert(self->framebufferSizeEventDispatcher.empty());
    assert(self->resizeEventDispatcher.empty());
    assert(self->shutdownEventDispatcher.empty());
    self->~WindowControlBlock();

    using ATraits = AllocatorTraits<PolyAllocatorReference>;
    PolyAllocatorReference alloc = self->allocator;
    ATraits::deallocate<WindowControlBlock>(alloc, self);
    m_internals = nullptr;
  }
}

void WindowImpl::close() {
  auto self = reinterpret_cast<WindowControlBlock*>(m_internals);
  bool closed = false;
  if (self->closed.compare_exchange_strong(closed, true)) {
    std::println("CLOSED-BY-USER");
    DeferredSetAttrib job{GLFW_VISIBLE, GLFW_FALSE};
    job.wait(self);
    self->shutdownEventDispatcher.dispatch(ShutdownEvent::State::CLOSED);
  }
}

bool WindowImpl::closed() const {
  auto self = reinterpret_cast<WindowControlBlock*>(m_internals);
  return self->closed;
}

void WindowImpl::setSize(unsigned int width, unsigned int height) {
  auto self = reinterpret_cast<WindowControlBlock*>(m_internals);
  if (self->closed) {
    return;
  }
  DeferredSetSize job{uvec2(width, height)};
  job.wait(self);
}

uvec2 WindowImpl::getSize() const {
  auto self = reinterpret_cast<WindowControlBlock*>(m_internals);
  std::lock_guard lck{self->attribMutex};
  return self->size;
}

uvec2 WindowImpl::getFramebufferSize() const {
  auto self = reinterpret_cast<WindowControlBlock*>(m_internals);
  std::lock_guard lck{self->attribMutex};
  return self->framebufferSize;
}

void WindowImpl::setTitle(std::string_view title) {
  auto self = reinterpret_cast<WindowControlBlock*>(m_internals);
  if (self->closed) {
    return;
  }
  DeferredSetTitle job{title};
  job.wait(self);
}

std::string WindowImpl::getTitle() const {
  auto self = reinterpret_cast<WindowControlBlock*>(m_internals);
  std::lock_guard lck{self->attribMutex};
  return std::string(self->ctitle.data());
}

void WindowImpl::setResizable(bool resizable) {
  auto self = reinterpret_cast<WindowControlBlock*>(m_internals);
  if (self->closed) {
    return;
  }
  DeferredSetAttrib job{GLFW_RESIZABLE, resizable ? GLFW_TRUE : GLFW_FALSE};
  job.wait(self);
}

bool WindowImpl::isResizable() const {
  auto self = reinterpret_cast<WindowControlBlock*>(m_internals);
  std::lock_guard lck{self->attribMutex};
  return self->resizable;
}

EventListenerHandle WindowImpl::addKeyboardEventListener(
    EventListenerRef<KeyboardEvent> listener) {
  auto self = reinterpret_cast<WindowControlBlock*>(m_internals);
  if (self->closed) {
    // essentially does nothing.
    return self->keyboardEventDispatcher.addListener(listener);
  }
  DeferredRegisterKeyEventListener job{std::move(listener)};
  auto handle =
      std::move(job.get(self));  // <- waits for the deferred job to complete.

  return std::move(handle);
}

EventListenerHandle WindowImpl::addCharEventListener(
    EventListenerRef<CharEvent> listener) {
  auto self = reinterpret_cast<WindowControlBlock*>(m_internals);
  if (self->closed) {
    return self->charEventDispatcher.addListener(listener);
  }
  DeferredRegisterCharEventListener job{listener};
  return job.get(self);
}

EventListenerHandle WindowImpl::addResizeListener(
    EventListenerRef<ResizeEvent> listener) {
  auto self = reinterpret_cast<WindowControlBlock*>(m_internals);
  if (self->closed) {
    return self->resizeEventDispatcher.addListener(listener);
  }
  DeferredRegisterResizeEventListener job{listener};
  return job.get(self);
}

EventListenerHandle WindowImpl::addFramebufferSizeListener(
    EventListenerRef<ResizeEvent> listener) {
  auto self = reinterpret_cast<WindowControlBlock*>(m_internals);
  if (self->closed) {
    return self->framebufferSizeEventDispatcher.addListener(listener);
  }
  DeferredRegisterFramebufferSizeListener job{listener};
  return job.get(self);
}

EventListenerHandle WindowImpl::addMouseButtonEventListener(
    EventListenerRef<MouseButtonEvent> listener) {
  auto self = reinterpret_cast<WindowControlBlock*>(m_internals);
  if (self->closed) {
    return self->mouseButtonEventDispatcher.addListener(listener);
  }
  DeferredRegisterMouseButtonEventListener job{listener};
  return job.get(self);
}

EventListenerHandle WindowImpl::addMouseMoveEventListener(
    EventListenerRef<MouseMoveEvent> listener) {
  auto self = reinterpret_cast<WindowControlBlock*>(m_internals);
  if (self->closed) {
    return self->mouseMoveEventDispatcher.addListener(listener);
  }
  DeferredRegisterMouseMoveEventListener job{listener};
  return job.get(self);
}

EventListenerHandle WindowImpl::addMouseScrollEventListener(
    EventListenerRef<MouseScrollEvent> listener) {
  auto self = reinterpret_cast<WindowControlBlock*>(m_internals);
  if (self->closed) {
    return self->mouseScrollEventDispatcher.addListener(listener);
  }
  DeferredRegisterMouseScrollEventListener job{listener};
  return job.get(self);
}

EventListenerHandle WindowImpl::addShutdownEventListener(
    EventListenerRef<ShutdownEvent> listener) {
  auto self = reinterpret_cast<WindowControlBlock*>(m_internals);
  if (self->closed) {
    return self->shutdownEventDispatcher.addListener(listener);
  }
  DeferredRegisterShutdownEventListener job{listener};
  return job.get(self);
}

}  // namespace strobe::window
