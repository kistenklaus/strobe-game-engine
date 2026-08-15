#pragma once

#include "strobe/core/memory/AllocatorReference.hpp"
#include "strobe/core/memory/ranked_mpsc_resource.hpp"
#include "strobe/gpu/device/allocator.hpp"

namespace strobe::gpu {

using cmd_buf_state_allocator =
    RankedMPSCResource<AllocatorReference<strobe::gpu::allocator>, 7, 12>;

using cmd_buf_state_allocator_ref = AllocatorReference<cmd_buf_state_allocator>;

} // namespace strobe::gpu
