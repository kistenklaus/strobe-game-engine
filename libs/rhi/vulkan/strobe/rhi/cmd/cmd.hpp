#pragma once

#include "strobe/rhi/allocator.hpp"
#include "strobe/rhi/cmd/command_buffer_state_alloctor.hpp"
#include "strobe/rhi/cmd/command_pool_impl.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/objects/command_buffer.hpp"
#include "strobe/rhi/objects/command_pool.hpp"

namespace strobe::rhi::cmd {

struct handle_allocators {
  explicit handle_allocators(strobe::rhi::allocator_ref alloc) noexcept
      : alloc(alloc), stateAllocator(alloc), poolAllocator(alloc) {}
  strobe::rhi::allocator_ref alloc;
  cmd_buf_state_allocator stateAllocator;
  handle_allocator<CommandPoolImpl> poolAllocator;
};

CommandPool create_cmd_pool(Context context, StagingPool stagingPool,
                            uint32_t queueFamily,
                            handle_allocators *alloc) noexcept;

} // namespace strobe::rhi::cmd
