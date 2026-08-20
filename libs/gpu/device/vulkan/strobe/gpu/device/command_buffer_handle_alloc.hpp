#pragma once

#include "strobe/core/memory/AllocatorReference.hpp"
#include "strobe/core/memory/mpsc_monotonic_pool_resource.hpp"
#include "strobe/gpu/device/allocator.hpp"
#include "strobe/gpu/device/command_buffer_impl.hpp"
#include "strobe/gpu/device/handle.hpp"

namespace strobe::gpu {

namespace details {
using cmd_buf_handle_alloc_layout =
    AllocatorReference<MPSCMonotonicPoolResource<
        sizeof(void *), alignof(void *), strobe::gpu::allocator_ref>>;

static constexpr std::size_t cmd_buf_handle_size =
    sizeof(handle_control_block<CommandBufferImpl,
                                details::cmd_buf_handle_alloc_layout>);
static constexpr std::size_t cmd_buf_handle_align =
    alignof(handle_control_block<CommandBufferImpl,
                                 details::cmd_buf_handle_alloc_layout>);

}

using cmd_buf_handle_allocator =
    MPSCMonotonicPoolResource<details::cmd_buf_handle_size, details::cmd_buf_handle_align,
                              strobe::gpu::allocator_ref>;

using cmd_buf_handle_allocator_ref =
    AllocatorReference<cmd_buf_handle_allocator>;

static_assert(sizeof(details::cmd_buf_handle_alloc_layout) ==
              sizeof(cmd_buf_handle_allocator_ref));

static_assert(alignof(details::cmd_buf_handle_alloc_layout) ==
              alignof(cmd_buf_handle_allocator_ref));

static_assert(sizeof(handle_control_block<CommandBufferImpl,
                                          cmd_buf_handle_allocator_ref>) ==
              details::cmd_buf_handle_size);

static_assert(alignof(handle_control_block<CommandBufferImpl,
                                           cmd_buf_handle_allocator_ref>) ==
              details::cmd_buf_handle_align);

} // namespace strobe::gpu
