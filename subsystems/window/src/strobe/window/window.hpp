#pragma once

#include "strobe/core/type_traits/fixed_string.hpp"
#include "strobe/ecs/resource_commands.hpp"
#include "strobe/window/window_impl.hpp"

namespace strobe {

template <fixed_string Name = fixed_string{"main"}> class Window {
public:
  static constexpr auto name = fixed_string{"window-"} + Name;

  explicit Window() : m_impl({400, 400}, name.data()) {
    // create
  }
  ~Window() = default;

  void start() noexcept {
    m_impl.show();
    m_impl.resizable(true);
  }

  void update(ResourceCommands rcmds) noexcept {
    m_impl.poll();
    if (m_impl.should_close()) {
      rcmds.destroy<Window>();
    }
    // fmt::println("size = ({}x{})", m_impl.framebuffer_size().x(),
    //              m_impl.framebuffer_size().y());
  }

  void stop() noexcept { m_impl.hide(); }

private:
  window::WindowImpl m_impl;
};

} // namespace strobe
