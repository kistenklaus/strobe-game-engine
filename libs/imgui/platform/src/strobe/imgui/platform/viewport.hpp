#pragma once

#include "imgui.h"
#include "strobe/imgui/platform/keyboard.hpp"
#include "strobe/imgui/platform/mouse.hpp"
#include "strobe/imgui/platform/window_state.hpp"
namespace strobe::imgui::platform {

class Viewport {
  std::optional<strobe::platform::Window> m_owned_window;

public:
  strobe::platform::Window *window;
  platform::Keyboard keyboard;
  platform::Mouse mouse;
  platform::WindowState windowState;

  explicit Viewport(uvec2 size, const char *name, ImGuiIO *io,
                    std::mutex *ioMutex) noexcept
      : m_owned_window(strobe::platform::Window{size, name}),
        window(&m_owned_window.value()), keyboard(window, io, ioMutex),
        mouse(window, io, ioMutex), windowState(window, io, ioMutex) {}

  explicit Viewport(strobe::platform::Window *window, ImGuiIO *io,
                    std::mutex *ioMutex)
      : m_owned_window{}, window(window), keyboard(window, io, ioMutex),
        mouse(window, io, ioMutex), windowState(window, io, ioMutex) {}
};

} // namespace strobe::imgui::platform
