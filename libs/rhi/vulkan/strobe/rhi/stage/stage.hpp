#pragma once

#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/stage/staging_pool.hpp"
#include "strobe/rhi/stage/staging_pool_impl.hpp"

namespace strobe::rhi::stage {

struct handle_allocators {
  handle_allocator<StagingPoolImpl> poolAllocator;
  strobe::rhi::allocator_ref alloc;
};

StagingPool create_staging_pool(MemoryPool memoryPool,
                                buf::handle_allocators *bufAllocators,
                                handle_allocators *alloc) noexcept;

} // namespace strobe::rhi::stage
