#pragma once

#include "strobe/rhi/buf/buf.hpp"
#include "strobe/rhi/dma/async_copy_engine.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/heap/buffer_descriptor.hpp"
#include "strobe/rhi/heap/buffer_descriptor_array.hpp"
#include "strobe/rhi/heap/buffer_descriptor_array_wizard.hpp"
#include "strobe/rhi/heap/buffer_descriptor_wizard.hpp"
#include "strobe/rhi/heap/resource_descriptor_heap.hpp"
#include "strobe/rhi/objects/timepoint.hpp"
#include "strobe/rhi/stage/stage_arena.hpp"

namespace strobe::rhi {

struct HeapController {

  BufferDescriptor create_buffer_descriptor(const BufferDescriptorInfo &info) {
    std::lock_guard lck{m_mutex};
    while (true) {
      // Let's be precise here:
      // do we have to serialize across grow and wizard completion right?
      auto wizard = m_resourceHeap.create_buffer_descriptor_wizard(info);
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
    std::lock_guard lck{m_mutex};
    while (true) {
      // Let's be precise here:
      // do we have to serialize across grow and wizard completion right?
      auto wizard = m_resourceHeap.create_buffer_descriptor_array_wizard(infos);
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
  void grow_resource_heap() {
    auto *heap = object_handle_ptr<ResourceDescriptorHeapImpl>(m_resourceHeap);
    Buffer newBuffer =
        buf::create_buffer(m_memoryPool,
                           {
                               .size = 0,
                               .bufferUsage = BufferUsage::descriptor_heap,
                               .memoryUsage = MemoryUsage::device,
                           },
                           {}, m_bufAlloc);
    Timepoint ready =
        m_dma.async_copy(newBuffer, heap->buffer(), heap->heap_size());
    object_handle_ptr<ResourceDescriptorHeapImpl>(m_resourceHeap)
        ->exchange(newBuffer, ready);
  }

private:
  std::mutex m_mutex;
  AsyncCopyEngine m_dma;
  ResourceDescriptorHeap m_resourceHeap;

  MemoryPool m_memoryPool;
  buf::handle_allocators *m_bufAlloc;
};

} // namespace strobe::rhi
