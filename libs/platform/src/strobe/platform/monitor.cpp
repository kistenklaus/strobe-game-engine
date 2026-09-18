#include "strobe/platform/monitor.hpp"

#include "strobe/core/containers/linear_set.hpp"
#include "strobe/core/containers/vector.hpp"
#include "strobe/core/events/event_dispatcher.hpp"
#include "strobe/core/events/event_listener_handle.hpp"
#include "strobe/platform.hpp"
#include "strobe/platform/monitor_event.hpp"

#include <GLFW/glfw3.h>

#include <algorithm>
#include <atomic>
#include <cassert>
#include <cstdint>
#include <exception>
#include <mutex>
#include <utility>

#include <fmt/ostream.h>

namespace strobe::platform {

struct MonitorImpl {
  std::atomic<GLFWmonitor *> monitor;
  std::atomic<uint32_t> refCount;
};

namespace detail {
struct MonitorAccess {
  static Monitor make(MonitorImpl *impl) noexcept { return Monitor{impl}; }
};

static std::mutex g_monitor_mutex;
static LinearSet<MonitorImpl *> g_monitors;
static EventDispatcher<MonitorEvent> g_monitorEventDispatcher;

static void pin_monitor(MonitorImpl *impl) noexcept {
  assert(impl != nullptr);
  impl->refCount.fetch_add(1, std::memory_order_relaxed);
}

static void unpin_monitor(MonitorImpl *impl) noexcept {
  assert(impl != nullptr);
  if (impl->refCount.fetch_sub(1, std::memory_order_acq_rel) == 1) {
    delete impl;
  }
}

static void monitor_callback(GLFWmonitor *monitor, int event) {
  auto *impl = static_cast<MonitorImpl *>(glfwGetMonitorUserPointer(monitor));
  if (event == GLFW_CONNECTED) {
    assert(impl == nullptr);
    impl = new MonitorImpl{
        .monitor = monitor,
        .refCount = 1,
    };
    glfwSetMonitorUserPointer(monitor, impl);
    {
      std::lock_guard lck{g_monitor_mutex};
      g_monitors.insert(impl);
    }
    g_monitorEventDispatcher.dispatch(
        {MonitorAccess::make(impl),
         true}); // <- private constructors are anyoing as fuck!

  } else if (event == GLFW_DISCONNECTED) {
    assert(impl != nullptr);
    glfwSetMonitorUserPointer(monitor, nullptr);
    {
      std::lock_guard lck{g_monitor_mutex};
      const bool erased = g_monitors.erase(impl);
      assert(erased);
      impl->monitor.store(nullptr, std::memory_order_release);
    }
    g_monitorEventDispatcher.dispatch({MonitorAccess::make(impl), false});
    unpin_monitor(impl);
  } else {
    fmt::println("invalid monitor callback event");
    std::terminate();
  }
}

void monitor_entry() noexcept {
  glfwSetMonitorCallback(monitor_callback);
  int count = 0;
  GLFWmonitor **monitors = glfwGetMonitors(&count);
  std::lock_guard lck{g_monitor_mutex};
  for (int i = 0; i < count; ++i) {
    auto *impl = new MonitorImpl{
        .monitor = monitors[i],
        .refCount = 1,
    };
    glfwSetMonitorUserPointer(monitors[i], impl);
    g_monitors.insert(impl);
  }
}

void monitor_exit() noexcept {
  glfwSetMonitorCallback(nullptr);
  std::lock_guard lck{g_monitor_mutex};
  for (MonitorImpl *impl : g_monitors) {
    GLFWmonitor *monitor =
        impl->monitor.exchange(nullptr, std::memory_order_acq_rel);
    if (monitor != nullptr) {
      glfwSetMonitorUserPointer(monitor, nullptr);
    }
    unpin_monitor(impl);
  }
  g_monitors.clear();
}

} // namespace detail

[[nodiscard]] EventListenerHandle
add_monitor_listener(const EventListenerRef<MonitorEvent> &listener) noexcept {
  return detail::g_monitorEventDispatcher.addListener(listener);
}

Monitor::Monitor(void *internals) noexcept : m_internals(internals) {
  if (m_internals != nullptr) {
    detail::pin_monitor(static_cast<MonitorImpl *>(m_internals));
  }
}

Monitor::Monitor(const Monitor &o) noexcept : m_internals(o.m_internals) {
  if (m_internals != nullptr) {
    detail::pin_monitor(static_cast<MonitorImpl *>(m_internals));
  }
}

Monitor::Monitor(Monitor &&o) noexcept
    : m_internals(std::exchange(o.m_internals, nullptr)) {}

Monitor &Monitor::operator=(const Monitor &o) noexcept {
  if (this == &o) {
    return *this;
  }
  auto *next = static_cast<MonitorImpl *>(o.m_internals);
  auto *prev = static_cast<MonitorImpl *>(m_internals);
  if (next != nullptr) {
    detail::pin_monitor(next);
  }
  m_internals = next;
  if (prev != nullptr) {
    detail::unpin_monitor(prev);
  }
  return *this;
}

Monitor &Monitor::operator=(Monitor &&o) noexcept {
  if (this == &o) {
    return *this;
  }
  auto *prev = static_cast<MonitorImpl *>(m_internals);
  m_internals = std::exchange(o.m_internals, nullptr);
  if (prev != nullptr) {
    detail::unpin_monitor(prev);
  }
  return *this;
}

Monitor::~Monitor() noexcept {
  if (m_internals != nullptr) {
    detail::unpin_monitor(static_cast<MonitorImpl *>(m_internals));
  }
}

Vector<Monitor> Monitor::monitors() noexcept {
  std::lock_guard lck{detail::g_monitor_mutex};
  Vector<Monitor> out{};
  out.reserve(detail::g_monitors.size());
  for (MonitorImpl *impl : detail::g_monitors) {
    out.push_back(Monitor{impl});
  }
  return out;
}

Monitor Monitor::primary() noexcept {
  return platform::run([] {
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    const auto it =
        std::ranges::find_if(detail::g_monitors, [&](const MonitorImpl *impl) {
          return impl->monitor.load(std::memory_order_relaxed) == monitor;
        });
    if (it == detail::g_monitors.end()) {
      fmt::println("failed to get primary monitor");
      std::terminate();
    }
    return Monitor{*it};
  });
}

bool Monitor::connected() const noexcept {
  if (m_internals == nullptr) {
    return false;
  }
  auto *impl = static_cast<MonitorImpl *>(m_internals);
  return impl->monitor.load(std::memory_order_acquire) != nullptr;
}

[[nodiscard]] std::optional<std::string> Monitor::name() const {
  auto *impl = static_cast<MonitorImpl *>(m_internals);
  return platform::run([impl] -> std::optional<std::string> {
    GLFWmonitor *monitor = impl->monitor.load(std::memory_order_acquire);
    if (monitor == nullptr) {
      return std::nullopt;
    }
    const char *name = glfwGetMonitorName(monitor);
    assert(name != nullptr);
    return std::string{name};
  });
}

[[nodiscard]] std::optional<ivec2> Monitor::position() const noexcept {
  auto *impl = static_cast<MonitorImpl *>(m_internals);
  return platform::run([impl] -> std::optional<ivec2> {
    GLFWmonitor *monitor = impl->monitor.load(std::memory_order_acquire);
    if (monitor == nullptr) {
      return std::nullopt;
    }
    int xpos, ypos;
    glfwGetMonitorPos(monitor, &xpos, &ypos);
    return ivec2{
        static_cast<int32_t>(xpos),
        static_cast<int32_t>(ypos),
    };
  });
}

[[nodiscard]] std::optional<WorkArea> Monitor::workarea() const noexcept {
  auto *impl = static_cast<MonitorImpl *>(m_internals);
  return platform::run([impl] -> std::optional<WorkArea> {
    GLFWmonitor *monitor = impl->monitor.load(std::memory_order_acquire);
    if (monitor == nullptr) {
      return std::nullopt;
    }
    int xpos, ypos;
    int width, height;
    glfwGetMonitorWorkarea(monitor, &xpos, &ypos, &width, &height);
    return WorkArea{
        .offset =
            ivec2{
                static_cast<int32_t>(xpos),
                static_cast<int32_t>(ypos),
            },
        .extent =
            uvec2{
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height),
            },
    };
  });
}

