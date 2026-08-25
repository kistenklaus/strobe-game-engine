#include "strobe/rhi/memory/memory_allocation_impl.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/memory/memory_pool_impl.hpp"
#include "strobe/rhi/vulkan/memory.hpp"

namespace strobe::rhi {

MemoryAllocationImpl::~MemoryAllocationImpl() noexcept {
  auto *pool_impl = void_handle_ptr<MemoryPoolImpl>(pool.m_handle);
  if (mapped) {
    vulkan::unmap_memory(pool_impl->context.ctx(), binding.memory);
  }
  pool_impl->free_memory(binding, internals);
}

void *MemoryAllocationImpl::map() {
  vulkan::Context *ctx =
      void_handle_ptr<MemoryPoolImpl>(pool.m_handle)->context.ctx();
  assert(binding);
  mapped = static_cast<std::byte *>(vulkan::map_memory(ctx, binding.memory)) +
           binding.offset;
  return mapped;
}
void MemoryAllocationImpl::flush() {
  vulkan::Context *ctx =
      void_handle_ptr<MemoryPoolImpl>(pool.m_handle)->context.ctx();
  assert(binding);
  vulkan::flush_memory(ctx, binding.memory, binding.offset);
}

void MemoryAllocationImpl::invalidate() {
  vulkan::Context *ctx =
      void_handle_ptr<MemoryPoolImpl>(pool.m_handle)->context.ctx();
  assert(binding);
  vulkan::invalidate_memory(ctx, binding.memory, binding.offset);
}

} // namespace strobe::rhi
