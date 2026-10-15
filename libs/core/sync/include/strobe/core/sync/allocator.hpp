#pragma once

#include "strobe/core/memory/allocator_ref.hpp"
#include "strobe/core/memory/mallocator.hpp"
#include "strobe/core/memory/monotonic_resource.hpp"
#include "strobe/core/memory/sync_resource.hpp"

namespace strobe::sync {

using allocator = SyncResource<MonotonicResource<Mallocator>>;
using allocator_ref = AllocatorReference<allocator>;

} // namespace strobe::sync