[[nodiscard]] std::optional<uvec2> Monitor::physical_size_mm() const noexcept {
  auto *impl = static_cast<MonitorImpl *>(m_internals);
  return platform::run([impl] -> std::optional<uvec2> {
    GLFWmonitor *monitor = impl->monitor.load(std::memory_order_acquire);
    if (monitor == nullptr) {
      return std::nullopt;
    }
    int width, height;
    glfwGetMonitorPhysicalSize(monitor, &width, &height);
    return uvec2{
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height),
    };
  });
}

[[nodiscard]] std::optional<vec2> Monitor::content_scale() const noexcept {
  auto *impl = static_cast<MonitorImpl *>(m_internals);
  return platform::run([impl] -> std::optional<vec2> {
    GLFWmonitor *monitor = impl->monitor.load(std::memory_order_acquire);
    if (monitor == nullptr) {
      return std::nullopt;
    }
    float xscale, yscale;
    glfwGetMonitorContentScale(monitor, &xscale, &yscale);
    return vec2{xscale, yscale};
  });
}

[[nodiscard]] std::optional<VideoMode> Monitor::video_mode() const noexcept {
  auto *impl = static_cast<MonitorImpl *>(m_internals);
  return platform::run([impl] -> std::optional<VideoMode> {
    GLFWmonitor *monitor = impl->monitor.load(std::memory_order_acquire);
    if (monitor == nullptr) {
      return std::nullopt;
    }
    const GLFWvidmode *vidmode = glfwGetVideoMode(monitor);
    assert(vidmode != nullptr);
    return VideoMode{
        .resolution =
            uvec2{
                static_cast<uint32_t>(vidmode->width),
                static_cast<uint32_t>(vidmode->height),
            },
        .refreshRate = static_cast<uint32_t>(vidmode->refreshRate),
    };
  });
}

