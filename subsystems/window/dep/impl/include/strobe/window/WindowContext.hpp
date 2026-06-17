#pragma once

#include "./Window.hpp"
#include "strobe/window/allocator.hpp"

namespace strobe::window {

class WindowContext {
 public:
  WindowContext(window::allocator_ref allocator);
  ~WindowContext();

  WindowContext(const WindowContext&) = delete;
  WindowContext& operator=(const WindowContext&) = delete;
  WindowContext(WindowContext&&) = delete;
  WindowContext& operator=(WindowContext&&) = delete;

  Window createWindow(uvec2 size, std::string_view title, bool resizable,
                      bool closeOnCallback);

 private:
  void* m_internals;
};

}  // namespace strobe::window
