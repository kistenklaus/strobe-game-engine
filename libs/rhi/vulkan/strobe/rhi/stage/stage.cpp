#include "strobe/rhi/stage/stage.hpp"

namespace strobe::rhi::stage {

StagingPool create_staging_pool(MemoryPool memoryPool,
                                handle_allocators *alloc) noexcept {
  return StagingPool{make_void_handle<StagingPoolImpl>(
      &alloc->poolAllocator, std::move(memoryPool), alloc->bufAlloc,
      alloc->alloc)

  };
}
} // namespace strobe::rhi::stage
