#pragma once

#include <cassert>
#include <cstddef>
#include <cstdlib>
#ifdef _MSC_VER
#include <malloc.h>
#endif
#include <tracy/Tracy.hpp>

#include "strobe/core/memory/allocator_traits.hpp"

namespace strobe {

/**
 * \ingroup core
 * \brief Stateless aligned allocator backed by the process heap.
 * \code{.cpp}
 * class Mallocator;
 * \endcode
 *
 * Allocations are aligned to the requested power-of-two alignment and are
 * released by the matching deallocation operation.
 */
class Mallocator {
public:
  /**
   * \brief Indicates that all Mallocator instances compare equal.
   * \ingroup core
   * \code{.cpp}
   * static constexpr bool is_always_equal = true;
   * \endcode
   */
  static constexpr bool is_always_equal = true;

  /**
   * \brief Allocates an aligned block of memory.
   * \ingroup core
   * \code{.cpp}
   * void* allocate(std::size_t size, std::size_t align) noexcept;
   * \endcode
   *
   * \param size Number of bytes to allocate.
   * \param align Power-of-two alignment in bytes.
   * \return Pointer to the allocated storage, or null if allocation fails.
   * \attention \p size must be nonzero and a multiple of \p align.
   */
  void *allocate(std::size_t size, std::size_t align) noexcept {
    ZoneScopedN("Mallocator::allocate");
    assert(size != 0);
    assert(align != 0 && (align & (align - 1)) == 0);
    assert(size % align == 0);
#ifdef _MSC_VER
    void *ptr = _aligned_malloc(size, align);
#else
    void *ptr = std::aligned_alloc(align, size);
#endif
    TracyAlloc(ptr, size);
    return ptr;
  }

  /**
   * \brief Releases an aligned allocation.
   * \ingroup core
   * \code{.cpp}
   * void deallocate(void* ptr, std::size_t size, std::size_t align) noexcept;
   * void deallocate(void* ptr) noexcept;
   * \endcode
   *
   * \param ptr Allocation returned by Mallocator.
   * \param size Original allocation size.
   * \param align Original allocation alignment.
   * \attention \p ptr must be null or returned by this allocator and must not
   * have been released already.
   */
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
