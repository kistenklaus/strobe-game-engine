#pragma once

#include "strobe/rhi/buf/buf.hpp"
#include "strobe/rhi/context/context.hpp"
#include "strobe/rhi/dma/async_copy_engine.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/heap/buffer_descriptor_array_wizard.hpp"
#include "strobe/rhi/heap/buffer_descriptor_wizard.hpp"
#include "strobe/rhi/heap/heap.hpp"
#include "strobe/rhi/heap/resource_descriptor_heap.hpp"
#include "strobe/rhi/memory/memory_pool.hpp"
#include "strobe/rhi/objects/buffer_descriptor.hpp"
#include "strobe/rhi/objects/buffer_descriptor_array.hpp"
#include "strobe/rhi/objects/timepoint.hpp"
#include "strobe/rhi/stage/stage_arena.hpp"

namespace strobe::rhi {

struct HeapControllerImpl {
  explicit HeapControllerImpl(MemoryPool mempool, AsyncCopyEngine dma,
                              heap::handle_allocators *heapAlloc,
                              buf::handle_allocators *bufAlloc) noexcept
      : m_memoryPool(std::move(mempool)), m_dma(std::move(dma)),
        m_bufAlloc(bufAlloc) {
    auto &props =
        m_memoryPool.context().ctx()->deviceInfo().properties.descriptorHeap;
    { // resource heap.
      m_rPageSize =
          props.bufferDescriptorSize * 1024 + props.imageDescriptorSize * 1024;
      uint64_t size = m_rPageSize + props.minResourceHeapReservedRange;

      Buffer rbuf =
          buf::create_buffer(m_memoryPool,
                             {
                                 .size = size,
                                 .bufferUsage = BufferUsage::descriptor_heap,
                                 .memoryUsage = MemoryUsage::automatic,
                             },
                             {}, bufAlloc);
      m_rHeap = heap::create_rheap(m_memoryPool.context(), std::move(rbuf),
                                   heapAlloc);
    }
  }

  BufferDescriptor create_buffer_descriptor(const BufferDescriptorInfo &info) {
    assert(info.buffer);
    while (true) {
      auto wizard = m_rHeap.create_buffer_descriptor_wizard(info);

      std::lock_guard lck{m_mutex};
      if (!wizard) {
        grow_resource_heap();
        continue;
      }
      auto async = m_dma.async_cmd();
      auto stage = async.alloc_stage(wizard.size(), wizard.alignment());
      return wizard.complete(
          stage.ptr, [&](BufferRange dst) noexcept -> Timepoint {
            async.copy({.buffer = dst.buffer, .offset = dst.offset}, stage,
                       dst.size);
            return async.finish();
          });
    }
  }

  BufferDescriptorArray
  create_buffer_descriptor_array(span<const BufferDescriptorInfo> infos) {
    while (true) {
      auto wizard = m_rHeap.create_buffer_descriptor_array_wizard(infos);

      std::lock_guard lck{m_mutex};
      if (!wizard) {
        grow_resource_heap();
        continue;
      }
      auto async = m_dma.async_cmd();
      auto stage = async.alloc_stage(wizard.size(), wizard.alignment());
      return wizard.complete(
          stage.ptr, [&](BufferRange dst) noexcept -> Timepoint {
            async.copy({.buffer = dst.buffer, .offset = dst.offset}, stage,
                       dst.size);
            return async.finish();
          });
    }
  }

private:
  // m_mutex must be held!
  void grow_resource_heap() {
    auto *heap = object_handle_ptr<ResourceDescriptorHeapImpl>(m_rHeap);
    uint64_t newSize = heap->buffer().size() + m_rPageSize;
    Buffer newBuffer =
        buf::create_buffer(m_memoryPool,
                           {
                               .size = newSize,
                               .bufferUsage = BufferUsage::descriptor_heap,
                               .memoryUsage = MemoryUsage::device,
                           },
                           {}, m_bufAlloc);
    Timepoint ready = m_dma.async_copy(newBuffer, heap->buffer(),
                                       heap->layout.descriptor_region_size());
    object_handle_ptr<ResourceDescriptorHeapImpl>(m_rHeap)->exchange(newBuffer,
                                                                     ready);
  }

private:
  MemoryPool m_memoryPool;
  AsyncCopyEngine m_dma;
  buf::handle_allocators *m_bufAlloc;

  uint32_t m_rPageSize;

  std::mutex m_mutex{};
  ResourceDescriptorHeap m_rHeap{};
};

} // namespace strobe::rhi
