#pragma once

#include "strobe/core/memory/AllocatorReference.hpp"
#include "strobe/core/memory/Mallocator.hpp"
namespace strobe::window {

using allocator = strobe::Mallocator;
using allocator_ref = AllocatorReference<allocator>;

namespace details {

allocator_ref makeAllocatorRef(allocator& alloc) {
  return allocator_ref(&alloc);
}

}  // namespace detailas

}  // namespace strobe::window
