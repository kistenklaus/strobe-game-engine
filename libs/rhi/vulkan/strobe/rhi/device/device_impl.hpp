#pragma once
#include "strobe/rhi/bvh/scratch_buffer.hpp"
#include "strobe/rhi/context/context.hpp"
#include "strobe/rhi/gc/garbage_collector.hpp"
#include "strobe/rhi/memory/memory_pool.hpp"
#include "strobe/rhi/queue/queue.hpp"
#include "strobe/rhi/stage/staging_pool.hpp"
#include "strobe/rhi/sync/binary_semaphore_pool.hpp"
#include "strobe/rhi/sync/fence_pool.hpp"

namespace strobe::rhi {

struct DeviceImpl {
  explicit DeviceImpl(Context context, FencePool fencePool,
                      BinarySemaphorePool semPool, MemoryPool memory,
                      StagingPool staging, ScratchBuffer scratch,
                      GarbageCollector gc, Queue universalQueue,
                      Queue dmaQueue) noexcept
      : context(std::move(context)), fencePool(std::move(fencePool)),
        semPool(std::move(semPool)), memory(std::move(memory)),
        staging(std::move(staging)), scratch(std::move(scratch)),
        universalQueue(std::move(universalQueue)),
        dmaQueue(std::move(dmaQueue)), gc(std::move(gc)) {}

  Context context;
  FencePool fencePool;
  BinarySemaphorePool semPool;
  MemoryPool memory;
  StagingPool staging;
  ScratchBuffer scratch;

  Queue universalQueue;
  Queue dmaQueue;
  GarbageCollector gc;
};

} // namespace strobe::rhi
