#pragma once

#include "strobe/core/memory/monotonic_pool_allocator.hpp"
#include "strobe/core/memory/mpsc_monotonic_pool_resource.hpp"
#include "strobe/core/memory/sync_resource.hpp"
#include "strobe/gpu/device/allocator.hpp"
#include "strobe/gpu/device/handle.hpp"
#include "strobe/gpu/device/memory_allocation_impl.hpp"

namespace strobe::gpu {

namespace details {
using memory_allocation_handle_alloc_layout =
    AllocatorReference<SyncResource<MonotonicPoolResource<
        sizeof(void *), alignof(void *), strobe::gpu::allocator_ref>>>;

static constexpr std::size_t memory_allocation_handle_size =
    sizeof(handle_control_block<MemoryAllocationImpl,
                                memory_allocation_handle_alloc_layout>);
static constexpr std::size_t memory_allocation_handle_align =
    alignof(handle_control_block<MemoryAllocationImpl,
                                 memory_allocation_handle_alloc_layout>);

} // namespace details

using memory_allocation_handle_allocator =
    SyncResource<MonotonicPoolResource<
        details::memory_allocation_handle_size,
        details::memory_allocation_handle_align, strobe::gpu::allocator_ref>>;

using memory_allocation_handle_allocator_ref =
    AllocatorReference<memory_allocation_handle_allocator>;

} // namespace strobe::gpu
