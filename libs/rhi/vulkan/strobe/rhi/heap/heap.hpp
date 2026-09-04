#pragma once

#include "strobe/rhi/allocator.hpp"
#include "strobe/rhi/context/context.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/heap/resource_descriptor_heap.hpp"
#include "strobe/rhi/heap/resource_descriptor_heap_impl.hpp"

namespace strobe::rhi::heap {

struct handle_allocators {
  explicit handle_allocators(strobe::rhi::allocator_ref alloc) noexcept
      : alloc(alloc), rHeapAlloc(alloc) {}

  strobe::rhi::allocator_ref alloc;
  handle_allocator<ResourceDescriptorHeapImpl> rHeapAlloc;
};

ResourceDescriptorHeap create_rheap(Context context, Buffer buffer,
                                    handle_allocators *alloc);

} // namespace strobe::rhi::heap
