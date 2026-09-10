#include "strobe/rhi/heap/heap.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/heap/resource_descriptor_heap_impl.hpp"
#include "strobe/rhi/object_factory.hpp"

namespace strobe::rhi {

ResourceDescriptorHeap heap::create_rheap(Context context, Buffer buffer,
                                          handle_allocators *alloc) {
  return detail::make_object<ResourceDescriptorHeap>(
      make_void_handle<ResourceDescriptorHeapImpl>(
          &alloc->rHeapAlloc, std::move(context), std::move(buffer),
          alloc->alloc));
}

} // namespace strobe::rhi
