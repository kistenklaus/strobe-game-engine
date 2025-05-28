#include <GLFW/glfw3.h>

#include <cstring>
#include <future>
#include <mutex>
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

  virtual std::optional<std::tuple<std::size_t, std::size_t>> allocationInfo()
      const {
    return std::nullopt;
  }
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
  EventDispatcher<KeyboardEvent, PolyAllocatorReference>
      keyboardEventDispatcher;
  EventDispatcher<ResizeEvent, PolyAllocatorReference>
      framebufferSizeEventDispatcher;

  strobe::mpsc::Sender<DeferredJob*> deferredJobTx;

  explicit WindowControlBlock(PolyAllocatorReference allocator,
                              strobe::mpsc::Sender<DeferredJob*> deferredJobTx,
                              uvec2 size,
                              Vector<char, PolyAllocatorReference> ctitle)
      : window(nullptr),
        thread({}),
        allocator(allocator),
        mouseButtonEventDispatcher(allocator),
        keyboardEventDispatcher(allocator),
        framebufferSizeEventDispatcher(allocator),
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

  void operator()(WindowControlBlock* cb) override {
    cb->mouseButtonEventDispatcher.removeListener(m_id);
  }

  std::optional<std::tuple<std::size_t, std::size_t>> allocationInfo()
      const override {
    return std::make_tuple(sizeof(DeferredUnregisterMouseButtonEventListener),
                           alignof(DeferredUnregisterMouseButtonEventListener));
  }

 private:
  events::EventListenerId m_id;
};

static void deferred_mouse_button_listener_handle_unregister(
    void* userData, events::EventListenerId id) {
  auto cb = reinterpret_cast<WindowControlBlock*>(userData);
  using ATraits = AllocatorTraits<decltype(cb->allocator)>;
  auto job = ATraits::allocate<DeferredUnregisterMouseButtonEventListener>(
      cb->allocator);
  new (job) DeferredUnregisterMouseButtonEventListener(id);
  while (!cb->deferredJobTx.send(job)) {
    glfwPostEmptyEvent();
    std::this_thread::yield();
  }
  glfwPostEmptyEvent();
}

class DeferredRegisterMouseButtonEventListener : public DeferredJob {
 public:
  DeferredRegisterMouseButtonEventListener(
      EventListenerRef<MouseButtonEvent> listener)
      : m_listener(std::move(listener)) {}

  EventListenerHandle get(WindowControlBlock* cb) {
    auto fut = m_handle.get_future();
    while (!cb->deferredJobTx.send(this)) {
      glfwPostEmptyEvent();
      std::this_thread::yield();
    }
    glfwPostEmptyEvent();
    return fut.get();
  }

  void operator()(WindowControlBlock* cb) override {
    auto handle = cb->mouseButtonEventDispatcher.addListener(m_listener);
    glfwSetMouseButtonCallback(cb->window, mouse_button_callback);
    handle.deferr(cb, deferred_mouse_button_listener_handle_unregister);
    m_handle.set_value(std::move(handle));
  }

 private:
  EventListenerRef<MouseButtonEvent> m_listener;
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

  void operator()(WindowControlBlock* cb) override {
    cb->keyboardEventDispatcher.removeListener(m_id);
  }

  std::optional<std::tuple<std::size_t, std::size_t>> allocationInfo()
      const override {
    return std::make_tuple(sizeof(DeferredUnregisterKeyEventListener),
                           alignof(DeferredUnregisterKeyEventListener));
  }

 private:
  events::EventListenerId m_id;
};

static void deferred_keyboard_handle_unregister(void* userData,
                                                events::EventListenerId id) {
  auto cb = reinterpret_cast<WindowControlBlock*>(userData);
  using ATraits = AllocatorTraits<decltype(cb->allocator)>;
  auto job =
      ATraits::allocate<DeferredUnregisterKeyEventListener>(cb->allocator);
  new (job) DeferredUnregisterKeyEventListener(id);
  while (!cb->deferredJobTx.send(job)) {
    glfwPostEmptyEvent();
    std::this_thread::yield();
  }
  glfwPostEmptyEvent();
}

class DeferredRegisterKeyEventListener : public DeferredJob {
 public:
  DeferredRegisterKeyEventListener(EventListenerRef<KeyboardEvent> listener)
      : m_listener(std::move(listener)) {}

  EventListenerHandle get(WindowControlBlock* cb) {
    std::future<EventListenerHandle> fut = m_handle.get_future();
    while (!cb->deferredJobTx.send(this)) {
      glfwPostEmptyEvent();       // <- avoid beeing stuck in a loop
      std::this_thread::yield();  // <- should essentially never ever happen.
    }
    glfwPostEmptyEvent();  // <- avoid beeing stuck in a loop
    return std::move(fut.get());
  }

  void operator()(WindowControlBlock* cb) override {
    auto handle = cb->keyboardEventDispatcher.addListener(m_listener);
    glfwSetKeyCallback(cb->window, keyboard_callback);
    handle.deferr(cb, deferred_keyboard_handle_unregister);
    m_handle.set_value(std::move(handle));
  }

