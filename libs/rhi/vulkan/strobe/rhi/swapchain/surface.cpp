#include "strobe/rhi/swapchain/surface.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/swapchain/surface_impl.hpp"

namespace strobe::rhi {

Surface::Surface(const Surface &other) noexcept : Object(other.m_handle) {
  if (m_handle) {
    pin_void_handle<SurfaceImpl>(m_handle);
  }
}

Surface::Surface(Surface &&other) noexcept : Object(other.m_handle) {
  other.m_handle = nullptr;
}

Surface &Surface::operator=(const Surface &other) noexcept {
  if (this == &other) {
    return *this;
  }
  if (other.m_handle) {
    pin_void_handle<SurfaceImpl>(other.m_handle);
  }
  if (m_handle) {
    unpin_void_handle<SurfaceImpl>(m_handle);
  }
  m_handle = other.m_handle;
  return *this;
}

Surface &Surface::operator=(Surface &&other) noexcept {
  if (this == &other) {
    return *this;
  }
  unpin_void_handle<SurfaceImpl>(m_handle);
  m_handle = other.m_handle;
  other.m_handle = nullptr;
  return *this;
}

Surface::~Surface() noexcept {
  if (m_handle) {
    unpin_void_handle<SurfaceImpl>(m_handle);
  }
}

vulkan::Surface Surface::get() const noexcept {
  auto *impl = void_handle_ptr<SurfaceImpl>(m_handle);
  return impl->surface;
}

vulkan::Context *Surface::ctx() const noexcept {
  auto *impl = void_handle_ptr<SurfaceImpl>(m_handle);
  return impl->context.ctx();
}

const Context &Surface::context() const noexcept {
  auto *impl = void_handle_ptr<SurfaceImpl>(m_handle);
  return impl->context;
}

} // namespace strobe::rhi
