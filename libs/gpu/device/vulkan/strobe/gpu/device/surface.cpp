#include "strobe/gpu/device/surface.hpp"
#include "strobe/gpu/device/handle.hpp"
#include "strobe/gpu/device/surface_impl.hpp"

namespace strobe::gpu {

Surface::Surface(const Surface &other) noexcept
    : m_handle(other.m_handle) {
  if (m_handle) {
    pin_void_handle<SurfaceImpl>(m_handle);
  }
}

Surface::Surface(Surface &&other) noexcept
    : m_handle(other.m_handle) {
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

} // namespace strobe::gpu
