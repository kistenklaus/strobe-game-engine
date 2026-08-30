#include "strobe/rhi/sync/fence_pool.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/sync/fence_pool_impl.hpp"

namespace strobe::rhi {

FencePool::FencePool(const FencePool &o) noexcept : Object(o.m_handle) {
  if (m_handle != nullptr) {
    pin_void_handle<FencePoolImpl>(m_handle);
  }
}

FencePool::FencePool(FencePool &&o) noexcept
    : Object(std::exchange(o.m_handle, nullptr)) {}

FencePool &FencePool::operator=(const FencePool &o) noexcept {
  if (this == &o) {
    return *this;
  }
  if (o.m_handle != nullptr) {
    pin_void_handle<FencePoolImpl>(o.m_handle);
  }
  unpin_void_handle<FencePoolImpl>(m_handle);
  m_handle = o.m_handle;
  return *this;
}

FencePool &FencePool::operator=(FencePool &&o) noexcept {
  if (this == &o) {
    return *this;
  }
  unpin_void_handle<FencePoolImpl>(m_handle);
  m_handle = std::exchange(o.m_handle, nullptr);
  return *this;
}

FencePool::~FencePool() noexcept { unpin_void_handle<FencePoolImpl>(m_handle); }

Fence FencePool::allocate() noexcept {
  auto *impl = void_handle_ptr<FencePoolImpl>(m_handle);
  FenceNode *node = impl->allocate();
  return Fence{
      make_void_handle<FenceImpl>(impl->get_fence_handle_alloc(), *this, node)};
}

} // namespace strobe::rhi
