#include "strobe/rhi/memory/memory_allocation.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/memory/memory_allocation_impl.hpp"
#include "strobe/rhi/memory/memory_pool.hpp"

namespace strobe::rhi {

MemoryAllocation::MemoryAllocation(const MemoryAllocation &o) noexcept
    : Object(o.m_handle) {
  if (m_handle) {
    pin_void_handle<MemoryAllocationImpl>(m_handle);
  }
}

MemoryAllocation::MemoryAllocation(MemoryAllocation &&o) noexcept
    : Object(std::exchange(o.m_handle, nullptr)) {}

MemoryAllocation &
MemoryAllocation::operator=(const MemoryAllocation &o) noexcept {
  if (this == &o) {
    return *this;
  }
  if (o.m_handle != nullptr) {
    pin_void_handle<MemoryAllocationImpl>(o.m_handle);
  }
  unpin_void_handle<MemoryAllocationImpl>(m_handle);
  m_handle = o.m_handle;
  return *this;
}

MemoryAllocation &MemoryAllocation::operator=(MemoryAllocation &&o) noexcept {
  if (this == &o) {
    return *this;
  }
  unpin_void_handle<MemoryAllocationImpl>(m_handle);
  m_handle = std::exchange(o.m_handle, nullptr);
  return *this;
}

MemoryAllocation::~MemoryAllocation() noexcept {
  unpin_void_handle<MemoryAllocationImpl>(m_handle);
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
