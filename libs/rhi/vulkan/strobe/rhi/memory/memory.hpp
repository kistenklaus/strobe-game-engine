#pragma once

#include "strobe/rhi/context/context.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/memory/memory_pool.hpp"
#include "strobe/rhi/memory/memory_pool_impl.hpp"

namespace strobe::rhi::mem {

struct handle_allocators {
  explicit handle_allocators(strobe::rhi::allocator_ref alloc) noexcept
      : alloc(alloc), poolAlloc(alloc) {}

  strobe::rhi::allocator_ref alloc;
  handle_allocator<MemoryPoolImpl> poolAlloc;
};

MemoryPool create_mem_pool(Context context, mem::handle_allocators *alloc);

} // namespace strobe::rhi::mem
