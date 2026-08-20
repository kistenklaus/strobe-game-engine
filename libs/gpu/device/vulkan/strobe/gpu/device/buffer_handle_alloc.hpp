#pragma once

#include "strobe/core/memory/AllocatorReference.hpp"
#include "strobe/core/memory/monotonic_pool_allocator.hpp"
#include "strobe/core/memory/sync_resource.hpp"
#include "strobe/gpu/device/allocator.hpp"
#include "strobe/gpu/device/buffer_impl.hpp"
#include "strobe/gpu/device/handle.hpp"

namespace strobe::gpu {

namespace details {

using buffer_handle_alloc_layout =
    SyncResource<AllocatorReference<MonotonicPoolResource<
        sizeof(void *), alignof(void *), strobe::gpu::allocator_ref>>>;

static constexpr size_t buffer_handle_size =
    sizeof(handle_control_block<BufferImpl, buffer_handle_alloc_layout>);
static constexpr size_t buffer_handle_align =
    alignof(handle_control_block<BufferImpl, buffer_handle_alloc_layout>);

} // namespace details

using buffer_handle_alloc =
    SyncResource<MonotonicPoolResource<details::buffer_handle_size,
                                       details::buffer_handle_align,
                                       strobe::gpu::allocator_ref>>;

// using buffer_handle_alloc = SyncResource<AllocatorReference<strobe::Mallocator>>;

using buffer_handle_alloc_ref = AllocatorReference<buffer_handle_alloc>;

} // namespace strobe::gpu
