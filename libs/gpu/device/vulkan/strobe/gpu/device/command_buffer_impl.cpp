#include "strobe/gpu/device/command_buffer_impl.hpp"
#include "strobe/gpu/device/command_pool_impl.hpp"

strobe::gpu::CommandBufferImpl::~CommandBufferImpl() noexcept {
  auto *pool_impl = void_handle_ptr<CommandPoolImpl>(pool.m_handle);
  pool_impl->recycle(nativePool);
}
