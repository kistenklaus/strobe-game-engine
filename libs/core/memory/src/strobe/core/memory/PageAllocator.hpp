#pragma once

#include <cstddef>
#include <cstdlib>

#include "strobe/core/memory/AllocatorTraits.hpp"

namespace strobe {

class PageAllocator {
 public:
  [[nodiscard]] void* allocate(std::size_t size, std::size_t align);

  void deallocate(void* ptr, std::size_t size, std::size_t);
};
static_assert(Allocator<PageAllocator>);

}  // namespace strobe
