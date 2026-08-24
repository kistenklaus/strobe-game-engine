#include "strobe/gpu/device/command_buffer_impl.hpp"
#include "strobe/gpu/device/command_buffer_type.hpp"
#include "strobe/gpu/device/command_pool_impl.hpp"
#include "strobe/gpu/device/handle.hpp"

strobe::gpu::CommandBufferImpl::CommandBufferImpl(
    CommandPool pool, NativeCommandPool *nativePool, vulkan::CommandBuffer cmd,
    CommandBufferFlags flags, const cmd_buf_state_allocator_ref &alloc) noexcept
    : pool(std::move(pool)), nativePool(nativePool), cmd(cmd), state{alloc},
      flags(flags), pnf(void_handle_ptr<CommandPoolImpl>(this->pool.m_handle)
                            ->context.get()
                            ->pnf()),
      m_profilerScope{void_handle_ptr<CommandPoolImpl>(this->pool.m_handle)
                          ->context.profiler()} {}

strobe::gpu::CommandBufferImpl::~CommandBufferImpl() noexcept {
  auto *pool_impl = void_handle_ptr<CommandPoolImpl>(pool.m_handle);
  pool_impl->recycle(nativePool);

  // push current stage alloc to recycle list
  if (StagingBuffer *stage = m_stageAllocator.buffer()) {
    stage->next = m_stageRecycleListHead;
    m_stageRecycleListHead = stage;
    if (m_stageRecycleListTail == nullptr) {
      m_stageRecycleListTail = stage;
    }
  }
  pool_impl->recycle_staging(m_stageRecycleListHead, m_stageRecycleListTail);
}

strobe::gpu::BufferBinding
strobe::gpu::CommandBufferImpl::alloc_staging(VkDeviceSize size,
                                              uint16_t alignment) {
  ZoneScopedN("cmd/alloc-staging");
  BufferBinding binding = m_stageAllocator.try_alloc(size, alignment);
  if (binding) {
    return binding;
  }

  // Allocation failed => request new staging buffer from pool.
  auto *pool_impl = void_handle_ptr<CommandPoolImpl>(pool.m_handle);
  StagingBuffer *newStage = pool_impl->alloc_staging(size);

  StagingBumpAllocator newAlloc{newStage};
  binding = newAlloc.try_alloc(size, alignment);
  assert(binding);
  if (newAlloc.remaining() < m_stageAllocator.remaining()) {
    // push new stage to recycle
    newStage->next = m_stageRecycleListHead;
    m_stageRecycleListHead = newStage;
    if (m_stageRecycleListTail == nullptr) {
      m_stageRecycleListTail = newStage;
    }
  } else {
    // push old stage to recycle
    if (StagingBuffer *stage = m_stageAllocator.buffer()) {
      stage->next = m_stageRecycleListHead;
      m_stageRecycleListHead = stage;
      if (m_stageRecycleListTail == nullptr) {
        m_stageRecycleListTail = stage;
      }
    }
    m_stageAllocator = newAlloc;
  }

  return binding;
}
