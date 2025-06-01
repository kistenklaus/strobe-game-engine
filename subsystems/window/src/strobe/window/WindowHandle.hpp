#pragma once

#include <strobe/core/memory/smart_pointers/SharedBlock.hpp>
#include <strobe/window/Window.hpp>
#include <strobe/window/allocator.hpp>

namespace strobe {

class WindowManager;
class WindowHandle {
 public:
  friend class WindowManager;

  window::Window& operator*() const noexcept { return *m_window; }
  window::Window* operator->() const noexcept { return m_window.get(); }

  operator bool() const noexcept { return static_cast<bool>(m_window); }
  bool valid() const noexcept { return m_window.valid(); }

  [[deprecated("Only use this for debugging")]] void release() {
    m_window.release();
  }

 private:
  WindowHandle(const window::allocator_ref& alloc, window::Window&& window)
      : m_window(makeSharedBlock<window::Window>(alloc, std::move(window))) {}

  SharedBlock<window::Window, window::allocator_ref> m_window;
};
}  // namespace strobe
