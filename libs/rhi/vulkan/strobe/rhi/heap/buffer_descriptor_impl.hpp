#pragma once

#include "strobe/rhi/heap/resource_descriptor_heap.hpp"
#include "strobe/rhi/objects/timepoint.hpp"
#include <cassert>
#include <cstdint>
#include <utility>

namespace strobe::rhi {

struct BufferDescriptorImpl {

  explicit BufferDescriptorImpl(ResourceDescriptorHeap heap, uint32_t index,
                                Timepoint ready, Buffer buffer) noexcept
      : heap(std::move(heap)), index(index), ready(std::move(ready)),
        m_buffer(std::move(buffer)) {
    assert(index != std::numeric_limits<uint32_t>::max());
  }

  ~BufferDescriptorImpl() noexcept {
    assert(index != std::numeric_limits<uint32_t>::max());
    heap.release_buffer_descriptor_index_range(index, 1);
  }

  ResourceDescriptorHeap heap;
  uint32_t index;
  Timepoint ready;

private:
  Buffer m_buffer;
};

} // namespace strobe::rhi
