#include "strobe/gpu/device/command_pool.hpp"
#include "strobe/gpu/device/command_buffer.hpp"
#include "strobe/gpu/device/command_buffer_impl.hpp"
#include "strobe/gpu/device/command_buffer_type.hpp"
#include "strobe/gpu/device/command_pool_impl.hpp"
#include "strobe/gpu/device/handle.hpp"

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

CommandBuffer CommandPool::alloc(CommandBufferFlags flags) {
  ZoneScopedN("CommandPool::alloc");
  assert(m_handle);
  auto *impl = void_handle_ptr<CommandPoolImpl>(m_handle);
  auto [nativePool, cmd] =
      impl->alloc((flags & CommandBufferFlags::secondary) == 0);

  return alloc_void_handle<CommandBufferImpl,
                           strobe::gpu::cmd_buf_handle_allocator_ref>(
      impl->get_handle_allocator(), *this, nativePool, cmd, flags,
      impl->get_state_allocator());
}

} // namespace strobe::gpu
