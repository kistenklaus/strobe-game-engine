#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <tracy/Tracy.hpp>

namespace strobe {

class Mallocator {
public:
  static constexpr bool is_always_equals = true;

  void *allocate(std::size_t size, std::size_t align) noexcept {
    align = std::max(align, alignof(std::max_align_t));
    size = (size + align - 1) & ~(align - 1);
#ifdef _MSC_VER
    void *ptr = _aligned_malloc(size, align);
#else
    void *ptr = std::aligned_alloc(align, size);
#endif
    TracyAlloc(ptr, size);
    return ptr;
  }

  void deallocate(void *ptr, std::size_t, std::size_t) noexcept {
    TracyFree(ptr);
    deallocate(ptr);
  }

  void deallocate(void *ptr) {
#ifdef _MSC_VER
    _aligned_free(ptr);
#else
    std::free(ptr);
#endif
  }
};



} // namespace strobe
