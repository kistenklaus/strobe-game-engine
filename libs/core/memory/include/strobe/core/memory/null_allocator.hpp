#pragma once

#include <cstddef>

#include "strobe/core/memory/allocator_traits.hpp"

namespace strobe {

/**
 * \ingroup core
 * \brief Allocator that never provides storage.
 * \code{.cpp}
 * struct NullAllocator;
 * \endcode
 *
 * Allocation always returns `nullptr`; both deallocation overloads are
 * no-ops. The type is stateless and can be instantiated on demand.
 */
struct NullAllocator {
  /**
   * \brief Reports that all instances are interchangeable.
   * \code{.cpp}
   * static constexpr bool is_always_equal;
   * \endcode
   */
  static constexpr bool is_always_equal = true;

  /**
   * \brief Attempts to allocate storage.
   * \code{.cpp}
   * void* allocate(std::size_t size, std::size_t align) noexcept;
   * \endcode
   *
   * \param size Requested byte count.
   * \param align Requested alignment.
   * \return Always nullptr.
   */
  void *allocate(std::size_t size, std::size_t align) noexcept {
    (void)size;
    (void)align;
    return nullptr;
  }

  /**
   * \brief Releases sized storage.
   * \code{.cpp}
   * void deallocate(void* ptr, std::size_t size,
   *                 std::size_t align) noexcept;
   * \endcode
   *
   * \param ptr Pointer to release.
   * \param size Allocation size.
   * \param align Allocation alignment.
   *
   * This operation has no effect.
   */
  void deallocate(void *ptr, std::size_t size, std::size_t align) noexcept {
    (void)ptr;
    (void)size;
    (void)align;
  }

  /**
   * \brief Releases storage without requiring its size.
   * \code{.cpp}
   * void deallocate(void* ptr) noexcept;
   * \endcode
   *
   * \param ptr Pointer to release.
   *
   * This operation has no effect.
   */
  void deallocate(void *ptr) noexcept { (void)ptr; }
};

static_assert(Allocator<NullAllocator>);
static_assert(SizeIndependentAllocator<NullAllocator>);
static_assert(StatelessAllocator<NullAllocator>);
static_assert(AllocatorTraits<NullAllocator>::is_always_equal);

} // namespace strobe
