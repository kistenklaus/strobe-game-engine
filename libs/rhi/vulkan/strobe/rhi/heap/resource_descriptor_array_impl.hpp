#pragma once

#include "strobe/rhi/heap/resource_descriptor_heap.hpp"
#include "strobe/rhi/objects/buffer_descriptor.hpp"
#include "strobe/rhi/objects/timepoint.hpp"
#include <utility>

namespace strobe::rhi {

struct ResourceDescriptorArrayImpl {

  explicit ResourceDescriptorArrayImpl(ResourceDescriptorHeap heap,
                                       uint32_t index, uint32_t size,
                                       Timepoint ready,
                                       ResourceDescriptor *descriptors) noexcept
      : heap(std::move(heap)), index(index), size(size),
        ready(std::move(ready)), m_descriptors(descriptors) {}

  ~ResourceDescriptorArrayImpl() noexcept;

  ResourceDescriptorHeap heap;
  uint32_t index;
  uint32_t size;
  Timepoint ready;

private:
  ResourceDescriptor *m_descriptors;
};

} // namespace strobe::rhi
