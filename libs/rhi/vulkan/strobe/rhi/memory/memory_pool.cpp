#include "strobe/rhi/memory/memory_pool.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/memory/memory_allocation_impl.hpp"
#include "strobe/rhi/memory/memory_pool_impl.hpp"
#include <tracy/Tracy.hpp>
#include <utility>
#include <vulkan/vulkan_core.h>

namespace strobe::rhi {

MemoryPool::MemoryPool(const MemoryPool &o) noexcept : m_handle(o.m_handle) {
  if (m_handle) {
    pin_void_handle<MemoryPoolImpl>(m_handle);
  }
}

MemoryPool::MemoryPool(MemoryPool &&o) noexcept
    : m_handle(std::exchange(o.m_handle, nullptr)) {}

MemoryPool &MemoryPool::operator=(const MemoryPool &o) noexcept {
  if (this == &o) {
    return *this;
  }
  if (o.m_handle != nullptr) {
    pin_void_handle<MemoryPoolImpl>(o.m_handle);
  }
  unpin_void_handle<MemoryPoolImpl>(m_handle);
  m_handle = o.m_handle;
  return *this;
}

MemoryPool &MemoryPool::operator=(MemoryPool &&o) noexcept {
  if (this == &o) {
    return *this;
  }
  unpin_void_handle<MemoryPoolImpl>(m_handle);
  m_handle = std::exchange(o.m_handle, nullptr);
  return *this;
}

MemoryPool::~MemoryPool() noexcept {
  unpin_void_handle<MemoryPoolImpl>(m_handle);
}

void MemoryPool::commit() {
  ZoneScopedN("MemoryPool::commit");
  auto *impl = void_handle_ptr<MemoryPoolImpl>(m_handle);
  impl->commit_all();
}

bool MemoryPool::memory_overlaps(const MemoryAllocation &lhs,
                                 const MemoryAllocation &rhs) const noexcept {
  lhs.commit();
  rhs.commit();
  auto *lhs_impl = void_handle_ptr<MemoryAllocationImpl>(lhs.m_handle);
  auto *rhs_impl = void_handle_ptr<MemoryAllocationImpl>(rhs.m_handle);
  assert(lhs_impl->binding);
  assert(rhs_impl->binding);
  const VkDeviceSize lhs_offset = lhs_impl->binding.offset;
  const VkDeviceSize lhs_size = lhs_impl->binding.size;
  const VkDeviceSize rhs_offset = rhs_impl->binding.offset;
  const VkDeviceSize rhs_size = rhs_impl->binding.size;
  const bool overlap =
      lhs_offset < rhs_offset + rhs_size && rhs_offset < lhs_offset + lhs_size;
  return overlap;
}

MemoryAllocation
MemoryPool::allocate_memory(const MemoryRequirements &requirements,
                            const MemoryLifetime &lifetime) const {
  auto *impl = void_handle_ptr<MemoryPoolImpl>(m_handle);
  const auto [binding, internals] =
      impl->allocate_memory(requirements, lifetime);
  return MemoryAllocation{make_void_handle<MemoryAllocationImpl>(
      impl->get_handle_alloc(), *this, binding, requirements.memoryUsage,
      internals)};
}

const Context &MemoryPool::context() const noexcept {
  auto *impl = void_handle_ptr<MemoryPoolImpl>(m_handle);
  return impl->context;
}

} // namespace strobe::rhi
