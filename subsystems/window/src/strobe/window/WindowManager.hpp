#pragma once

#include <GL/gl.h>
#include <GL/glext.h>

#include <concepts>
#include <string_view>
#include <strobe/core/memory/Mallocator.hpp>
#include <strobe/core/memory/PolyAllocator.hpp>
#include <strobe/lina.hpp>
#include <strobe/memory.hpp>
#include <strobe/window/Window.hpp>
#include <strobe/window/WindowContext.hpp>
#include <strobe/window/allocator.hpp>
#include <type_traits>

#include "./WindowHandle.hpp"

namespace strobe {

class WindowManager {
 public:
  WindowManager(const window::allocator& allocator = {})
      : m_allocator(allocator),
        m_context(window::details::makeAllocatorRef(m_allocator)) {}
  WindowManager(window::allocator&& allocator)
      : m_allocator(std::move(allocator)),
        m_context(window::details::makeAllocatorRef(m_allocator)) {}

  ~WindowManager() = default;

  // Ensure object identity
  WindowManager(const WindowManager&) = delete;
  WindowManager& operator=(const WindowManager&) = delete;
  WindowManager(WindowManager&&) = delete;
  WindowManager& operator=(WindowManager&&) = delete;

  WindowHandle createWindow(uvec2 size, std::string_view title,
                            bool resizable = false,
                            bool closeOnCallback = true);

 private:
  [[no_unique_address]] window::allocator m_allocator;
  window::WindowContext m_context;
};

}  // namespace strobe
