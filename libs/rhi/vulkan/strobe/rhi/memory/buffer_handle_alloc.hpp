#pragma once

#include "strobe/core/memory/AllocatorReference.hpp"
#include "strobe/core/memory/monotonic_pool_allocator.hpp"
#include "strobe/core/memory/sync_resource.hpp"
#include "strobe/rhi/allocator.hpp"
#include "strobe/rhi/memory/buffer_impl.hpp"
#include "strobe/rhi/handle.hpp"

namespace strobe::rhi {

namespace details {

// using buffer_handle_alloc_layout =
//     SyncResource<AllocatorReference<MonotonicPoolResource<
//         sizeof(void *), alignof(void *), strobe::rhi::allocator_ref>>>;
//
// static constexpr size_t buffer_handle_size =
//     sizeof(handle_control_block<BufferImpl, buffer_handle_alloc_layout>);
// static constexpr size_t buffer_handle_align =
//     alignof(handle_control_block<BufferImpl, buffer_handle_alloc_layout>);

} // namespace details

// TODO: dependency cycle somewhere or bad templates
// using buffer_handle_alloc =
//     SyncResource<MonotonicPoolResource<details::buffer_handle_size,
//                                        details::buffer_handle_align,
//                                        strobe::rhi::allocator_ref>>;

using buffer_handle_alloc = strobe::rhi::allocator_ref;

// using buffer_handle_alloc = SyncResource<AllocatorReference<strobe::Mallocator>>;

using buffer_handle_alloc_ref = AllocatorReference<buffer_handle_alloc>;

} // namespace strobe::rhi
