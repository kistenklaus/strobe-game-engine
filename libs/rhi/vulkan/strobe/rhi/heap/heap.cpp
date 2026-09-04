#include "strobe/rhi/heap/heap.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/heap/resource_descriptor_heap_impl.hpp"

namespace strobe::rhi {

ResourceDescriptorHeap heap::create_rheap(Context context, Buffer buffer,
                                          handle_allocators *alloc) {
  return ResourceDescriptorHeap{make_void_handle<ResourceDescriptorHeapImpl>(
      &alloc->rHeapAlloc, std::move(context), std::move(buffer), alloc->alloc)};
}

} // namespace strobe::rhi
