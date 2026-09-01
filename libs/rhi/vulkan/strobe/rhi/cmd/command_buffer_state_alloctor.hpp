#pragma once

#include "strobe/core/memory/AllocatorReference.hpp"
#include "strobe/core/memory/ranked_sync_resource.hpp"
#include "strobe/rhi/allocator.hpp"

namespace strobe::rhi {

using cmd_buf_state_allocator =
    RankedSyncResource<AllocatorReference<strobe::rhi::allocator>, 7, 12>;

using cmd_buf_state_allocator_ref = AllocatorReference<cmd_buf_state_allocator>;

} // namespace strobe::rhi
