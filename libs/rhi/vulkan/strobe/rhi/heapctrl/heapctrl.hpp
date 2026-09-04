#pragma once

#include "strobe/rhi/buf/buf.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/heap/heap.hpp"
#include "strobe/rhi/heapctrl/heap_controller.hpp"
#include "strobe/rhi/heapctrl/heap_controller_impl.hpp"
#include "strobe/rhi/memory/memory_pool.hpp"
namespace strobe::rhi::heapctrl {

struct handle_allocators {

  explicit handle_allocators(strobe::rhi::allocator_ref alloc,
                             heap::handle_allocators *heapAlloc,
                             buf::handle_allocators *bufAlloc) noexcept
      : heapAlloc(heapAlloc), bufAlloc(bufAlloc), heapCtrlAlloc(alloc) {}
  heap::handle_allocators *heapAlloc;
  buf::handle_allocators *bufAlloc;
  handle_allocator<HeapControllerImpl> heapCtrlAlloc;
};

HeapController create_heapctrl(MemoryPool mempool, AsyncCopyEngine dma,
                               handle_allocators *alloc) noexcept;

} // namespace strobe::rhi::heapctrl
