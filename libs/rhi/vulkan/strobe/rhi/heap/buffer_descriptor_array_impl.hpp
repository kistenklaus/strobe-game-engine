#pragma once

#include "strobe/rhi/heap/resource_descriptor_heap.hpp"
#include <limits>
#include <utility>

namespace strobe::rhi {

struct BufferDescriptorArrayImpl {
  explicit BufferDescriptorArrayImpl(ResourceDescriptorHeap heap,
                                     uint32_t index, uint32_t size) noexcept
      : heap(std::move(heap)), index(index), size(size) {}
  ~BufferDescriptorArrayImpl() noexcept {
    assert(index != std::numeric_limits<uint32_t>::max());
    heap.release_buffer_descriptor_index_range(index, size);
  }

  ResourceDescriptorHeap heap;
  uint32_t index;
  uint32_t size;
};

} // namespace strobe::rhi
