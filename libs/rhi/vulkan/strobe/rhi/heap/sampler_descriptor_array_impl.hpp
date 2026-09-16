#pragma once

#include "strobe/rhi/heap/sampler_descriptor_heap.hpp"
#include "strobe/rhi/objects/sampler_descriptor.hpp"
#include "strobe/rhi/objects/timepoint.hpp"
namespace strobe::rhi {

struct SamplerDescriptorArrayImpl {

  explicit SamplerDescriptorArrayImpl(SamplerDescriptorHeap heap,
                                      uint32_t index, uint32_t size,
                                      Timepoint ready,
                                      SamplerDescriptor *descriptors) noexcept
      : heap(std::move(heap)), index(index), size(size),
        ready(std::move(ready)), m_descriptors(descriptors) {}

  ~SamplerDescriptorArrayImpl() noexcept;

  SamplerDescriptorHeap heap;
  uint32_t index;
  uint32_t size;
  Timepoint ready;

private:
  SamplerDescriptor *m_descriptors;
};

} // namespace strobe::rhi
