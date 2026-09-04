#include "strobe/rhi/sync/fence.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/sync/fence_impl.hpp"
#include "strobe/rhi/sync/fence_pool_impl.hpp"
#include "strobe/rhi/vulkan/fence.hpp"

namespace strobe::rhi {

Fence::Fence(const Fence &o) noexcept : Object(o.m_handle) {
  if (m_handle != nullptr) {
    pin_void_handle<FenceImpl>(m_handle);
  }
}
Fence::Fence(Fence &&o) noexcept : Object(std::exchange(o.m_handle, nullptr)) {}
Fence &Fence::operator=(const Fence &o) noexcept {
  if (this == &o) {
    return *this;
  }
  if (o.m_handle != nullptr) {
    pin_void_handle<FenceImpl>(o.m_handle);
  }
  unpin_void_handle<FenceImpl>(m_handle);
  m_handle = o.m_handle;
  return *this;
}
Fence &Fence::operator=(Fence &&o) noexcept {
  if (this == &o) {
    return *this;
  }
  unpin_void_handle<FenceImpl>(m_handle);
  m_handle = std::exchange(o.m_handle, nullptr);
  return *this;
}
Fence::~Fence() noexcept { unpin_void_handle<FenceImpl>(m_handle); }

bool Fence::wait(uint64_t timeout) const noexcept {
  if (m_handle == nullptr) {
    return true;
  }
  auto *impl = void_handle_ptr<FenceImpl>(m_handle);
  if (impl->node == nullptr) {
    return true;
  }
  std::lock_guard lck{impl->node->mutex};
  auto *pool_impl = object_handle_ptr<FencePoolImpl>(impl->pool);
  bool signaled =
      vulkan::wait_for_fence(pool_impl->ctx(), impl->node->fence, timeout);
  if (!signaled) {
    return false;
  }
  if (impl->callback != nullptr) {
    impl->callback(impl->pUserData);
  }
  pool_impl->recycle(impl->node);
  impl->node = nullptr;
  impl->callback = nullptr;
  return signaled;
}

bool Fence::signaled() const noexcept {
  assert(m_handle);
  auto *impl = void_handle_ptr<FenceImpl>(m_handle);
  if (impl->node == nullptr) {
    return true;
  }
  std::lock_guard lck{impl->node->mutex};
  auto *pool_impl = object_handle_ptr<FencePoolImpl>(impl->pool);
  const bool signaled =
      vulkan::is_fence_signaled(pool_impl->ctx(), impl->node->fence);
  if (signaled) {
    if (impl->callback) {
      impl->callback(impl->pUserData);
    }
    pool_impl->recycle(impl->node);
    impl->node = nullptr;
    impl->callback = nullptr;
  }
  return signaled;
}

vulkan::Fence Fence::fence() const noexcept {
  assert(m_handle);
  auto *impl = void_handle_ptr<FenceImpl>(m_handle);
  if (impl->node == nullptr) {
    return {};
  }
  return impl->node->fence;
}

} // namespace strobe::rhi
