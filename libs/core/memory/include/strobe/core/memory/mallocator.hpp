#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#ifdef _MSC_VER
#include <malloc.h>
#endif
#include <tracy/Tracy.hpp>

#include "strobe/core/memory/allocator_traits.hpp"

namespace strobe {

class Mallocator {
public:
  static constexpr bool is_always_equal = true;

  void *allocate(std::size_t size, std::size_t align) noexcept {
    ZoneScopedN("Mallocator::allocate");
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
    deallocate(ptr);
  }

  void deallocate(void *ptr) {
    ZoneScopedN("Mallocator::deallocate");
    TracyFree(ptr);
#ifdef _MSC_VER
    _aligned_free(ptr);
#else
    std::free(ptr);
#endif
  }
};

} // namespace strobe

static_assert(strobe::Allocator<strobe::Mallocator>);
static_assert(strobe::StatelessAllocator<strobe::Mallocator>);
static_assert(strobe::AllocatorTraits<strobe::Mallocator>::is_always_equal);
