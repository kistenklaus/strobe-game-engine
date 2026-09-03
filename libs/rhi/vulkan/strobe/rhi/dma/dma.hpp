#pragma once

#include "strobe/rhi/cmd/cmd.hpp"
#include "strobe/rhi/dma/async_copy_engine.hpp"
#include "strobe/rhi/dma/async_copy_engine_impl.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/stage/staging_pool.hpp"

namespace strobe::rhi::dma {

struct handle_allocators {
  explicit handle_allocators(strobe::rhi::allocator_ref alloc,
                             cmd::handle_allocators *cmdAlloc)
      : alloc(alloc), dmaAlloc(this->alloc), cmdAlloc(cmdAlloc) {}
  strobe::rhi::allocator_ref alloc;
  handle_allocator<AsyncCopyEngineImpl> dmaAlloc;
  cmd::handle_allocators *cmdAlloc;
};

AsyncCopyEngine create_dma(Context context, Timeline timeline,
                           GarbageCollector gc, Queue queue,
                           StagingPool staging,
                           handle_allocators *allocs) noexcept;
} // namespace strobe::rhi::dma
