#include "strobe/rhi/sync/binary_semaphore_pool.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/sync/binary_semaphore_pool_impl.hpp"
#include <atomic>

namespace strobe::rhi {

void BinarySemaphorePool::pin(void *handle) noexcept {
  pin_void_handle<BinarySemaphorePoolImpl>(handle);
}

void BinarySemaphorePool::unpin(void *handle) noexcept {
  unpin_void_handle<BinarySemaphorePoolImpl>(handle);
}

BinarySemaphore BinarySemaphorePool::allocate() noexcept {
  auto *impl = void_handle_ptr<BinarySemaphorePoolImpl>(m_handle);
  BinarySemaphoreNode *node = impl->alloc();
  node->pool = m_handle;
  node->refCount.store(1, std::memory_order_relaxed);
  return BinarySemaphore{node};
}

} // namespace strobe::rhi
