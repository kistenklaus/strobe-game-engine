#pragma once

#include <cstddef>
#include <cstdlib>
#include<algorithm>

namespace strobe {

class Mallocator {
public:
  static constexpr bool is_always_equals = true;

  void *allocate(std::size_t size, std::size_t align) {
    align = std::max(align, alignof(std::max_align_t));
    size = (size + align - 1) & ~(align - 1);
    return std::aligned_alloc(align, size);
  }

  void deallocate(void *ptr, std::size_t size, std::size_t) { std::free(ptr); }

  void deallocate(void *ptr) { std::free(ptr); }
};

} // namespace strobe
