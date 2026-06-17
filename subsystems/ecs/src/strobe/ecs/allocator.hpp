#pragma once

#include "strobe/core/memory/AllocatorReference.hpp"
#include "strobe/core/memory/Mallocator.hpp"
#include "strobe/core/memory/named_allocator.hpp"
#include "strobe/core/type_traits/fixed_string.hpp"
#include <fmt/printf.h>

namespace strobe::ecs {

using allocator = NamedAllocator<Mallocator, fixed_string{"strobe-ecs"}>;

using allocator_ref = AllocatorReference<allocator>;

using event_queue_allocator = allocator_ref;

} // namespace strobe::ecs
