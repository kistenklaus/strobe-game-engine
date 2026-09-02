#pragma once

#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/heap/buffer_descriptor.hpp"
#include "strobe/rhi/heap/buffer_descriptor_impl.hpp"
#include "strobe/rhi/heap/resource_descriptor_heap.hpp"
#include "strobe/rhi/heap/resource_descriptor_heap_impl.hpp"
#include "strobe/rhi/objects/timepoint.hpp"
#include "strobe/rhi/types/buffer_offset.hpp"
#include <type_traits>
#include <utility>

namespace strobe::rhi {

class BufferDescriptorWizard {
public:
  template <typename Fn>
    requires std::is_invocable_r_v<Timepoint, Fn, BufferOffset>
  BufferDescriptor complete(Fn &&fn) {
    auto *impl = object_handle_ptr<ResourceDescriptorHeapImpl>(m_heap);
    uint64_t offset = impl->buffer_stride() * static_cast<uint64_t>(m_index);
    Buffer buffer = impl->buffer();
    Timepoint ready = fn(BufferOffset{.buffer = buffer, .offset = offset});
    return BufferDescriptor{make_void_handle<BufferDescriptorImpl>(
        impl->get_buffer_desc_allocator(), std::move(m_heap),
        std::exchange(m_index, 0))};
  }

private:
  friend class ResourceDescriptorHeap;
  explicit BufferDescriptorWizard(ResourceDescriptorHeap heap,
                                  uint32_t index) noexcept
      : m_heap(std::move(heap)), m_index(index) {}
  ResourceDescriptorHeap m_heap;
  uint32_t m_index;
};

} // namespace strobe::rhi
