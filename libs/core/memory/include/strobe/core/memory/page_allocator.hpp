#pragma once

#include <cstddef>
#include <cstdlib>
#include <utility>

#include "strobe/core/memory/allocator_traits.hpp"

namespace strobe {

/**
 * \ingroup core
 * \brief Allocates page-aligned virtual memory.
 * \code{.cpp}
 * class PageAllocator;
 * \endcode
 *
 * Allocation sizes are rounded up to the system page size. The allocator has
 * no per-instance state and separate instances can release one another's
 * allocations.
 */
class PageAllocator {
 public:
  /**
   * \brief Allocates page-rounded storage.
   * \code{.cpp}
   * [[nodiscard]] void* allocate(std::size_t size, std::size_t align);
   * \endcode
   *
   * \param size Minimum number of bytes to allocate.
   * \param align Alignment requested by the caller.
   * \return Page-aligned storage, or nullptr when \p size is zero or the
   * allocation fails.
   * \attention 1. \p align must be nonzero and divide the system page size.
   */
  [[nodiscard]] void* allocate(std::size_t size, std::size_t align);

  /**
   * \brief Allocates storage and reports the page-rounded size.
   * \code{.cpp}
   * [[nodiscard]] std::pair<void*, std::size_t>
   * allocate_at_least(std::size_t size, std::size_t align);
   * \endcode
   *
   * \param size Minimum number of bytes to allocate.
   * \param align Alignment requested by the caller.
   * \return The allocated pointer and the number of bytes reserved after page
   * rounding.
   * \attention 1. \p align must be nonzero and divide the system page size.
   */
  [[nodiscard]] std::pair<void*, std::size_t> allocate_at_least(
      std::size_t size, std::size_t align);

  /**
   * \brief Releases a page-rounded allocation.
   * \code{.cpp}
   * void deallocate(void* ptr, std::size_t size, std::size_t align);
   * \endcode
   *
   * \param ptr Storage returned by allocate or allocate_at_least.
   * \param size Original requested size in bytes.
   * \param align Alignment supplied for the allocation.
   * \attention 1. \p ptr must be null or refer to a live allocation from a
   * PageAllocator.
   * \attention 2. The size and alignment must describe that allocation.
   */
  void deallocate(void* ptr, std::size_t size, std::size_t align);
};
static_assert(Allocator<PageAllocator>);
static_assert(OverAllocator<PageAllocator>);
static_assert(StatelessAllocator<PageAllocator>);
static_assert(AllocatorTraits<PageAllocator>::is_stateless);

}  // namespace strobe
