#include "strobe/rhi/sync/fence_pool.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/sync/fence_callback_flag.hpp"
#include "strobe/rhi/sync/fence_pool_impl.hpp"

namespace strobe::rhi {

void FencePool::pin(void *handle) noexcept {
  pin_void_handle<FencePoolImpl>(handle);
}
void FencePool::unpin(void *handle) noexcept {
  unpin_void_handle<FencePoolImpl>(handle);
}

Fence FencePool::allocate(void *pUserData,
                          void (*callback)(void *,
                                           FenceCallbackFlag)) noexcept {
  auto *impl = void_handle_ptr<FencePoolImpl>(m_handle);
  FenceNode *node = impl->allocate();
  return Fence{make_void_handle<FenceImpl>(impl->get_fence_handle_alloc(),
                                           *this, node, pUserData, callback)};
}

} // namespace strobe::rhi
