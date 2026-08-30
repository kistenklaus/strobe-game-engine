#include "strobe/rhi/sync/binary_semaphore_pool.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/sync/binary_semaphore_pool_impl.hpp"
#include <atomic>
#include <utility>

namespace strobe::rhi {

BinarySemaphorePool::BinarySemaphorePool(const BinarySemaphorePool &o) noexcept
    : Object(o.m_handle) {
  if (m_handle != nullptr) {
    pin_void_handle<BinarySemaphorePoolImpl>(m_handle);
  }
}
BinarySemaphorePool::BinarySemaphorePool(BinarySemaphorePool &&o) noexcept
    : Object(std::exchange(o.m_handle, nullptr)) {}

BinarySemaphorePool &
BinarySemaphorePool::operator=(const BinarySemaphorePool &o) noexcept {
  if (this == &o) {
    return *this;
  }
  if (o.m_handle != nullptr) {
    pin_void_handle<BinarySemaphorePoolImpl>(o.m_handle);
  }
  unpin_void_handle<BinarySemaphorePoolImpl>(m_handle);
  m_handle = o.m_handle;
  return *this;
}

BinarySemaphorePool &
BinarySemaphorePool::operator=(BinarySemaphorePool &&o) noexcept {
  if (this == &o) {
    return *this;
  }
  unpin_void_handle<BinarySemaphorePoolImpl>(m_handle);
  m_handle = std::exchange(o.m_handle, nullptr);
  return *this;
}

BinarySemaphorePool::~BinarySemaphorePool() noexcept {
  unpin_void_handle<BinarySemaphorePoolImpl>(m_handle);
}

BinarySemaphore BinarySemaphorePool::allocate() noexcept {
  auto *impl = void_handle_ptr<BinarySemaphorePoolImpl>(m_handle);
  BinarySemaphoreNode *node = impl->alloc();
  node->pool = this;
  node->refCount.store(1, std::memory_order_relaxed);
  return BinarySemaphore{node};
}

} // namespace strobe::rhi
