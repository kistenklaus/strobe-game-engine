#include "strobe/rhi/memory/memory_allocation.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/memory/memory_allocation_impl.hpp"
#include "strobe/rhi/memory/memory_pool.hpp"

namespace strobe::rhi {

void MemoryAllocation::pin(void *handle) noexcept {
  pin_void_handle<MemoryAllocationImpl>(handle);
}

void MemoryAllocation::unpin(void *handle) noexcept {
  unpin_void_handle<MemoryAllocationImpl>(handle);
}

void *MemoryAllocation::map() const {
  auto *impl = void_handle_ptr<MemoryAllocationImpl>(m_handle);
  if (!impl->mapped) {
    impl->map();
  }
  assert(impl->mapped);
  return impl->mapped;
}

void MemoryAllocation::flush() const {
  auto *impl = void_handle_ptr<MemoryAllocationImpl>(m_handle);
  impl->flush();
}

void MemoryAllocation::invalidate() const {
  auto *impl = void_handle_ptr<MemoryAllocationImpl>(m_handle);
  impl->invalidate();
}

bool MemoryAllocation::commit() const {
  auto *impl = void_handle_ptr<MemoryAllocationImpl>(m_handle);
  return impl->commit();
}

const MemoryBinding &MemoryAllocation::binding() const {
  auto *impl = void_handle_ptr<MemoryAllocationImpl>(m_handle);
  return impl->binding;
}

} // namespace strobe::rhi
