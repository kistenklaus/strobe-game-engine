#pragma once

#include "strobe/rhi/buf/buf.hpp"
#include "strobe/rhi/context/context.hpp"
#include "strobe/rhi/dma/async_copy_engine.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/heap/heap.hpp"
#include "strobe/rhi/heap/resource_descriptor_array_wizard.hpp"
#include "strobe/rhi/heap/resource_descriptor_heap.hpp"
#include "strobe/rhi/heap/resource_descriptor_wizard.hpp"
#include "strobe/rhi/heap/sampler_descriptor_array_wizard.hpp"
#include "strobe/rhi/heap/sampler_descriptor_heap_impl.hpp"
#include "strobe/rhi/heap/sampler_descriptor_wizard.hpp"
#include "strobe/rhi/memory/memory_pool.hpp"
#include "strobe/rhi/objects/resource_descriptor.hpp"
#include "strobe/rhi/objects/resource_descriptor_array.hpp"
#include "strobe/rhi/objects/sampler_descriptor.hpp"
#include "strobe/rhi/objects/sampler_descriptor_array.hpp"
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
                                 .bufferUsage = BufferUsage::descriptor_heap |
                                                BufferUsage::transfer_dst |
                                                BufferUsage::transfer_src,
                                 .memoryUsage = MemoryUsage::automatic,
                             },
                             {}, m_bufAlloc);
      m_rHeap = heap::create_rheap(m_memoryPool.context(), std::move(rbuf),
                                   heapAlloc);
    }
    { // sampler heap
      m_sPageSize = props.samplerDescriptorSize * 128;
      uint64_t size = m_sPageSize + props.minSamplerHeapReservedRange;
      Buffer sbuf =
          buf::create_buffer(m_memoryPool,
                             {
                                 .size = size,
                                 .bufferUsage = BufferUsage::descriptor_heap |
                                                BufferUsage::transfer_dst |
                                                BufferUsage::transfer_src,
                                 .memoryUsage = MemoryUsage::device,
                             },
                             {}, m_bufAlloc);
      m_sHeap = heap::create_sheap(m_memoryPool.context(), std::move(sbuf),
                                   heapAlloc);
    }
  }

  ResourceDescriptor
  create_resource_descriptor(const ResourceDescriptorInfo &info) noexcept {
    while (true) {
      std::lock_guard lck{m_rMutex};
      auto wizard = m_rHeap.create_descriptor_wizard(info);
      if (!wizard) {
        grow_resource_heap();
        continue;
      }
      auto async = m_dma.async_cmd();
      auto stage = async.alloc_stage(wizard.size(), wizard.alignment());
      return wizard.complete(
          stage.ptr, [&](BufferRange dst) noexcept -> Timepoint {
            return async
                .copy({.buffer = dst.buffer, .offset = dst.offset}, stage,
                      dst.size)
                .finish();
          });
    }
  }

  ResourceDescriptorArray create_resource_descriptor_array(
      span<const ResourceDescriptorInfo> infos) noexcept {
    while (true) {

      std::lock_guard lck{m_rMutex};
      auto wizard = m_rHeap.create_descriptor_array_wizard(infos);

      if (!wizard) {
        grow_resource_heap();
        continue;
      }
      auto async = m_dma.async_cmd();
      auto stage = async.alloc_stage(wizard.size(), wizard.alignment());
      return wizard.complete(
          stage.ptr, [&](BufferRange dst) noexcept -> Timepoint {
            return async
                .copy({.buffer = dst.buffer, .offset = dst.offset}, stage,
                      dst.size)
                .finish();
          });
    }
  }

  SamplerDescriptor
  create_sampler_descriptor(const SamplerDescriptorInfo &info) noexcept {

    while (true) {
      std::lock_guard lck{m_sMutex};
      auto wizard = m_sHeap.create_descriptor_wizard(info);
      if (!wizard) {
        grow_sampler_heap();
        continue;
      }
      auto async = m_dma.async_cmd();
      auto stage = async.alloc_stage(wizard.size(), wizard.alignment());
      return wizard.complete(
          stage.ptr, [&](BufferRange dst) noexcept -> Timepoint {
            return async
                .copy({.buffer = dst.buffer, .offset = dst.offset}, stage,
                      dst.size)
                .finish();
          });
    }
  }

  SamplerDescriptorArray
  create_sampler_descriptor_array(span<const SamplerDescriptorInfo> infos) {
    while (true) {
      std::lock_guard lck{m_sMutex};
      auto wizard = m_sHeap.create_descriptor_array_wizard(infos);
      if (!wizard) {
        grow_sampler_heap();
        continue;
      }
      auto async = m_dma.async_cmd();
      auto stage = async.alloc_stage(wizard.size(), wizard.alignment());
      return wizard.complete(
          stage.ptr, [&](BufferRange dst) noexcept -> Timepoint {
            return async
                .copy({.buffer = dst.buffer, .offset = dst.offset}, stage,
                      dst.size)
                .finish();
          });
    }
  }

private:
  // m_rMutex must be held!
  void grow_resource_heap() noexcept {
    auto *heap = object_handle_ptr<ResourceDescriptorHeapImpl>(m_rHeap);
    uint64_t newSize = heap->buffer().size() + m_rPageSize;
    Buffer newBuffer =
        buf::create_buffer(m_memoryPool,
                           {
                               .size = newSize,
                               .bufferUsage = BufferUsage::descriptor_heap |
                                              BufferUsage::transfer_dst |
                                              BufferUsage::transfer_src,
                               .memoryUsage = MemoryUsage::device,
                           },
                           {}, m_bufAlloc);
    Timepoint ready = m_dma.async_copy({newBuffer}, {heap->buffer()},
                                       heap->layout.descriptor_region_size());
    heap->exchange(newBuffer, ready);
  }

  // m_sMutex must be held!
  void grow_sampler_heap() noexcept {
    auto *heap = object_handle_ptr<SamplerDescriptorHeapImpl>(m_sHeap);
    uint64_t newSize = heap->buffer().size() + m_sPageSize;
    Buffer newBuffer =
        buf::create_buffer(m_memoryPool,
                           {
                               .size = newSize,
                               .bufferUsage = BufferUsage::descriptor_heap |
                                              BufferUsage::transfer_dst |
                                              BufferUsage::transfer_src,
                               .memoryUsage = MemoryUsage::device,
                           },
                           {}, m_bufAlloc);
    Timepoint ready = m_dma.async_copy({newBuffer}, {heap->buffer()},
                                       heap->layout.descriptor_region_size());
    heap->exchange(newBuffer, ready);
  }

private:
  MemoryPool m_memoryPool;
  AsyncCopyEngine m_dma;
  buf::handle_allocators *m_bufAlloc;

  uint64_t m_rPageSize;
  std::mutex m_rMutex{};
  ResourceDescriptorHeap m_rHeap{};

  uint64_t m_sPageSize;
  std::mutex m_sMutex;
  SamplerDescriptorHeap m_sHeap;
};

} // namespace strobe::rhi
