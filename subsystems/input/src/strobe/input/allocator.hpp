#pragma once

#include "strobe/core/memory/allocator_ref.hpp"
#include "strobe/core/memory/mallocator.hpp"

namespace strobe::input {

using allocator = strobe::Mallocator;
using allocator_ref = AllocatorReference<allocator>;

}
