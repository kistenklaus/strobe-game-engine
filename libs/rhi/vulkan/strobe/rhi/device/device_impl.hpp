#pragma once
#include "strobe/rhi/bvh/scratch_buffer.hpp"
#include "strobe/rhi/context/context.hpp"
#include "strobe/rhi/dma/DMA.hpp"
#include "strobe/rhi/gc/garbage_collector.hpp"
#include "strobe/rhi/memory/memory_pool.hpp"
#include "strobe/rhi/objects/queue.hpp"
#include "strobe/rhi/stage/staging_pool.hpp"
#include "strobe/rhi/sync/binary_semaphore_pool.hpp"
#include "strobe/rhi/sync/fence_pool.hpp"

namespace strobe::rhi {

struct handle_allocators;

struct DeviceImpl {
  explicit DeviceImpl(Context context, handle_allocators *allocs,
                      FencePool fencePool, BinarySemaphorePool semPool,
                      MemoryPool memory, StagingPool staging,
                      ScratchBuffer scratch, GarbageCollector gc,
                      Queue universalQueue, Queue dmaQueue, DMA dma) noexcept
      : context(std::move(context)), allocs(allocs),
        fencePool(std::move(fencePool)), semPool(std::move(semPool)),
        memory(std::move(memory)), staging(std::move(staging)),
        scratch(std::move(scratch)), gc(std::move(gc)),
        universalQueue(std::move(universalQueue)),
        dmaQueue(std::move(dmaQueue)), dma(std::move(dma)) {}

  Context context;
  handle_allocators *allocs;
  FencePool fencePool;
  BinarySemaphorePool semPool;
  MemoryPool memory;
  StagingPool staging;
  ScratchBuffer scratch;
  GarbageCollector gc;
  Queue universalQueue;
  Queue dmaQueue;
  DMA dma;
};

} // namespace strobe::rhi
