#pragma once

#include "strobe/rhi/buf/buf.hpp"
#include "strobe/rhi/bvh/blas_impl.hpp"
#include "strobe/rhi/bvh/scratch_buffer.hpp"
#include "strobe/rhi/bvh/scratch_buffer_impl.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/memory/memory_pool.hpp"
#include "strobe/rhi/objects/blas.hpp"
#include "strobe/rhi/types/blas_info.hpp"
#include "strobe/rhi/types/memory_lifetime.hpp"

namespace strobe::rhi::bvh {

struct handle_allocators {
  buf::handle_allocators *bufAllocators;
  handle_allocator<BlasImpl> blasAllocator;
  handle_allocator<ScratchBufferImpl> scratchBufferAllocator;
};

Blas create_blas(MemoryPool memoryPool, ScratchBuffer scratchBuffer,
                 const BlasInfo &info, const MemoryLifetime &lifetime,
                 handle_allocators* alloc);

ScratchBuffer
create_scratch(MemoryPool memoryPool, handle_allocators* alloc);

} // namespace strobe::rhi::bvh
