#pragma once

#include "strobe/core/containers/lockfree_mpsc_stack.hpp"
#include "strobe/gpu/device/allocator.hpp"
#include "strobe/gpu/device/command_buffer.hpp"
#include "strobe/gpu/device/device_impl.hpp"
#include "strobe/gpu/device/handle.hpp"
#include "strobe/gpu/vulkan/command_buffer.hpp"
#include "strobe/gpu/vulkan/command_pool.hpp"
#include <vulkan/vulkan_core.h>
namespace strobe::gpu {

struct CommandPoolImpl {

  CommandPoolImpl(Device device, vulkan::CommandPool pool)
      : device(std::move(device)), pool(pool) {}
  CommandPoolImpl(const CommandPoolImpl &) = delete;
  CommandPoolImpl(CommandPoolImpl &&) = delete;
  CommandPoolImpl &operator=(const CommandPoolImpl &) = delete;
  CommandPoolImpl &operator=(CommandPoolImpl &&) = delete;

  ~CommandPoolImpl() noexcept {
    auto *device_impl = void_handle_ptr<DeviceImpl>(device.m_handle);
    vulkan::destroy_command_pool(&device_impl->context, pool);
  }

  void recycle(vulkan::CommandBuffer cmd, bool primary) {
    if (primary) {
      primaryFreelist.push(cmd);
    } else {
      secondaryFreelist.push(cmd);
    }
  }

  vulkan::CommandBuffer alloc_cached(bool primary) {
    std::optional<vulkan::CommandBuffer> cmd;
    if (primary) {
      cmd = primaryFreelist.pop();
    } else {
      cmd = secondaryFreelist.pop();
    }
    return cmd.value_or({});
  }

  const Device device;
  const vulkan::CommandPool pool;

  LockFreeMPSCStack<vulkan::CommandBuffer, strobe::gpu::allocator>
      primaryFreelist{};
  LockFreeMPSCStack<vulkan::CommandBuffer, strobe::gpu::allocator>
      secondaryFreelist{};
};

} // namespace strobe::gpu
