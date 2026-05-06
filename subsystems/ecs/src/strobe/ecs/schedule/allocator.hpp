#pragma once

#include "strobe/core/memory/AllocatorReference.hpp"
#include "strobe/core/memory/monotonic_resource.hpp"
#include "strobe/core/memory/sync_resource.hpp"
#include "strobe/ecs/allocator.hpp"

namespace strobe::ecs {

using job_allocator =
    strobe::SyncResource<strobe::MonotonicResource<strobe::ecs::allocator_ref>>;

using job_allocator_ref = AllocatorReference<job_allocator>;

} // namespace strobe::ecs
