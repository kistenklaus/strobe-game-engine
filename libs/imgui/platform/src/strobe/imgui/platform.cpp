#include "strobe/imgui/platform.hpp"

#include "imgui.h"
#include "strobe/core/containers/string.hpp"
#include "strobe/core/containers/vector_deque.hpp"
#include "strobe/core/events/event_listener.hpp"
#include "strobe/core/events/event_listener_handle.hpp"
#include "strobe/imgui/platform/viewport.hpp"
#include "strobe/imgui/platform/window_state.hpp"
#include "strobe/platform.hpp"
#include "strobe/platform/monitor.hpp"
#include "strobe/platform/monitor_event.hpp"

#include <atomic>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <fmt/ostream.h>
#include <mutex>
#include <utility>
#include <variant>

namespace strobe::imgui {

struct PlatformImpl;

static void create_window(ImGuiViewport *viewport);
static void destroy_window(ImGuiViewport *viewport);
static void show_window(ImGuiViewport *viewport);
static void set_window_pos(ImGuiViewport *viewport, ImVec2 pos);
static ImVec2 get_window_pos(ImGuiViewport *viewport);
static void set_window_size(ImGuiViewport *viewport, ImVec2 size);
static ImVec2 get_window_size(ImGuiViewport *viewport);
static void set_window_focus(ImGuiViewport *viewport);
static bool get_window_focus(ImGuiViewport *viewport);
static bool get_window_minimized(ImGuiViewport *viewport);
static void set_window_title(ImGuiViewport *viewport, const char *title);
static void set_window_alpha(ImGuiViewport *viewport, float alpha);
static void flush_platform_events(PlatformImpl &impl) noexcept;

static void monitor_callback(void *,
                             const strobe::platform::MonitorEvent &) noexcept;

struct DestroyWindowEvent {
  platform::Viewport *viewport;
};

struct ShowWindowEvent {
  platform::Viewport *viewport;
};

struct SetWindowPosEvent {
  platform::Viewport *viewport;
  ivec2 pos;
};

struct SetWindowSizeEvent {
  platform::Viewport *viewport;
  uvec2 size;
};

struct SetWindowFocusEvent {
  platform::Viewport *viewport;
};

struct SetWindowTitleEvent {
  platform::Viewport *viewport;
  String<> title;
};

struct SetWindowAlphaEvent {
  platform::Viewport *viewport;
  float alpha;
};

using PlatformEvent =
    std::variant<DestroyWindowEvent, ShowWindowEvent, SetWindowPosEvent,
                 SetWindowSizeEvent, SetWindowFocusEvent, SetWindowTitleEvent,
                 SetWindowAlphaEvent>;

struct PlatformImpl {
  ImGuiIO *io;
  std::mutex ioMutex;
  platform::Viewport mainViewport;
  VectorDeque<PlatformEvent> events;

  std::atomic<bool> monitorsDirty{true};
  EventListenerHandle monitorCallbackHandle;

  explicit PlatformImpl(strobe::platform::Window *window) noexcept
      : io(&ImGui::GetIO()), ioMutex{}, mainViewport{window, io, &ioMutex},
        monitorCallbackHandle(strobe::platform::add_monitor_listener(
            strobe::EventListenerRef<strobe::platform::MonitorEvent>::
                fromNative(this, monitor_callback))) {
    io->BackendPlatformUserData = this;
    io->BackendPlatformName = "strobe-platform";

    // io->BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
    io->BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
    io->BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;

    auto &platformIO = ImGui::GetPlatformIO();

    platformIO.Platform_CreateWindow = create_window;
    platformIO.Platform_DestroyWindow = destroy_window;
    platformIO.Platform_ShowWindow = show_window;
    platformIO.Platform_SetWindowPos = set_window_pos;
    platformIO.Platform_GetWindowPos = get_window_pos;
    platformIO.Platform_SetWindowSize = set_window_size;
    platformIO.Platform_GetWindowSize = get_window_size;
    platformIO.Platform_SetWindowFocus = set_window_focus;
    platformIO.Platform_GetWindowFocus = get_window_focus;
    platformIO.Platform_GetWindowMinimized = get_window_minimized;
    platformIO.Platform_SetWindowTitle = set_window_title;
    platformIO.Platform_SetWindowAlpha = set_window_alpha;

    auto *main = ImGui::GetMainViewport();
    main->PlatformHandle = window->window_ptr();
    main->PlatformUserData = &mainViewport;
  }

