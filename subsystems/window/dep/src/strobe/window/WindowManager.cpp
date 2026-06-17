#include "./WindowManager.hpp"

namespace strobe {

WindowHandle WindowManager::createWindow(uvec2 size, std::string_view title,
                                         bool resizable, bool closeOnCallback) {
  static_assert(std::constructible_from<window::Window, window::Window&&>);
  static_assert(std::movable<window::Window>);
  static_assert(std::is_destructible_v<window::Window>);
  window::Window window =
      m_context.createWindow(size, title, resizable, closeOnCallback);
  return WindowHandle(window::details::makeAllocatorRef(m_allocator),
                      std::move(window));
}
}  // namespace strobe
