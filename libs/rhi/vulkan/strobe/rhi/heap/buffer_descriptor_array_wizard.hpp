#pragma once

#include "strobe/core/memory/AllocatorTraits.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/heap/buffer_descriptor_array.hpp"
#include "strobe/rhi/heap/buffer_descriptor_array_impl.hpp"
#include "strobe/rhi/heap/resource_descriptor_heap.hpp"
#include "strobe/rhi/heap/resource_descriptor_heap_impl.hpp"
#include "strobe/rhi/objects/timepoint.hpp"
#include "strobe/rhi/types/buffer_range.hpp"
#include <limits>
#include <memory>
#include <type_traits>
#include <utility>

namespace strobe::rhi {

class BufferDescriptorArrayWizard {
public:
  template <typename Fn>
    requires std::is_invocable_r_v<Timepoint, Fn, BufferRange>
  BufferDescriptorArray complete(Fn &&fn) noexcept {
    auto *impl = object_handle_ptr<ResourceDescriptorHeapImpl>(m_heap);
    uint64_t offset = impl->buffer_stride() * static_cast<uint64_t>(m_index);
    uint64_t size = impl->buffer_stride() * static_cast<uint64_t>(m_size);
    Buffer buffer = impl->buffer();
    Timepoint ready = fn(BufferRange{
        .buffer = buffer,
        .offset = offset,
        .size = size,
    });
    using traits = AllocatorTraits<strobe::rhi::allocator_ref>;
    Buffer *buffers = traits::allocate<Buffer>(m_alloc, m_size);
    for (uint32_t i = 0; i < m_size; ++i) {
      std::construct_at(buffers + i, m_buffers[i].buffer); // copy constructor.
    }

    return BufferDescriptorArray{make_void_handle<BufferDescriptorArrayImpl>(
        impl->get_buffer_desc_array_allocator(), std::move(m_heap),
        std::exchange(m_index, std::numeric_limits<uint32_t>::max()),
        std::move(ready), buffers, m_alloc)};
  }

  ~BufferDescriptorArrayWizard() noexcept {
    if (m_index != std::numeric_limits<uint32_t>::max()) {
      m_heap.release_buffer_descriptor_index_range(m_index, m_size);
    }
  }

private:
  friend class ResourceDescriptorHeap;
  explicit BufferDescriptorArrayWizard(
      ResourceDescriptorHeap heap, uint32_t index, uint32_t size,
      span<const BufferRange> buffers,
      strobe::rhi::allocator_ref alloc) noexcept
      : m_heap(std::move(heap)), m_index(index), m_size(size),
        m_buffers(buffers), m_alloc(alloc) {}
  ResourceDescriptorHeap m_heap;
  uint32_t m_index;
  uint32_t m_size;
  span<const BufferRange> m_buffers;
  [[no_unique_address]] strobe::rhi::allocator_ref m_alloc;
};

} // namespace strobe::rhi
