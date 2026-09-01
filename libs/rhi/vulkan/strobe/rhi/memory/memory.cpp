#include "strobe/rhi/memory/memory.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/memory/memory_pool_impl.hpp"

namespace strobe::rhi {

MemoryPool mem::create_mem_pool(Context context,
                                mem::handle_allocators *alloc) {
  ZoneScopedN("mem/create-mem-pool");
  return MemoryPool{make_void_handle<MemoryPoolImpl>(
      &alloc->poolAlloc, std::move(context), alloc->alloc)};
}

} // namespace strobe::rhi
