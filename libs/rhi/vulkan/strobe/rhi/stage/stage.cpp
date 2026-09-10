#include "strobe/rhi/stage/stage.hpp"
#include "strobe/rhi/object_factory.hpp"

namespace strobe::rhi::stage {

StagingPool create_staging_pool(MemoryPool memoryPool,
                                handle_allocators *alloc) noexcept {
  ZoneScopedN("stage/create-stage");
  return detail::make_object<StagingPool>(make_void_handle<StagingPoolImpl>(
      &alloc->poolAllocator, std::move(memoryPool), alloc->bufAlloc,
      alloc->alloc));
}
} // namespace strobe::rhi::stage
