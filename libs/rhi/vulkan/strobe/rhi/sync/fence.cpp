#include "strobe/rhi/sync/fence.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/sync/fence_callback_flag.hpp"
#include "strobe/rhi/sync/fence_impl.hpp"
#include "strobe/rhi/sync/fence_pool_impl.hpp"
#include "strobe/rhi/vulkan/fence.hpp"

namespace strobe::rhi {

void Fence::pin(void *handle) noexcept { pin_void_handle<FenceImpl>(handle); }

void Fence::unpin(void *handle) noexcept {
  unpin_void_handle<FenceImpl>(handle);
}

bool Fence::wait(uint64_t timeout) const noexcept {
  if (m_handle == nullptr) {
    return true;
  }
  auto *impl = void_handle_ptr<FenceImpl>(m_handle);
  std::lock_guard lck{impl->mutex};
  if (impl->node == nullptr) {
    return true;
  }
  auto *pool_impl = object_handle_ptr<FencePoolImpl>(impl->pool);
  bool signaled =
      vulkan::wait_for_fence(pool_impl->ctx(), impl->node->fence, timeout);
  if (!signaled) {
    return false;
  }
  if (impl->callback != nullptr) {
    impl->callback(impl->pUserData, FenceCallbackFlag::signaled);
  }
  pool_impl->recycle(impl->node);
  impl->node = nullptr;
  impl->callback = nullptr;
  return signaled;
}

bool Fence::signaled() const noexcept {
  assert(m_handle);
  auto *impl = void_handle_ptr<FenceImpl>(m_handle);
  std::lock_guard lck{impl->mutex};
  if (impl->node == nullptr) {
    return true;
  }
  auto *pool_impl = object_handle_ptr<FencePoolImpl>(impl->pool);
  const bool signaled =
      vulkan::is_fence_signaled(pool_impl->ctx(), impl->node->fence);
  if (signaled) {
    if (impl->callback) {
      impl->callback(impl->pUserData, FenceCallbackFlag::signaled);
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
  std::lock_guard lck{impl->mutex};
  if (impl->node == nullptr) {
    return {};
  }
  return impl->node->fence;
}

} // namespace strobe::rhi
