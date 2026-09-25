#pragma once

#include "strobe/core/memory/allocator_ref.hpp"
#include "strobe/core/memory/mallocator.hpp"
namespace strobe::window {

using allocator = strobe::Mallocator;
using allocator_ref = AllocatorReference<allocator>;

namespace details {

inline allocator_ref makeAllocatorRef(allocator& alloc) {
  return allocator_ref(&alloc);
}

}  // namespace detailas

}  // namespace strobe::window
