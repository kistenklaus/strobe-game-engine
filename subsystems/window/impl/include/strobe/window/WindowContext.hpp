#pragma once

#include "./Window.hpp"
#include "strobe/core/memory/PolyAllocator.hpp"

namespace strobe::window {

class WindowContext {
  public:
    WindowContext(PolyAllocatorReference allocator);
    ~WindowContext();

    WindowContext(const WindowContext&) = delete;
    WindowContext& operator=(const WindowContext&) = delete;
    WindowContext(WindowContext&&) = delete;
    WindowContext& operator=(WindowContext&&) = delete;

    Window createWindow(uvec2 size, std::string_view title, bool resizable, bool closeOnCallback);

  private:
    void* m_internals;
};

}
