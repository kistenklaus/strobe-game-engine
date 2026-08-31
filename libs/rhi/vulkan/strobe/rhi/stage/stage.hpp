#pragma once

#include "strobe/rhi/buf/buf.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/stage/staging_pool.hpp"
#include "strobe/rhi/stage/staging_pool_impl.hpp"

namespace strobe::rhi::stage {

struct handle_allocators {
  explicit handle_allocators(strobe::rhi::allocator_ref alloc,
                             buf::handle_allocators *bufAlloc) noexcept
      : alloc(alloc), bufAlloc(bufAlloc), poolAllocator(alloc) {}
  strobe::rhi::allocator_ref alloc;
  buf::handle_allocators *bufAlloc;
  handle_allocator<StagingPoolImpl> poolAllocator;
};

StagingPool create_staging_pool(MemoryPool memoryPool,
                                handle_allocators *alloc) noexcept;

} // namespace strobe::rhi::stage
