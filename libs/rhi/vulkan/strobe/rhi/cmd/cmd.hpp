#pragma once

#include "strobe/rhi/allocator.hpp"
#include "strobe/rhi/cmd/command_buffer_impl.hpp"
#include "strobe/rhi/cmd/command_buffer_state_alloctor.hpp"
#include "strobe/rhi/cmd/command_pool_impl.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/objects/command_buffer.hpp"
#include "strobe/rhi/objects/command_pool.hpp"

namespace strobe::rhi::cmd {

struct allocators {
  strobe::rhi::allocator_ref alloc;
  cmd_buf_state_allocator stateAllocator;
  handle_allocator<CommandBufferImpl> cmdAllocator;
  handle_allocator<CommandPoolImpl> poolAllocator;
};

CommandPool create_cmd_pool(Context context, StagingPool stagingPool,
                            uint32_t queueFamily, allocators *alloc) noexcept;

} // namespace strobe::rhi::cmd
