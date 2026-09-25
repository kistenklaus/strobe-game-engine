#pragma once

#include "strobe/core/memory/allocator_ref.hpp"
#include "strobe/core/memory/allocator_traits.hpp"
#include "strobe/core/memory/monotonic_resource.hpp"
#include "strobe/core/memory/sync_resource.hpp"
#include "strobe/ecs/allocator.hpp"
namespace strobe::ecs::scheduler {

using allocator = SyncResource<MonotonicResource<strobe::ecs::allocator_ref>>;
using allocator_traits = AllocatorTraits<allocator>;

using allocator_ref = AllocatorReference<allocator>;
using allocator_ref_traits = AllocatorTraits<allocator_ref>;

} // namespace strobe::ecs::scheduler
