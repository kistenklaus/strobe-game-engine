#include "strobe/rhi/cmd/command_buffer_impl.hpp"
#include "strobe/rhi/cmd/command_pool_impl.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/types/command_buffer_type.hpp"
#include "strobe/rhi/vulkan/context/pnf.hpp"
#include <vulkan/vulkan_core.h>

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

void strobe::rhi::CommandBufferImpl::flush_pc() noexcept {
  if (pushDirtyBegin < pushDirtyEnd) {
    auto *pool_impl = void_handle_ptr<CommandPoolImpl>(pool.m_handle);
    vulkan::Context *ctx = pool_impl->context.ctx();

    VkPushDataInfoEXT info{
        .sType = VK_STRUCTURE_TYPE_PUSH_DATA_INFO_EXT,
        .pNext = nullptr,
        .offset = pushDirtyBegin,
        .data =
            VkHostAddressRangeConstEXT{
                .address = &pushData[pushDirtyBegin],
                .size = pushDirtyEnd - pushDirtyBegin,
            },
    };
    ZoneScopedN("vkCmdPushData");
    vulkan::vk_cmd_push_data(ctx->pnf(), cmd.handle, &info);
  }
}
