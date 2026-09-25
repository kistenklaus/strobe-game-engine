#pragma once

#include "strobe/core/memory/allocator_ref.hpp"
#include "strobe/core/memory/mallocator.hpp"
namespace strobe::rhi {

using allocator = strobe::Mallocator;
using allocator_ref = AllocatorReference<allocator>;

// default constructible global scratch space.
using scratch_allocator = strobe::Mallocator;

} // namespace strobe::rhi
