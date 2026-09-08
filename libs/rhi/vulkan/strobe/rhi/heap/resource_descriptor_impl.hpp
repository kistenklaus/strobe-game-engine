#pragma once

#include "strobe/rhi/heap/resource.hpp"
#include "strobe/rhi/heap/resource_descriptor_heap.hpp"
#include "strobe/rhi/objects/timepoint.hpp"
#include <cassert>
#include <cstdint>
#include <utility>

namespace strobe::rhi {

struct ResourceDescriptorImpl {

  explicit ResourceDescriptorImpl(ResourceDescriptorHeap heap, uint32_t index,
                                  Timepoint ready, Resource resource) noexcept
      : heap(std::move(heap)), index(index), ready(std::move(ready)),
        m_resource(std::move(resource)) {
    assert(index != std::numeric_limits<uint32_t>::max());
  }

  ~ResourceDescriptorImpl() noexcept;

  ResourceDescriptorHeap heap;
  uint32_t index;
  Timepoint ready;

private:
  Resource m_resource;
};

} // namespace strobe::rhi
