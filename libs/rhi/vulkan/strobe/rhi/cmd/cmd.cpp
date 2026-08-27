#include "strobe/rhi/cmd/cmd.hpp"

strobe::rhi::CommandPool
strobe::rhi::cmd::create_cmd_pool(Context context, StagingPool stagingPool,
                                  uint32_t queueFamily,
                                  allocators *alloc) noexcept {

  return CommandPool{make_void_handle<CommandPoolImpl>(
      &alloc->poolAllocator, std::move(context), std::move(stagingPool),
      queueFamily, &alloc->stateAllocator, &alloc->cmdAllocator, alloc->alloc)};
}
