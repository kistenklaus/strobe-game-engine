#include "strobe/gpu/device/command_buffer_impl.hpp"
#include "strobe/gpu/device/command_pool_impl.hpp"

strobe::gpu::CommandBufferImpl::CommandBufferImpl(
    CommandPool pool, NativeCommandPool *nativePool, vulkan::CommandBuffer cmd,
    const cmd_buf_state_allocator_ref &alloc) noexcept
    : pool(std::move(pool)), nativePool(nativePool), cmd(cmd), state{alloc},
      pnf_shaderObject(void_handle_ptr<CommandPoolImpl>(this->pool.m_handle)
              ->context.get()
              ->shaderObjFunc()) {
}

strobe::gpu::CommandBufferImpl::~CommandBufferImpl() noexcept {
  auto *pool_impl = void_handle_ptr<CommandPoolImpl>(pool.m_handle);
  pool_impl->recycle(nativePool);
}
