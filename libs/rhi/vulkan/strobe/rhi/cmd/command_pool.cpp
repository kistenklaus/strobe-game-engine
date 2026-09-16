#include "strobe/rhi/objects/command_pool.hpp"
#include "strobe/rhi/cmd/command_buffer_impl.hpp"
#include "strobe/rhi/cmd/command_pool_impl.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/objects/command_buffer.hpp"
#include "strobe/rhi/object_factory.hpp"

namespace strobe::rhi {

void CommandPool::pin(void *handle) noexcept {
  pin_void_handle<CommandPoolImpl>(handle);
}
void CommandPool::unpin(void *handle) noexcept {
  unpin_void_handle<CommandPoolImpl>(handle);
}

CommandBuffer CommandPool::alloc(CommandBufferFlags flags) noexcept {
  ZoneScopedN("CommandPool::alloc");
  assert(m_handle);
  auto *impl = void_handle_ptr<CommandPoolImpl>(m_handle);
  auto [nativePool, cmd] =
      impl->alloc((flags & CommandBufferFlags::secondary) == 0);

  return detail::make_object<CommandBuffer>(make_void_handle<CommandBufferImpl>(
      &impl->cmdAlloc, *this, impl->stagingPool, nativePool, cmd, flags,
      impl->stateAlloc));
}

} // namespace strobe::rhi
