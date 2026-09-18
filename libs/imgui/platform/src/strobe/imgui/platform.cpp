#include "strobe/imgui/platform.hpp"

#include "imgui.h"
#include "strobe/core/events/event_listener.hpp"
#include "strobe/core/events/event_listener_handle.hpp"
#include "strobe/imgui/platform/viewport.hpp"
#include "strobe/imgui/platform/window_state.hpp"
#include "strobe/platform/monitor.hpp"
#include "strobe/platform/monitor_event.hpp"

#include <atomic>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <fmt/ostream.h>
#include <mutex>
#include <utility>

namespace strobe::imgui {

struct PlatformImpl;

static void create_window(ImGuiViewport *viewport);

static void destroy_window(ImGuiViewport *viewport) {
  assert(viewport);
  assert(viewport->PlatformUserData);

  auto *impl = static_cast<platform::Viewport *>(viewport->PlatformUserData);

  delete impl;

  viewport->PlatformUserData = nullptr;
  viewport->PlatformHandle = nullptr;
  viewport->PlatformHandleRaw = nullptr;
}

static void show_window(ImGuiViewport *viewport) {
  assert(viewport);

  auto *impl = static_cast<platform::Viewport *>(viewport->PlatformUserData);
  assert(impl);

  impl->window->visible(true);
}

static void set_window_pos(ImGuiViewport *viewport, ImVec2 pos) {
  assert(viewport);

  auto *impl = static_cast<platform::Viewport *>(viewport->PlatformUserData);
  assert(impl);

  impl->window->position(ivec2{
      static_cast<int32_t>(std::round(pos.x)),
      static_cast<int32_t>(std::round(pos.y)),
  });
}

static ImVec2 get_window_pos(ImGuiViewport *viewport) {
  assert(viewport);

  auto *impl = static_cast<platform::Viewport *>(viewport->PlatformUserData);
  assert(impl);

  const ivec2 pos = impl->window->position();

  return ImVec2{
      static_cast<float>(pos.x()),
      static_cast<float>(pos.y()),
  };
}

static void set_window_size(ImGuiViewport *viewport, ImVec2 size) {
  assert(viewport);

  auto *impl = static_cast<platform::Viewport *>(viewport->PlatformUserData);
  assert(impl);

  impl->window->size(uvec2{
      static_cast<uint32_t>(std::round(size.x)),
      static_cast<uint32_t>(std::round(size.y)),
  });
}

static ImVec2 get_window_size(ImGuiViewport *viewport) {
  assert(viewport);

  auto *impl = static_cast<platform::Viewport *>(viewport->PlatformUserData);
  assert(impl);

  const uvec2 size = impl->window->size();

  return ImVec2{
      static_cast<float>(size.x()),
      static_cast<float>(size.y()),
  };
}

static void set_window_focus(ImGuiViewport *viewport) {
  assert(viewport);

  auto *impl = static_cast<platform::Viewport *>(viewport->PlatformUserData);
  assert(impl);

  impl->window->focus();
}

static bool get_window_focus(ImGuiViewport *viewport) {
  assert(viewport);

  auto *impl = static_cast<platform::Viewport *>(viewport->PlatformUserData);
  assert(impl);

  return impl->window->focused();
}

static bool get_window_minimized(ImGuiViewport *viewport) {
  assert(viewport);

  auto *impl = static_cast<platform::Viewport *>(viewport->PlatformUserData);
  assert(impl);

  return impl->window->minimized();
}

static void set_window_title(ImGuiViewport *viewport, const char *title) {
  assert(viewport);
  assert(title);

  auto *impl = static_cast<platform::Viewport *>(viewport->PlatformUserData);
  assert(impl);

  impl->window->title(title);
}

static void set_window_alpha(ImGuiViewport *viewport, float alpha) {
  assert(viewport);

  auto *impl = static_cast<platform::Viewport *>(viewport->PlatformUserData);
  assert(impl);

  impl->window->opacity(alpha);
}

static void monitor_callback(void *,
                             const strobe::platform::MonitorEvent &) noexcept;

struct PlatformImpl {
  ImGuiIO *io;
  std::mutex ioMutex;
  platform::Viewport mainViewport;

  std::atomic<bool> monitorsDirty{true};
  EventListenerHandle monitorCallbackHandle;

  explicit PlatformImpl(strobe::platform::Window *window) noexcept
      : io(&ImGui::GetIO()), ioMutex{}, mainViewport{window, io, &ioMutex},
        monitorCallbackHandle(strobe::platform::add_monitor_listener(
            strobe::EventListenerRef<strobe::platform::MonitorEvent>::
                fromNative(this, monitor_callback))) {
    io->BackendPlatformUserData = this;
    io->BackendPlatformName = "strobe-platform";

    // TODO: ImGuiBackendFlags_HasMouseCursors
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

  auto *view = new platform::Viewport{size, "", impl->io, &impl->ioMutex};

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

  std::lock_guard lock{impl->ioMutex};

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

} // namespace strobe::imgui
