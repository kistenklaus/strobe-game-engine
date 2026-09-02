#pragma once

#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/heap/buffer_descriptor.hpp"
#include "strobe/rhi/heap/buffer_descriptor_impl.hpp"
#include "strobe/rhi/heap/resource_descriptor_heap.hpp"
#include "strobe/rhi/heap/resource_descriptor_heap_impl.hpp"
#include "strobe/rhi/objects/timepoint.hpp"
#include "strobe/rhi/types/buffer_range.hpp"
#include <limits>
#include <type_traits>
#include <utility>

namespace strobe::rhi {

class BufferDescriptorWizard {
public:
  template <typename Fn>
    requires std::is_invocable_r_v<Timepoint, Fn, BufferRange>
  BufferDescriptor complete(Fn &&fn) noexcept {
    auto *impl = object_handle_ptr<ResourceDescriptorHeapImpl>(m_heap);
    const uint64_t size = impl->buffer_stride();
    const uint64_t offset = size * static_cast<uint64_t>(m_index);
    Buffer buffer = impl->buffer();
    Timepoint ready = fn(BufferRange{
        .buffer = buffer,
        .offset = offset,
        .size = size,
    });
    return BufferDescriptor{make_void_handle<BufferDescriptorImpl>(
        impl->get_buffer_desc_allocator(), std::move(m_heap),
        std::exchange(m_index, std::numeric_limits<uint32_t>::max()),
        std::move(ready), m_bufferRange.buffer)};
  }
  ~BufferDescriptorWizard() noexcept {
    if (m_index != std::numeric_limits<uint32_t>::max()) {
      m_heap.release_buffer_descriptor_index_range(m_index, 1);
    }
  }

private:
  friend class ResourceDescriptorHeap;
  explicit BufferDescriptorWizard(ResourceDescriptorHeap heap, uint32_t index,
                                  BufferRange bufferRange) noexcept
      : m_heap(std::move(heap)), m_index(index), m_bufferRange(bufferRange) {}
  ResourceDescriptorHeap m_heap;
  uint32_t m_index;
  BufferRange m_bufferRange;
};

} // namespace strobe::rhi
