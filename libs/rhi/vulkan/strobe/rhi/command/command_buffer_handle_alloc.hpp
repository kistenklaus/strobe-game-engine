#pragma once

#include "strobe/core/memory/AllocatorReference.hpp"
#include "strobe/rhi/allocator.hpp"
// #include "strobe/core/memory/mpsc_monotonic_pool_resource.hpp"
// #include "strobe/rhi/allocator.hpp"
// #include "strobe/rhi/command/command_buffer_impl.hpp"
// #include "strobe/rhi/handle.hpp"

namespace strobe::rhi {

namespace details {
// using cmd_buf_handle_alloc_layout =
//     AllocatorReference<MPSCMonotonicPoolResource<
//         sizeof(void *), alignof(void *), strobe::rhi::allocator_ref>>;
//
// static constexpr std::size_t cmd_buf_handle_size =
//     sizeof(handle_control_block<CommandBufferImpl,
//                                 details::cmd_buf_handle_alloc_layout>);
// static constexpr std::size_t cmd_buf_handle_align =
//     alignof(handle_control_block<CommandBufferImpl,
//                                  details::cmd_buf_handle_alloc_layout>);

} // namespace details

// TODO: dependency cycle
// using cmd_buf_handle_allocator =
//     MPSCMonotonicPoolResource<details::cmd_buf_handle_size,
//     details::cmd_buf_handle_align,
//                               strobe::rhi::allocator_ref>;

using cmd_buf_handle_allocator = strobe::rhi::allocator_ref;

using cmd_buf_handle_allocator_ref =
    AllocatorReference<cmd_buf_handle_allocator>;

// static_assert(sizeof(details::cmd_buf_handle_alloc_layout) ==
//               sizeof(cmd_buf_handle_allocator_ref));
//
// static_assert(alignof(details::cmd_buf_handle_alloc_layout) ==
//               alignof(cmd_buf_handle_allocator_ref));
//
// static_assert(sizeof(handle_control_block<CommandBufferImpl,
//                                           cmd_buf_handle_allocator_ref>) ==
//               details::cmd_buf_handle_size);
//
// static_assert(alignof(handle_control_block<CommandBufferImpl,
//                                            cmd_buf_handle_allocator_ref>) ==
//               details::cmd_buf_handle_align);

} // namespace strobe::rhi
