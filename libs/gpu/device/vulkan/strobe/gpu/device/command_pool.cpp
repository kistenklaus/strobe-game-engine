#include "strobe/gpu/device/command_pool.hpp"
#include "strobe/gpu/device/command_buffer.hpp"
#include "strobe/gpu/device/command_buffer_impl.hpp"
#include "strobe/gpu/device/command_pool_impl.hpp"
#include "strobe/gpu/device/device_impl.hpp"
#include "strobe/gpu/device/handle.hpp"
#include "strobe/gpu/vulkan/command_buffer.hpp"
#include "strobe/gpu/vulkan/command_pool.hpp"

namespace strobe::gpu {

CommandPool::CommandPool(const CommandPool &o) noexcept : m_handle(o.m_handle) {
  if (m_handle != nullptr) {
    pin_void_handle<CommandPoolImpl>(m_handle);
  }
}

CommandPool::CommandPool(CommandPool &&o) noexcept
    : m_handle(std::exchange(o.m_handle, nullptr)) {}

CommandPool &CommandPool::operator=(const CommandPool &o) noexcept {
  if (this == &o) {
    return *this;
  }
  if (o.m_handle != nullptr) {
    pin_void_handle<CommandPoolImpl>(o.m_handle);
  }
  unpin_void_handle<CommandPoolImpl>(m_handle);
  m_handle = o.m_handle;
  return *this;
}

CommandPool &CommandPool::operator=(CommandPool &&o) noexcept {
  if (this == &o) {
    return *this;
  }
  unpin_void_handle<CommandPoolImpl>(m_handle);
  m_handle = std::exchange(o.m_handle, nullptr);
  return *this;
}

CommandPool::~CommandPool() noexcept {
  unpin_void_handle<CommandPoolImpl>(m_handle);
}

CommandBuffer CommandPool::alloc(bool primary) {
  auto *impl = void_handle_ptr<CommandPoolImpl>(m_handle);

  vulkan::CommandBuffer cmd = impl->alloc_cached(primary);
  if (cmd) {
    vulkan::reset_command_buffer(cmd);
  } else {
    auto *device_impl = void_handle_ptr<DeviceImpl>(impl->device.m_handle);
    cmd = vulkan::alloc_command_buffer(
        &device_impl->context,
        {.pool = impl->pool,
         .level = primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY
                          : VK_COMMAND_BUFFER_LEVEL_SECONDARY});
  }
  assert(cmd);
  return CommandBuffer{
      make_void_handle<CommandBufferImpl>(*this, cmd, primary)};
}

void CommandPool::reset() {
  auto *impl = void_handle_ptr<CommandPoolImpl>(m_handle);
  auto *device_impl = void_handle_ptr<DeviceImpl>(impl->device.m_handle);
  vulkan::reset_command_pool(&device_impl->context, impl->pool);
}

} // namespace strobe::gpu
