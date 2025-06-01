#pragma once

#include "strobe/core/memory/AllocatorReference.hpp"
#include "strobe/core/memory/Mallocator.hpp"

namespace strobe::input {

using allocator = strobe::Mallocator;
using allocator_ref = AllocatorReference<allocator>;

}
