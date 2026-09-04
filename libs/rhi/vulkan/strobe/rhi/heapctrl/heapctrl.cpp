#include "strobe/rhi/heapctrl/heapctrl.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/heapctrl/heap_controller.hpp"

namespace strobe::rhi {

HeapController heapctrl::create_heapctrl(MemoryPool mempool,
                                         AsyncCopyEngine dma,
                                         handle_allocators *alloc) noexcept {

  return HeapController{make_void_handle<HeapControllerImpl>(
      &alloc->heapCtrlAlloc, std::move(mempool), std::move(dma),
      alloc->heapAlloc, alloc->bufAlloc)};
}
} // namespace strobe::rhi
