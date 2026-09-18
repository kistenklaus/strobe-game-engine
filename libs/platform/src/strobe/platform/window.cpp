#include "strobe/platform/window.hpp"
#include "strobe/platform/window_impl.hpp"
#include <utility>

namespace strobe::platform {

Window::Window(uvec2 size, const char *title) noexcept
    : m_internals(new WindowImpl(size, title)) {}

Window::Window(Window &&o) noexcept
    : m_internals(std::exchange(o.m_internals, nullptr)) {}

Window &Window::operator=(Window &&o) noexcept {
  if (this == &o) {
    return *this;
  }
  if (m_internals) {
    delete static_cast<platform::WindowImpl *>(m_internals);
  }
  m_internals = std::exchange(o.m_internals, nullptr);
  return *this;
}

Window::~Window() noexcept {
  if (m_internals) {
    delete static_cast<platform::WindowImpl *>(m_internals);
  }
}
bool Window::should_close() const noexcept {
  auto *impl = static_cast<const WindowImpl *>(m_internals);
  return impl->should_close();
}
void Window::should_close(bool value) noexcept {
  auto *impl = static_cast<WindowImpl *>(m_internals);
  impl->should_close(value);
}
void Window::title(const char *value) noexcept {
  auto *impl = static_cast<WindowImpl *>(m_internals);
  impl->title(value);
}
ivec2 Window::position() const noexcept {
  auto *impl = static_cast<const WindowImpl *>(m_internals);
  return impl->position();
}
void Window::position(ivec2 value) noexcept {
  auto *impl = static_cast<WindowImpl *>(m_internals);
  impl->position(value);
}
uvec2 Window::size() const noexcept {
  auto *impl = static_cast<const WindowImpl *>(m_internals);
  return impl->size();
}
void Window::size(uvec2 value) noexcept {
  auto *impl = static_cast<WindowImpl *>(m_internals);
  impl->size(value);
}
uvec2 Window::framebuffer_size() const noexcept {
  auto *impl = static_cast<const WindowImpl *>(m_internals);
  return impl->framebuffer_size();
}
vec2 Window::content_scale() const noexcept {
  auto *impl = static_cast<const WindowImpl *>(m_internals);
  return impl->content_scale();
}
bool Window::visible() const noexcept {
  auto *impl = static_cast<const WindowImpl *>(m_internals);
  return impl->visible();
}
void Window::visible(bool value) noexcept {
  auto *impl = static_cast<WindowImpl *>(m_internals);
  impl->visible(value);
}
bool Window::focused() const noexcept {
  auto *impl = static_cast<const WindowImpl *>(m_internals);
  return impl->focused();
}
bool Window::minimized() const noexcept {
  auto *impl = static_cast<const WindowImpl *>(m_internals);
  return impl->minimized();
}
bool Window::maximized() const noexcept {
  auto *impl = static_cast<const WindowImpl *>(m_internals);
  return impl->maximized();
}
bool Window::hovered() const noexcept {
  auto *impl = static_cast<const WindowImpl *>(m_internals);
  return impl->hovered();
}
void Window::minimize() noexcept {
  auto *impl = static_cast<WindowImpl *>(m_internals);
  impl->minimize();
}
void Window::maximize() noexcept {
  auto *impl = static_cast<WindowImpl *>(m_internals);
  impl->maximize();
}
void Window::restore() noexcept {
  auto *impl = static_cast<WindowImpl *>(m_internals);
  impl->restore();
}
void Window::focus() noexcept {
  auto *impl = static_cast<WindowImpl *>(m_internals);
  impl->focus();
}
void Window::request_attention() noexcept {
  auto *impl = static_cast<WindowImpl *>(m_internals);
  impl->request_attention();
}
bool Window::resizable() const noexcept {
  auto *impl = static_cast<const WindowImpl *>(m_internals);
  return impl->resizable();
}
void Window::resizable(bool value) noexcept {
  auto *impl = static_cast<WindowImpl *>(m_internals);
  impl->resizable(value);
}
bool Window::decorated() const noexcept {
  auto *impl = static_cast<const WindowImpl *>(m_internals);
  return impl->decorated();
}
void Window::decorated(bool value) noexcept {
  auto *impl = static_cast<WindowImpl *>(m_internals);
  impl->decorated(value);
}
bool Window::floating() const noexcept {
  auto *impl = static_cast<const WindowImpl *>(m_internals);
  return impl->floating();
}
void Window::floating(bool value) noexcept {
  auto *impl = static_cast<WindowImpl *>(m_internals);
  impl->floating(value);
}
bool Window::auto_iconify() const noexcept {
  auto *impl = static_cast<const WindowImpl *>(m_internals);
  return impl->auto_iconify();
}
void Window::auto_iconify(bool value) noexcept {
  auto *impl = static_cast<WindowImpl *>(m_internals);
  impl->auto_iconify(value);
}
bool Window::focus_on_show() const noexcept {
  auto *impl = static_cast<const WindowImpl *>(m_internals);
  return impl->focus_on_show();
}
void Window::focus_on_show(bool value) noexcept {
  auto *impl = static_cast<WindowImpl *>(m_internals);
  impl->focus_on_show(value);
}
bool Window::mouse_passthrough() const noexcept {
  auto *impl = static_cast<const WindowImpl *>(m_internals);
  return impl->mouse_passthrough();
}
void Window::mouse_passthrough(bool value) noexcept {
  auto *impl = static_cast<WindowImpl *>(m_internals);
  impl->mouse_passthrough(value);
}
float Window::opacity() const noexcept {
  auto *impl = static_cast<const WindowImpl *>(m_internals);
  return impl->opacity();
}
void Window::opacity(float value) noexcept {
  auto *impl = static_cast<WindowImpl *>(m_internals);
  impl->opacity(value);
}
bool Window::transparent_framebuffer() const noexcept {
  auto *impl = static_cast<const WindowImpl *>(m_internals);
  return impl->transparent_framebuffer();
}
EventListenerHandle Window::add_window_position_listener(
    const EventListenerRef<WindowPositionEvent> &listener, std::uint8_t layer) {
  auto *impl = static_cast<WindowImpl *>(m_internals);
  return impl->add_window_position_listener(listener, layer);
}
EventListenerHandle Window::add_window_size_listener(
    const EventListenerRef<WindowSizeEvent> &listener, std::uint8_t layer) {
  auto *impl = static_cast<WindowImpl *>(m_internals);
  return impl->add_window_size_listener(listener, layer);
}
EventListenerHandle Window::add_window_close_listener(
    const EventListenerRef<WindowCloseEvent> &listener, std::uint8_t layer) {
  auto *impl = static_cast<WindowImpl *>(m_internals);
  return impl->add_window_close_listener(listener, layer);
}
EventListenerHandle Window::add_window_refresh_listener(
    const EventListenerRef<WindowRefreshEvent> &listener, std::uint8_t layer) {
  auto *impl = static_cast<WindowImpl *>(m_internals);
  return impl->add_window_refresh_listener(listener, layer);
}
EventListenerHandle Window::add_window_focus_listener(
    const EventListenerRef<WindowFocusEvent> &listener, std::uint8_t layer) {
  auto *impl = static_cast<WindowImpl *>(m_internals);
  return impl->add_window_focus_listener(listener, layer);
}
EventListenerHandle Window::add_window_iconify_listener(
    const EventListenerRef<WindowIconifyEvent> &listener, std::uint8_t layer) {
  auto *impl = static_cast<WindowImpl *>(m_internals);
  return impl->add_window_iconify_listener(listener, layer);
}
EventListenerHandle Window::add_window_maximize_listener(
    const EventListenerRef<WindowMaximizeEvent> &listener, std::uint8_t layer) {
  auto *impl = static_cast<WindowImpl *>(m_internals);
  return impl->add_window_maximize_listener(listener, layer);
}
EventListenerHandle Window::add_framebuffer_size_listener(
    const EventListenerRef<FramebufferSizeEvent> &listener,
    std::uint8_t layer) {
  auto *impl = static_cast<WindowImpl *>(m_internals);
  return impl->add_framebuffer_size_listener(listener, layer);
}
EventListenerHandle Window::add_window_content_scale_listener(
    const EventListenerRef<WindowContentScaleEvent> &listener,
    std::uint8_t layer) {
  auto *impl = static_cast<WindowImpl *>(m_internals);
  return impl->add_window_content_scale_listener(listener, layer);
}
EventListenerHandle
Window::add_key_listener(const EventListenerRef<KeyEvent> &listener,
                         std::uint8_t layer) {
  auto *impl = static_cast<WindowImpl *>(m_internals);
  return impl->add_key_listener(listener, layer);
}
EventListenerHandle
Window::add_character_listener(const EventListenerRef<CharacterEvent> &listener,
                               std::uint8_t layer) {
  auto *impl = static_cast<WindowImpl *>(m_internals);
  return impl->add_character_listener(listener, layer);
}
EventListenerHandle Window::add_character_mods_listener(
    const EventListenerRef<CharacterModsEvent> &listener, std::uint8_t layer) {
  auto *impl = static_cast<WindowImpl *>(m_internals);
  return impl->add_character_mods_listener(listener, layer);
}
EventListenerHandle Window::add_cursor_position_listener(
    const EventListenerRef<CursorPositionEvent> &listener, std::uint8_t layer) {
  auto *impl = static_cast<WindowImpl *>(m_internals);
  return impl->add_cursor_position_listener(listener, layer);
}
EventListenerHandle Window::add_cursor_enter_listener(
    const EventListenerRef<CursorEnterEvent> &listener, std::uint8_t layer) {
  auto *impl = static_cast<WindowImpl *>(m_internals);
  return impl->add_cursor_enter_listener(listener, layer);
}
EventListenerHandle Window::add_mouse_button_listener(
    const EventListenerRef<MouseButtonEvent> &listener, std::uint8_t layer) {
  auto *impl = static_cast<WindowImpl *>(m_internals);
  return impl->add_mouse_button_listener(listener, layer);
}
EventListenerHandle
Window::add_scroll_listener(const EventListenerRef<ScrollEvent> &listener,
                            std::uint8_t layer) {
  auto *impl = static_cast<WindowImpl *>(m_internals);
  return impl->add_scroll_listener(listener, layer);
}
EventListenerHandle
Window::add_drop_listener(const EventListenerRef<DropEvent> &listener,
                          std::uint8_t layer) {
  auto *impl = static_cast<WindowImpl *>(m_internals);
  return impl->add_drop_listener(listener, layer);
}

GLFWwindow *Window::window_ptr() const noexcept {
  auto *impl = static_cast<WindowImpl *>(m_internals);
  return impl->m_window;
}

} // namespace strobe::platform
