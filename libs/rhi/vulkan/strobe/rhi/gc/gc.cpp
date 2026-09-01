#include "strobe/rhi/gc/gc.hpp"
#include "strobe/rhi/gc/garbage_collector_impl.hpp"
#include "strobe/rhi/handle.hpp"

namespace strobe::rhi {

GarbageCollector gc::create_gc(Context context, span<Timeline> timelines,
                               gc::handle_allocators *alloc) {
  ZoneScopedN("gc/create-gc");
  return GarbageCollector{make_void_handle<GarbageCollectorImpl>(
      &alloc->gcAlloc, std::move(context), timelines, alloc->alloc)};
}

} // namespace strobe::rhi
