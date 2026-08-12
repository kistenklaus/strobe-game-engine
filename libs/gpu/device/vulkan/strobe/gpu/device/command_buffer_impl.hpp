#pragma once

#include "strobe/gpu/device/command_pool.hpp"
#include "strobe/gpu/device/command_pool_impl.hpp"
#include "strobe/gpu/vulkan/command_buffer.hpp"
namespace strobe::gpu {

struct CommandBufferImpl {

  CommandBufferImpl(CommandPool pool, vulkan::CommandBuffer cmd, bool primary) noexcept
      : pool(std::move(pool)), cmd(cmd), primary(primary) {}

  CommandBufferImpl(const CommandBufferImpl &) = delete;
  CommandBufferImpl(CommandBufferImpl &&) = delete;
  CommandBufferImpl &operator=(const CommandBufferImpl &) = delete;
  CommandBufferImpl &operator=(CommandBufferImpl &&) = delete;
  ~CommandBufferImpl() noexcept {
    assert(cmd);
    assert(pool);
    auto* pool_impl = void_handle_ptr<CommandPoolImpl>(pool.m_handle);
    pool_impl->recycle(cmd, primary);
  }

  const CommandPool pool; // holds reference
  const vulkan::CommandBuffer cmd;
  const bool primary;
};

} // namespace strobe::gpu
