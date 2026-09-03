#include "strobe/rhi/dma/dma.hpp"
#include "strobe/rhi/cmd/cmd.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/objects/command_pool.hpp"

namespace strobe::rhi {

AsyncCopyEngine dma::create_dma(Context context, Timeline timeline,
                                GarbageCollector gc, Queue queue,
                                StagingPool staging,
                                handle_allocators *allocs) noexcept {
  CommandPool cmdpool = cmd::create_cmd_pool(
      context, staging, object_handle_ptr<QueueImpl>(queue)->family(),
      allocs->cmdAlloc);

  return AsyncCopyEngine{make_void_handle<AsyncCopyEngineImpl>(
      &allocs->dmaAlloc, std::move(timeline), std::move(gc), std::move(queue),
      std::move(cmdpool), allocs->alloc)};
}

} // namespace strobe::rhi