 private:
  EventListenerRef<KeyboardEvent> m_listener;
  std::promise<EventListenerHandle> m_handle;
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
  auto cb =
      reinterpret_cast<WindowControlBlock*>(glfwGetWindowUserPointer(window));
  cb->framebufferSizeEventDispatcher.dispatch(uvec2(width, height));
}

class DeferredUnregisterFramebufferSizeListener : public DeferredJob {
 public:
  DeferredUnregisterFramebufferSizeListener(events::EventListenerId id)
      : m_id(std::move(id)) {}

  void operator()(WindowControlBlock* cb) override {
    cb->framebufferSizeEventDispatcher.removeListener(m_id);
  }

  std::optional<std::tuple<std::size_t, std::size_t>> allocationInfo()
      const override {
    return std::make_tuple(sizeof(DeferredUnregisterFramebufferSizeListener),
                           alignof(DeferredUnregisterFramebufferSizeListener));
  }

 private:
  events::EventListenerId m_id;
};

void deferred_framebuffer_size_handle_unregister(void* userData,
                                                 events::EventListenerId id) {
  auto cb = reinterpret_cast<WindowControlBlock*>(userData);
  using ATraits = AllocatorTraits<decltype(cb->allocator)>;
  auto job = ATraits::allocate<DeferredUnregisterFramebufferSizeListener>(
      cb->allocator);
  new (job) DeferredUnregisterFramebufferSizeListener(id);
  while (!cb->deferredJobTx.send(job)) {
    glfwPostEmptyEvent();
    std::this_thread::yield();
  }
  // can happen whenever <- just to clean up the buffers properly.
  glfwPostEmptyEvent();
}

class DeferredRegisterFramebufferSizeListener : public DeferredJob {
 public:
  DeferredRegisterFramebufferSizeListener(
      EventListenerRef<ResizeEvent> listener)
      : m_listener(std::move(listener)) {}

  EventListenerHandle get(WindowControlBlock* cb) {
    auto fut = m_handle.get_future();

    while (!cb->deferredJobTx.send(this)) {
      glfwPostEmptyEvent();
      std::this_thread::yield();
    }
    glfwPostEmptyEvent();
    return fut.get();
  }

  void operator()(WindowControlBlock* cb) override {
    auto handle = cb->framebufferSizeEventDispatcher.addListener(m_listener);
    glfwSetFramebufferSizeCallback(cb->window, framebuffer_size_callback);
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
    auto fut = m_comp.get_future();
    while (!cb->deferredJobTx.send(this)) {
      glfwPostEmptyEvent();
      std::this_thread::yield();
    }
    glfwPostEmptyEvent();
    fut.wait();
  }

  void operator()(WindowControlBlock* cb) override {
    if (m_attrib == GLFW_RESIZABLE) {
      std::lock_guard lck{cb->attribMutex};
      cb->resizable = m_value == GLFW_TRUE;
    }
    glfwSetWindowAttrib(cb->window, m_attrib, m_value);
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
    auto fut = m_comp.get_future();
    while (!cb->deferredJobTx.send(this)) {
      glfwPostEmptyEvent();
      std::this_thread::yield();
    }
    glfwPostEmptyEvent();
    fut.wait();
  }

  void operator()(WindowControlBlock* cb) override {
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
    auto fut = m_comp.get_future();
    while (!cb->deferredJobTx.send(this)) {
      glfwPostEmptyEvent();
      std::this_thread::yield();
    }
    glfwPostEmptyEvent();
    fut.wait();
  }

  void operator()(WindowControlBlock* cb) override {
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

  cb->thread.get_stop_source().request_stop();
  glfwPostEmptyEvent();
}

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
  glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
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

  glfwSetWindowAttrib(self->window, GLFW_VISIBLE, GLFW_TRUE);

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

  asyncInitReturn.set_value(WindowCreateErno::OK);

  while (!stop_token.stop_requested()) {
    glfwWaitEvents();
    std::optional<DeferredJob*> job;
    while ((job = deferredRx.recv()).has_value()) {
      (**job)(self);

      // optionally deallocate the job
      const auto info = (*job)->allocationInfo();
      if (info.has_value()) {
        auto [size, align] = info.value();
        self->allocator.deallocate(*job, size, align);
      }
    }
  }
  self->closed = true;
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
  std::future<WindowCreateErno> asyncErno = asyncReturn.get_future();
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
    using ATraits = AllocatorTraits<PolyAllocatorReference>;
    auto self = reinterpret_cast<WindowControlBlock*>(m_internals);
    PolyAllocatorReference alloc = self->allocator;
    self->~WindowControlBlock();  // <- implicity joins the thread.
    ATraits::deallocate<WindowControlBlock>(alloc, self);
  }
}

void WindowImpl::close() {
  auto self = reinterpret_cast<WindowControlBlock*>(m_internals);
  std::jthread thread = std::move(self->thread);
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
  return job.get(self);  // <- waits for the deferred job to complete.
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

}  // namespace strobe::window
