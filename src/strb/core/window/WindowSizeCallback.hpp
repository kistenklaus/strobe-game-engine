#pragma once
#include <cinttypes>

namespace strb::window {

class WindowSizeCallback {
 public:
  WindowSizeCallback() = default;
  virtual ~WindowSizeCallback() = default;
  WindowSizeCallback(WindowSizeCallback&) = default;
  WindowSizeCallback(WindowSizeCallback&&) = default;
  virtual void callback(const uint32_t width, const uint32_t height) = 0;
};

}  // namespace strb::window
