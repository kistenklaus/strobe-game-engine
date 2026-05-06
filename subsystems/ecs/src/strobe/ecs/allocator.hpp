#pragma once

#include "strobe/core/memory/AllocatorReference.hpp"
#include "strobe/core/memory/Mallocator.hpp"

namespace strobe::ecs {

using allocator = strobe::Mallocator;
using allocator_ref = AllocatorReference<allocator>;

using event_queue_allocator = allocator_ref;

} // namespace strobe::ecs
