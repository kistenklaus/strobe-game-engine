#include "strobe/rhi/swapchain/surface.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/swapchain/surface_impl.hpp"

namespace strobe::rhi {

void Surface::pin(void *handle) noexcept {
  pin_void_handle<SurfaceImpl>(handle);
}

void Surface::unpin(void *handle) noexcept {
  unpin_void_handle<SurfaceImpl>(handle);
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
