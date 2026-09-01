#pragma once

#include "strobe/rhi/cmd/cmd.hpp"
#include "strobe/rhi/dma/DMA.hpp"
#include "strobe/rhi/dma/DMAImpl.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/stage/staging_pool.hpp"

namespace strobe::rhi::dma {

struct handle_allocators {
  explicit handle_allocators(strobe::rhi::allocator_ref alloc,
                             cmd::handle_allocators *cmdAlloc)
      : dmaAlloc(alloc), cmdAlloc(cmdAlloc) {}
  handle_allocator<DMAImpl> dmaAlloc;
  cmd::handle_allocators *cmdAlloc;
};

DMA create_dma(Context context, Timeline timeline, GarbageCollector gc,
               Queue queue, StagingPool staging,
               handle_allocators *allocs) noexcept;
} // namespace strobe::rhi::dma