  PlatformImpl(const PlatformImpl &) = delete;
  PlatformImpl &operator=(const PlatformImpl &) = delete;
  PlatformImpl(PlatformImpl &&) = delete;
  PlatformImpl &operator=(PlatformImpl &&) = delete;

  ~PlatformImpl() noexcept {
    ImGui::DestroyPlatformWindows();
    flush_platform_events(*this);

    // After this returns, monitor_callback() can no longer access `this`.
    monitorCallbackHandle.release();

    io->BackendPlatformUserData = nullptr;
    io->BackendPlatformName = nullptr;

    io->BackendFlags &= ~(ImGuiBackendFlags_HasSetMousePos |
                          ImGuiBackendFlags_PlatformHasViewports);

    auto &platformIO = ImGui::GetPlatformIO();

    platformIO.Platform_CreateWindow = nullptr;
    platformIO.Platform_DestroyWindow = nullptr;
    platformIO.Platform_ShowWindow = nullptr;
    platformIO.Platform_SetWindowPos = nullptr;
    platformIO.Platform_GetWindowPos = nullptr;
    platformIO.Platform_SetWindowSize = nullptr;
    platformIO.Platform_GetWindowSize = nullptr;
    platformIO.Platform_SetWindowFocus = nullptr;
    platformIO.Platform_GetWindowFocus = nullptr;
    platformIO.Platform_GetWindowMinimized = nullptr;
    platformIO.Platform_SetWindowTitle = nullptr;
    platformIO.Platform_SetWindowAlpha = nullptr;
  }
};

static PlatformImpl &get_platform_impl() noexcept {
  auto *impl =
      static_cast<PlatformImpl *>(ImGui::GetIO().BackendPlatformUserData);
  assert(impl);
  return *impl;
}

struct PlatformEventExecutor {
  void operator()(DestroyWindowEvent &event) const noexcept {
    delete event.viewport;
  }

  void operator()(ShowWindowEvent &event) const noexcept {
    event.viewport->window->visible(true);
  }

  void operator()(SetWindowPosEvent &event) const noexcept {
    event.viewport->window->position(event.pos);
  }

  void operator()(SetWindowSizeEvent &event) const noexcept {
    event.viewport->window->size(event.size);
  }

  void operator()(SetWindowFocusEvent &event) const noexcept {
    event.viewport->window->focus();
  }

  void operator()(SetWindowTitleEvent &event) const noexcept {
    event.viewport->window->title(event.title.c_str());
  }

