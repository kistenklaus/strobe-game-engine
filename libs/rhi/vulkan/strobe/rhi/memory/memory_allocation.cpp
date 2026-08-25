#include "strobe/rhi/memory/memory_allocation.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/memory/memory_allocation_handle_alloc.hpp"
#include "strobe/rhi/memory/memory_allocation_impl.hpp"
#include "strobe/rhi/memory/memory_pool_impl.hpp"
#include "strobe/rhi/objects/memory_pool.hpp"
#include "strobe/rhi/utils/always_inline.hpp"
#include "strobe/rhi/vulkan/buffer.hpp"
#include "strobe/rhi/vulkan/memory.hpp"
#include <atomic>

namespace strobe::rhi {

namespace {

using handle_alloc = memory_allocation_handle_allocator_ref;

void unpin_memory_allocation(void *h) noexcept {
  if (h == nullptr) {
    return;
  }
  using control_block =
      handle_control_block<MemoryAllocationImpl, handle_alloc>;
  using allocator_traits = AllocatorTraits<handle_alloc>;

  auto *block = static_cast<control_block *>(h);

  if (block->refCount.fetch_sub(1, std::memory_order_acq_rel) != 1) {
    return;
  }
  MemoryPool keepAlive = block->value.pool;
  handle_alloc alloc = std::move(block->alloc);
  std::destroy_at(block);
  // alloc, still has to be valid after destroy!, but destroy might destroy the
  // underlying pool!
  allocator_traits::template deallocate<control_block>(alloc, block);
}

} // namespace

MemoryAllocation::MemoryAllocation(const MemoryAllocation &o) noexcept
    : m_handle(o.m_handle) {
  if (m_handle) {
    pin_void_handle<MemoryAllocationImpl, handle_alloc>(m_handle);
  }
}

MemoryAllocation::MemoryAllocation(MemoryAllocation &&o) noexcept
    : m_handle(std::exchange(o.m_handle, nullptr)) {}

MemoryAllocation &
MemoryAllocation::operator=(const MemoryAllocation &o) noexcept {
  if (this == &o) {
    return *this;
  }
  if (o.m_handle != nullptr) {
    pin_void_handle<MemoryAllocationImpl, handle_alloc>(o.m_handle);
  }
  unpin_memory_allocation(m_handle);
  // unpin_void_handle<MemoryAllocationImpl>(m_handle);
  m_handle = o.m_handle;
  return *this;
}

MemoryAllocation &MemoryAllocation::operator=(MemoryAllocation &&o) noexcept {
  if (this == &o) {
    return *this;
  }
  unpin_memory_allocation(m_handle);
  // unpin_void_handle<MemoryAllocationImpl>(m_handle);
  m_handle = std::exchange(o.m_handle, nullptr);
  return *this;
}

MemoryAllocation::~MemoryAllocation() noexcept {
  unpin_memory_allocation(m_handle);
  // unpin_void_handle<MemoryAllocationImpl>(m_handle);
}

void *MemoryAllocation::map() const {
  auto *impl = void_handle_ptr<MemoryAllocationImpl, handle_alloc>(m_handle);
  if (!impl->mapped) {
    impl->map();
  }
  assert(impl->mapped);
  return impl->mapped;
}

void MemoryAllocation::flush() const {
  auto *impl = void_handle_ptr<MemoryAllocationImpl, handle_alloc>(m_handle);
  impl->flush();
}

void MemoryAllocation::invalidate() const {
  auto *impl = void_handle_ptr<MemoryAllocationImpl, handle_alloc>(m_handle);
  impl->invalidate();
}

} // namespace strobe::rhi
