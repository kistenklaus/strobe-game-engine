#pragma once

#include "strobe/core/memory/AllocatorReference.hpp"
#include "strobe/core/memory/Mallocator.hpp"

namespace strobe::gpu::vulkan {

using allocator = strobe::Mallocator;
using allocator_ref = AllocatorReference<allocator>;

}
