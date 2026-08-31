#pragma once

#include "strobe/rhi/gc/garbage_collector.hpp"
#include "strobe/rhi/gc/garbage_collector_impl.hpp"
#include "strobe/rhi/handle.hpp"
namespace strobe::rhi::gc {

struct handle_allocators {
  explicit handle_allocators(strobe::rhi::allocator_ref alloc) noexcept
      : alloc(alloc), gcAlloc(alloc) {}

  strobe::rhi::allocator_ref alloc;
  handle_allocator<GarbageCollectorImpl> gcAlloc;
};

GarbageCollector create_gc(Context context, span<Timeline> timelines,
                           gc::handle_allocators *alloc);

} // namespace strobe::rhi::gc
