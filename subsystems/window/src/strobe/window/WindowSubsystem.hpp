#pragma once

#include <string_view>
#include <strobe/core/lina/vec.hpp>
#include <strobe/memory.hpp>

#include "strobe/window/WindowImpl.hpp"

#include <GL/gl.h>
#include <GL/glext.h>

namespace strobe {

template <strobe::Allocator A>
class WindowSubsystem {
 public:
  WindowSubsystem(uvec2 size, std::string_view title, const A& allocator = {})
      : m_allocator(allocator), m_window(size, title, &m_allocator) {}
  ~WindowSubsystem() = default;
  // Ensure object identity
  WindowSubsystem(const WindowSubsystem&) = delete;
  WindowSubsystem& operator=(const WindowSubsystem&) = delete;
  WindowSubsystem(WindowSubsystem&&) = delete;
  WindowSubsystem& operator=(WindowSubsystem&&) = delete;

  uvec2 getFramebufferSize() const { return m_window.getFramebufferSize(); }
  void setTitle(std::string_view title) { m_window.setTitle(title); }
  std::string_view getTitle() const { return m_window.getTitle(); }
  void setResizable(bool resizable) { return m_window.setResizable(resizable); }
  bool isResizable() const { return m_window.isResizable(); }
  bool shouldClose() const { return m_window.shouldClose(); }

 private:
  [[no_unique_address]] A m_allocator;
  window::WindowImpl m_window;
};

}  // namespace strobe::window