  void operator()(SetWindowAlphaEvent &event) const noexcept {
    event.viewport->window->opacity(event.alpha);
  }
};

static void flush_platform_events(PlatformImpl &impl) noexcept {
  if (impl.events.empty()) {
    return;
  }

  VectorDeque<PlatformEvent> events = std::move(impl.events);
  strobe::platform::run([&events] noexcept {
    while (!events.empty()) {
      PlatformEvent event = std::move(events.front());
      events.pop_front();
      std::visit(PlatformEventExecutor{}, event);
    }
  });
}

static void destroy_window(ImGuiViewport *viewport) {
  assert(viewport);

  auto *view = static_cast<platform::Viewport *>(viewport->PlatformUserData);
  viewport->PlatformUserData = nullptr;
  viewport->PlatformHandle = nullptr;
  viewport->PlatformHandleRaw = nullptr;

  if (!view) {
    return;
  }

  auto &impl = get_platform_impl();
  if (view != &impl.mainViewport) {
    impl.events.emplace_back(DestroyWindowEvent{view});
  }
}

static void show_window(ImGuiViewport *viewport) {
  assert(viewport);
  auto *view = static_cast<platform::Viewport *>(viewport->PlatformUserData);
  assert(view);
  get_platform_impl().events.emplace_back(ShowWindowEvent{view});
}

static void set_window_pos(ImGuiViewport *viewport, ImVec2 pos) {
  assert(viewport);
  auto *view = static_cast<platform::Viewport *>(viewport->PlatformUserData);
  assert(view);
  get_platform_impl().events.emplace_back(SetWindowPosEvent{
      view,
      ivec2{
          static_cast<int32_t>(std::round(pos.x)),
          static_cast<int32_t>(std::round(pos.y)),
      },
  });
}

static ImVec2 get_window_pos(ImGuiViewport *viewport) {
  assert(viewport);
  auto *view = static_cast<platform::Viewport *>(viewport->PlatformUserData);
  assert(view);
  const ivec2 pos = view->windowState.pos();
  return ImVec2{
      static_cast<float>(pos.x()),
      static_cast<float>(pos.y()),
  };
}

static void set_window_size(ImGuiViewport *viewport, ImVec2 size) {
  assert(viewport);
  auto *view = static_cast<platform::Viewport *>(viewport->PlatformUserData);
  assert(view);
  get_platform_impl().events.emplace_back(SetWindowSizeEvent{
      view,
      uvec2{
          static_cast<uint32_t>(std::round(size.x)),
          static_cast<uint32_t>(std::round(size.y)),
      },
  });
}

static ImVec2 get_window_size(ImGuiViewport *viewport) {
  assert(viewport);
  auto *view = static_cast<platform::Viewport *>(viewport->PlatformUserData);
  assert(view);
  const uvec2 size = view->windowState.size();
  return ImVec2{
      static_cast<float>(size.x()),
      static_cast<float>(size.y()),
  };
}

static void set_window_focus(ImGuiViewport *viewport) {
  assert(viewport);
  auto *view = static_cast<platform::Viewport *>(viewport->PlatformUserData);
  assert(view);
  get_platform_impl().events.emplace_back(SetWindowFocusEvent{view});
}

static bool get_window_focus(ImGuiViewport *viewport) {
  assert(viewport);
  auto *view = static_cast<platform::Viewport *>(viewport->PlatformUserData);
  assert(view);
  return view->windowState.focused();
}

static bool get_window_minimized(ImGuiViewport *viewport) {
  assert(viewport);
  auto *view = static_cast<platform::Viewport *>(viewport->PlatformUserData);
  assert(view);
  return view->windowState.minimized();
}

static void set_window_title(ImGuiViewport *viewport, const char *title) {
  assert(viewport);
  assert(title);
  auto *view = static_cast<platform::Viewport *>(viewport->PlatformUserData);
  assert(view);
  get_platform_impl().events.emplace_back(
      SetWindowTitleEvent{view, String<>{title}});
}

static void set_window_alpha(ImGuiViewport *viewport, float alpha) {
  assert(viewport);
  auto *view = static_cast<platform::Viewport *>(viewport->PlatformUserData);
  assert(view);
  get_platform_impl().events.emplace_back(SetWindowAlphaEvent{view, alpha});
}

static void monitor_callback(void *ptr,
                             const strobe::platform::MonitorEvent &) noexcept {
  assert(ptr);
  auto *impl = static_cast<PlatformImpl *>(ptr);
  impl->monitorsDirty.store(true, std::memory_order_release);
}

static void create_window(ImGuiViewport *viewport) {
  assert(viewport);
  auto &io = ImGui::GetIO();
  auto *impl = static_cast<PlatformImpl *>(io.BackendPlatformUserData);
  assert(impl);
  const uvec2 size{
      static_cast<uint32_t>(std::round(viewport->Size.x)),
      static_cast<uint32_t>(std::round(viewport->Size.y)),
  };
  auto *view = new platform::Viewport{
      size,
      "strobe-imgui",
      !(viewport->Flags & ImGuiViewportFlags_NoDecoration),
      static_cast<bool>(viewport->Flags & ImGuiViewportFlags_TopMost),
      impl->io,
      &impl->ioMutex};
  viewport->PlatformUserData = view;
  viewport->PlatformHandle = view->window->window_ptr();
}

static void update_monitors() noexcept {
  auto &platformIO = ImGui::GetPlatformIO();

  platformIO.Monitors.clear();

  const auto monitors = strobe::platform::Monitor::monitors();
  platformIO.Monitors.reserve(static_cast<int>(monitors.size()));

  for (const auto &monitor : monitors) {
    const auto info = monitor.info();

    // The monitor may have disappeared after Monitor::monitors()
    // produced its snapshot.
    if (!info.has_value()) {
      continue;
    }

    ImGuiPlatformMonitor imguiMonitor{};

    imguiMonitor.MainPos = ImVec2{
        static_cast<float>(info->position.x()),
        static_cast<float>(info->position.y()),
    };

    imguiMonitor.MainSize = ImVec2{
        static_cast<float>(info->videoMode.resolution.x()),
        static_cast<float>(info->videoMode.resolution.y()),
    };

    imguiMonitor.WorkPos = ImVec2{
        static_cast<float>(info->workArea.offset.x()),
        static_cast<float>(info->workArea.offset.y()),
    };

    imguiMonitor.WorkSize = ImVec2{
        static_cast<float>(info->workArea.extent.x()),
        static_cast<float>(info->workArea.extent.y()),
    };

    // GLFW exposes independent X/Y scales while ImGui currently
    // represents monitor DPI using one scalar.
    imguiMonitor.DpiScale = info->contentScale.x();

    platformIO.Monitors.push_back(imguiMonitor);
  }
}

static void update_viewport_requests() noexcept {
  auto &platformIO = ImGui::GetPlatformIO();

  for (int i = 1; i < platformIO.Viewports.Size; ++i) {
    ImGuiViewport *viewport = platformIO.Viewports[i];
    auto *view = static_cast<platform::Viewport *>(viewport->PlatformUserData);

    if (!view) {
      continue;
    }

    if (view->windowState.consume_pos_changed()) {
      viewport->PlatformRequestMove = true;
    }
    if (view->windowState.consume_size_changed()) {
      viewport->PlatformRequestResize = true;
    }
    if (view->windowState.should_close()) {
      viewport->PlatformRequestClose = true;
    }
  }
}

Platform::Platform(strobe::platform::Window *window) noexcept
    : m_internals(new PlatformImpl(window)) {}

Platform::Platform(Platform &&o) noexcept
    : m_internals(std::exchange(o.m_internals, nullptr)) {}

Platform &Platform::operator=(Platform &&o) noexcept {
  if (this == &o) {
    return *this;
  }

  if (m_internals) {
    delete static_cast<PlatformImpl *>(m_internals);
  }

  m_internals = std::exchange(o.m_internals, nullptr);
  return *this;
}

Platform::~Platform() noexcept {
  if (m_internals) {
    delete static_cast<PlatformImpl *>(m_internals);
  }
}

void Platform::new_frame() noexcept {
  auto *impl = static_cast<PlatformImpl *>(m_internals);
  assert(impl);

  if (impl->monitorsDirty.exchange(false, std::memory_order_acq_rel)) {
    update_monitors();
  }

  const auto size = impl->mainViewport.windowState.size();
  const auto framebufferScale =
      impl->mainViewport.windowState.framebuffer_scale();

  {
    std::lock_guard lock{impl->ioMutex};

    update_viewport_requests();

    impl->io->DisplaySize = ImVec2{
        static_cast<float>(size.x()),
        static_cast<float>(size.y()),
    };

    impl->io->DisplayFramebufferScale = ImVec2{
        framebufferScale.x(),
        framebufferScale.y(),
    };

    impl->io->DeltaTime = 1.0f / 60.0f;

    ImGui::NewFrame();
  }

  flush_platform_events(*impl);
}

} // namespace strobe::imgui
