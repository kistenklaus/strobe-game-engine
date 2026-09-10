#include "strobe/rhi/memory/memory_allocation_impl.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/memory/memory_pool_impl.hpp"
#include "strobe/rhi/vulkan/memory.hpp"

namespace strobe::rhi {

MemoryAllocationImpl::~MemoryAllocationImpl() noexcept {
  auto *pool_impl = object_handle_ptr<MemoryPoolImpl>(pool);
  if (mapped) {
    vulkan::unmap_memory(pool_impl->context.ctx(), binding.memory);
  }
  pool_impl->free_memory(binding, internals);
}

void *MemoryAllocationImpl::map() {
  vulkan::Context *ctx =
      object_handle_ptr<MemoryPoolImpl>(pool)->context.ctx();
  assert(binding);
  mapped = static_cast<std::byte *>(vulkan::map_memory(ctx, binding.memory)) +
           binding.offset;
  return mapped;
}
void MemoryAllocationImpl::flush() {
  vulkan::Context *ctx =
      object_handle_ptr<MemoryPoolImpl>(pool)->context.ctx();
  assert(binding);
  vulkan::flush_memory(ctx, binding.memory, binding.offset);
}

void MemoryAllocationImpl::invalidate() {
  vulkan::Context *ctx =
      object_handle_ptr<MemoryPoolImpl>(pool)->context.ctx();
  assert(binding);
  vulkan::invalidate_memory(ctx, binding.memory, binding.offset);
}

bool MemoryAllocationImpl::commit() {
  if (!binding) {
    auto *pool_impl = object_handle_ptr<MemoryPoolImpl>(pool);
    binding = pool_impl->commit_memory(internals);
    assert(binding);
    return true;
  }
  return false;
}

} // namespace strobe::rhi