[[nodiscard]] std::optional<Vector<VideoMode>>
Monitor::supported_video_modes() const {
  auto *impl = static_cast<MonitorImpl *>(m_internals);
  return platform::run([impl] -> std::optional<Vector<VideoMode>> {
    GLFWmonitor *monitor = impl->monitor.load(std::memory_order_acquire);
    if (monitor == nullptr) {
      return std::nullopt;
    }
    int count = 0;
    const GLFWvidmode *vidmodes = glfwGetVideoModes(monitor, &count);
    assert(vidmodes != nullptr);
    Vector<VideoMode> result;
    result.reserve(static_cast<size_t>(count));
    for (int i = 0; i < count; ++i) {
      const GLFWvidmode &vidmode = vidmodes[i];
      result.push_back(VideoMode{
          .resolution =
              uvec2{
                  static_cast<uint32_t>(vidmode.width),
                  static_cast<uint32_t>(vidmode.height),
              },
          .refreshRate = static_cast<uint32_t>(vidmode.refreshRate),
      });
    }
    return result;
  });
}

[[nodiscard]] std::optional<MonitorInfo> Monitor::info() const noexcept {
  auto *impl = static_cast<MonitorImpl *>(m_internals);
  return platform::run([impl] -> std::optional<MonitorInfo> {
    GLFWmonitor *monitor = impl->monitor.load(std::memory_order_acquire);
    if (monitor == nullptr) {
      return std::nullopt;
    }
    int xpos, ypos;
    glfwGetMonitorPos(monitor, &xpos, &ypos);
    int workX, workY;
    int workWidth, workHeight;
    glfwGetMonitorWorkarea(monitor, &workX, &workY, &workWidth, &workHeight);
    float xscale, yscale;
    glfwGetMonitorContentScale(monitor, &xscale, &yscale);
    const GLFWvidmode *vidmode = glfwGetVideoMode(monitor);
    assert(vidmode != nullptr);
    return MonitorInfo{
        .position =
            ivec2{
                static_cast<int32_t>(xpos),
                static_cast<int32_t>(ypos),
            },
        .workArea =
            WorkArea{
                .offset =
                    ivec2{
                        static_cast<int32_t>(workX),
                        static_cast<int32_t>(workY),
                    },
                .extent =
                    uvec2{
                        static_cast<uint32_t>(workWidth),
                        static_cast<uint32_t>(workHeight),
                    },
            },
        .contentScale = vec2{xscale, yscale},
        .videoMode =
            VideoMode{
                .resolution =
                    uvec2{
                        static_cast<uint32_t>(vidmode->width),
                        static_cast<uint32_t>(vidmode->height),
                    },
                .refreshRate = static_cast<uint32_t>(vidmode->refreshRate),
            },
    };
  });
}

void *Monitor::native() const noexcept {
  if (m_internals == nullptr) {
    return nullptr;
  }
  auto *impl = static_cast<MonitorImpl *>(m_internals);
  return impl->monitor.load(std::memory_order_acquire);
}

} // namespace strobe::platform
