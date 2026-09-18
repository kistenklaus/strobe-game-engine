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
  platform::WindowState windowState;
  platform::Keyboard keyboard;
  platform::Mouse mouse;

  explicit Viewport(uvec2 size, const char *name, bool decorated, bool topMost,
                    ImGuiIO *io, std::mutex *ioMutex) noexcept
      : m_owned_window(strobe::platform::Window{{
            .size = size,
            .title = name,
            .decorated = decorated,
            .floating = topMost,
        }}),
        window(&m_owned_window.value()), windowState(window, io, ioMutex),
        keyboard(window, io, ioMutex), mouse(window, &windowState, io, ioMutex) {}

  explicit Viewport(strobe::platform::Window *window, ImGuiIO *io,
                    std::mutex *ioMutex)
      : m_owned_window{}, window(window), windowState(window, io, ioMutex),
        keyboard(window, io, ioMutex), mouse(window, &windowState, io, ioMutex) {}
};

} // namespace strobe::imgui::platform
