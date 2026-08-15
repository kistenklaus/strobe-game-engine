#pragma once

#include "strobe/gpu/device/command_buffer_state.hpp"
#include "strobe/gpu/device/command_pool.hpp"
#include "strobe/gpu/device/native_command_pool.hpp"
#include "strobe/gpu/vulkan/command_buffer.hpp"

namespace strobe::gpu {

struct CommandBufferImpl {

  CommandBufferImpl(CommandPool pool, NativeCommandPool *nativePool,
                    vulkan::CommandBuffer cmd,
                    const cmd_buf_state_allocator_ref &alloc) noexcept
      : pool(std::move(pool)), nativePool(nativePool), cmd(cmd), state{alloc} {}

  CommandBufferImpl(const CommandBufferImpl &) = delete;
  CommandBufferImpl(CommandBufferImpl &&) = delete;
  CommandBufferImpl &operator=(const CommandBufferImpl &) = delete;
  CommandBufferImpl &operator=(CommandBufferImpl &&) = delete;
  ~CommandBufferImpl() noexcept;

  CommandPool pool;
  NativeCommandPool *nativePool;
  vulkan::CommandBuffer cmd;
  CommandBufferState state;
};

} // namespace strobe::gpu
