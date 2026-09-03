#pragma once

#include "strobe/core/memory/AllocatorTraits.hpp"
#include "strobe/rhi/allocator.hpp"
#include "strobe/rhi/heap/resource_descriptor_heap.hpp"
#include "strobe/rhi/objects/buffer.hpp"
#include "strobe/rhi/objects/timepoint.hpp"
#include <memory>
#include <utility>

namespace strobe::rhi {

struct BufferDescriptorArrayImpl {

  explicit BufferDescriptorArrayImpl(ResourceDescriptorHeap heap,
                                     uint32_t index, uint32_t size,
                                     Timepoint ready, Buffer *buffers,
                                     strobe::rhi::allocator_ref alloc) noexcept
      : heap(std::move(heap)), index(index), size(size),
        ready(std::move(ready)), m_alloc(alloc), m_buffers(buffers) {}
  ~BufferDescriptorArrayImpl() noexcept {
    assert(index != std::numeric_limits<uint32_t>::max());
    heap.release_buffer_descriptor_index_range(index, size);
    assert(m_buffers != nullptr);
    std::destroy_n(m_buffers, size);
    AllocatorTraits<strobe::rhi::allocator_ref>::deallocate(
        m_alloc, m_buffers, size * sizeof(Buffer), alignof(Buffer));
  }

  ResourceDescriptorHeap heap;
  uint32_t index;
  uint32_t size;
  Timepoint ready;

private:
  [[no_unique_address]] strobe::rhi::allocator_ref m_alloc;
  Buffer *m_buffers;
};

} // namespace strobe::rhi
