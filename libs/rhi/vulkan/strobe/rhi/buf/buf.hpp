#pragma once

#include "strobe/rhi/buf/buffer_impl.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/memory/memory_pool.hpp"
#include "strobe/rhi/objects/buffer.hpp"
#include "strobe/rhi/types/buffer_info.hpp"
#include "strobe/rhi/types/memory_lifetime.hpp"

namespace strobe::rhi::buf {

struct handle_allocators {
  handle_allocator<BufferImpl> bufferAllocator;
};

Buffer create_buffer(const MemoryPool &memoryPool, const BufferInfo &info,
                     const MemoryLifetime &lifetime, handle_allocators *alloc);

} // namespace strobe::rhi::buf
