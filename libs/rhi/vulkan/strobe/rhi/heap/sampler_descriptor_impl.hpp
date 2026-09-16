#pragma once

#include "strobe/rhi/heap/sampler_descriptor_heap.hpp"
#include "strobe/rhi/objects/timepoint.hpp"
#include <limits>
namespace strobe::rhi {

struct SamplerDescriptorImpl {

  explicit SamplerDescriptorImpl(SamplerDescriptorHeap heap, uint32_t index,
                                 Timepoint ready) noexcept
      : heap(std::move(heap)), index(index), ready(std::move(ready)) {
    assert(index != std::numeric_limits<uint32_t>::max());
  }
  ~SamplerDescriptorImpl() noexcept;

  SamplerDescriptorHeap heap;
  uint32_t index;
  Timepoint ready;
};

} // namespace strobe::rhi
