#pragma once
#include "strobe/rhi/allocator.hpp"
#include "strobe/rhi/buf/buf.hpp"
#include "strobe/rhi/bvh/bvh.hpp"
#include "strobe/rhi/cmd/cmd.hpp"
#include "strobe/rhi/device/device_impl.hpp"
#include "strobe/rhi/dma/dma.hpp"
#include "strobe/rhi/gc/gc.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/img/img.hpp"
#include "strobe/rhi/memory/memory.hpp"
#include "strobe/rhi/queue/que.hpp"
#include "strobe/rhi/shader/shader.hpp"
#include "strobe/rhi/stage/stage.hpp"
#include "strobe/rhi/swapchain/swap.hpp"
#include "strobe/rhi/sync/sync.hpp"

namespace strobe::rhi {

struct handle_allocators {
  explicit handle_allocators(strobe::rhi::allocator_ref alloc)
      : alloc(alloc), shaderAlloc(alloc), syncAlloc(alloc), memAlloc(alloc),
        bufAlloc(alloc), imgAlloc(alloc), stageAlloc(alloc, &bufAlloc),
        bvhAlloc(alloc, &bufAlloc), cmdAlloc(alloc),
        swapAlloc(alloc, &imgAlloc), gcAlloc(alloc), queAlloc(alloc),
        dmaAlloc(alloc, &cmdAlloc), deviceAlloc(alloc) {}

  handle_allocators(const handle_allocators &) = delete;
  handle_allocators(handle_allocators &&) = delete;
  handle_allocators &operator=(const handle_allocators &) = delete;
  handle_allocators &operator=(handle_allocators &&) = delete;

  strobe::rhi::allocator_ref alloc;
  shader::handle_allocators shaderAlloc;
  sync::handle_allocators syncAlloc;
  mem::handle_allocators memAlloc;
  buf::handle_allocators bufAlloc;
  img::handle_allocators imgAlloc;
  stage::handle_allocators stageAlloc;
  bvh::handle_allocators bvhAlloc;
  cmd::handle_allocators cmdAlloc;
  swap::handle_allocators swapAlloc;
  gc::handle_allocators gcAlloc;
  que::handle_allocators queAlloc;
  dma::handle_allocators dmaAlloc;
  handle_allocator<DeviceImpl> deviceAlloc;
};

} // namespace strobe::rhi
