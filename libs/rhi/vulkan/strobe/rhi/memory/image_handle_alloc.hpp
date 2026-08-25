#pragma once

#include "strobe/core/memory/AllocatorReference.hpp"
#include "strobe/core/memory/monotonic_pool_allocator.hpp"
#include "strobe/core/memory/sync_resource.hpp"
#include "strobe/rhi/allocator.hpp"
#include "strobe/rhi/handle.hpp"
// #include "strobe/rhi/device/image_impl.hpp"

namespace strobe::rhi {

namespace details {

// using image_handle_alloc_layout =
//     SyncResource<AllocatorReference<MonotonicPoolResource<
//         sizeof(void *), alignof(void *), strobe::gpu::allocator_ref>>>;
//
// static constexpr size_t image_handle_size =
//     sizeof(handle_control_block<ImageImpl, image_handle_alloc_layout>);
// static constexpr size_t image_handle_align =
//     alignof(handle_control_block<ImageImpl, image_handle_alloc_layout>);

} // namespace details

// using image_handle_alloc =
//     SyncResource<MonotonicPoolResource<details::image_handle_size,
//                                        details::image_handle_align,
//                                        strobe::gpu::allocator_ref>>;
using image_handle_alloc = strobe::rhi::allocator_ref;

using image_handle_alloc_ref = AllocatorReference<image_handle_alloc>;

} // namespace strobe::rhi
