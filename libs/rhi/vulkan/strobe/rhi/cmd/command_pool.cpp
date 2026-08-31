#include "strobe/rhi/objects/command_pool.hpp"
#include "strobe/rhi/cmd/command_buffer_impl.hpp"
#include "strobe/rhi/cmd/command_pool_impl.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/objects/command_buffer.hpp"
#include "strobe/rhi/types/command_buffer_type.hpp"

namespace strobe::rhi {

CommandPool::CommandPool(const CommandPool &o) noexcept : Object(o.m_handle) {
  if (m_handle != nullptr) {
    pin_void_handle<CommandPoolImpl>(m_handle);
  }
}

CommandPool::CommandPool(CommandPool &&o) noexcept
    : Object(std::exchange(o.m_handle, nullptr)) {}

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

  return CommandBuffer{make_void_handle<CommandBufferImpl>(
      &impl->cmdAlloc, *this, impl->stagingPool, nativePool, cmd, flags,
      impl->stateAlloc)};
}

} // namespace strobe::rhi
