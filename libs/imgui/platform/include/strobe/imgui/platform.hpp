#pragma once

#include "strobe/platform/window.hpp"
namespace strobe::imgui {

class Platform {
public:
  explicit Platform(platform::Window *window) noexcept;
  Platform(const Platform &) = delete;
  Platform(Platform &&) noexcept;
  Platform &operator=(const Platform &) = delete;
  Platform &operator=(Platform &&) noexcept;
  ~Platform() noexcept;

  void new_frame() noexcept;

private:
  void *m_internals;
};

} // namespace strobe::imgui
