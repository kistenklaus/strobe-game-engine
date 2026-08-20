#pragma once

#include "strobe/core/memory/AllocatorReference.hpp"
#include "strobe/core/memory/Mallocator.hpp"
#include <sys/wait.h>

namespace strobe::gpu {

using allocator = Mallocator;
using allocator_ref = AllocatorReference<allocator>;

}
