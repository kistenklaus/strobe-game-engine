#include "strobe/rhi/cmd/command_buffer_impl.hpp"
#include "strobe/rhi/cmd/command_pool_impl.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/types/command_buffer_type.hpp"

strobe::rhi::CommandBufferImpl::CommandBufferImpl(
    CommandPool pool, StagingPool stagePool, NativeCommandPool *nativePool,
    vulkan::CommandBuffer cmd, CommandBufferFlags flags,
    const cmd_buf_state_allocator_ref &alloc) noexcept
    : pool(std::move(pool)), nativePool(nativePool), cmd(cmd), state{alloc},
      flags(flags),
      ctx(void_handle_ptr<CommandPoolImpl>(this->pool.m_handle)->context.ctx()),
      localStage(std::move(stagePool))
#ifdef STROBE_TRACY
      ,
      m_profilerScope(
          &object_handle_ptr<CommandPoolImpl>(this->pool)->profilerContext)
#endif
{
}

strobe::rhi::CommandBufferImpl::~CommandBufferImpl() noexcept {
  auto *pool_impl = void_handle_ptr<CommandPoolImpl>(pool.m_handle);
  pool_impl->recycle(nativePool);
}
