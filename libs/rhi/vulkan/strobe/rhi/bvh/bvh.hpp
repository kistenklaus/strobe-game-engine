#pragma once

#include "strobe/rhi/buf/buf.hpp"
#include "strobe/rhi/bvh/bvh_impl.hpp"
#include "strobe/rhi/bvh/scratch_buffer.hpp"
#include "strobe/rhi/bvh/scratch_buffer_impl.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/memory/memory_pool.hpp"
#include "strobe/rhi/objects/blas.hpp"
#include "strobe/rhi/objects/tlas.hpp"
#include "strobe/rhi/types/blas_info.hpp"
#include "strobe/rhi/types/memory_lifetime.hpp"
#include "strobe/rhi/types/tlas_info.hpp"

namespace strobe::rhi::bvh {

struct handle_allocators {
  explicit handle_allocators(strobe::rhi::allocator_ref alloc,
                             buf::handle_allocators *bufAlloc) noexcept
      : bufAllocators(bufAlloc), bvhAllocator(alloc),
        scratchAllocator(alloc) {}
  buf::handle_allocators *bufAllocators;
  handle_allocator<BvhImpl> bvhAllocator;
  handle_allocator<ScratchBufferImpl> scratchAllocator;
};

Blas create_blas(MemoryPool memoryPool, ScratchBuffer scratchBuffer,
                 const BlasInfo &info, const MemoryLifetime &lifetime,
                 handle_allocators *alloc);

Tlas create_tlas(MemoryPool memoryPool, ScratchBuffer scratchBuffer,
                 const TlasInfo &info, const MemoryLifetime &lifetime,
                 handle_allocators *alloc);

ScratchBuffer create_scratch(MemoryPool memoryPool, handle_allocators *alloc);

} // namespace strobe::rhi::bvh
