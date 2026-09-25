#pragma once

#include <cassert>
#include <cstddef>
#include <type_traits>
#include <utility>

#include "strobe/core/memory/allocator_traits.hpp"
#include "strobe/core/type_traits/fixed_string.hpp"
#include <fmt/printf.h>
#include <tracy/Tracy.hpp>

namespace strobe {

/**
 * \ingroup core
 * \brief Adds a compile-time name and tracing to an allocator.
 * \code{.cpp}
 * template<Allocator Alloc, fixed_string Name>
 * class NamedAllocator;
 * \endcode
 *
 * The wrapper forwards the capabilities provided by its upstream allocator.
 * Its statelessness is determined by the upstream allocator type.
 */
template <Allocator Alloc, fixed_string Name> class NamedAllocator {
public:
  /**
   * \brief Type of the wrapped allocator.
   * \code{.cpp}
   * using allocator = std::remove_cvref_t<Alloc>;
   * \endcode
   */
  using allocator = std::remove_cvref_t<Alloc>;

  /**
   * \brief Traits for the wrapped allocator.
   * \code{.cpp}
   * using allocator_traits = AllocatorTraits<allocator>;
   * \endcode
   */
  using allocator_traits = AllocatorTraits<allocator>;

  /**
   * \brief Compile-time name used for allocation tracing.
   * \code{.cpp}
   * static constexpr auto name = Name;
   * \endcode
   */
  static constexpr auto name = Name;

  /**
   * \brief Propagates whether the upstream allocator is always equal.
   * \code{.cpp}
   * static constexpr bool is_always_equal;
   * \endcode
   */
  static constexpr bool is_always_equal = allocator_traits::is_always_equal;

  /**
   * \brief Propagates whether the upstream allocator is stateless.
   * \code{.cpp}
   * static constexpr bool is_stateless;
   * \endcode
   */
  static constexpr bool is_stateless = allocator_traits::is_stateless;

  /**
   * \brief Constructs a named allocator.
   * \code{.cpp}
   * NamedAllocator(const allocator& alloc = {});
   * \endcode
   *
   * \param alloc Upstream allocator to copy.
   */
  NamedAllocator(const allocator &alloc = {}) : m_upstream(alloc) {}

  /**
   * \brief Allocates raw storage through the upstream allocator.
   * \code{.cpp}
   * void* allocate(std::size_t size, std::size_t alignment);
   * \endcode
   *
   * \param size Number of bytes to allocate.
   * \param alignment Required alignment.
   * \return Pointer to allocated storage.
   */
  void *allocate(std::size_t size, std::size_t alignment) {
    void *ptr = allocator_traits::allocate(m_upstream, size, alignment);
    assert(ptr != nullptr);
    TracyAllocN(ptr, size, name.data());
    return ptr;
  }

  /**
   * \brief Allocates storage and reports the actual available size.
   * \code{.cpp}
   * std::pair<void*, std::size_t>
   * allocate_at_least(std::size_t size, std::size_t alignment);
   * \endcode
   *
   * \param size Minimum number of bytes to allocate.
   * \param alignment Required alignment.
   * \return Allocated storage and its available byte count.
   * \attention 1. The upstream allocator must satisfy OverAllocator.
   */
  std::pair<void *, std::size_t> allocate_at_least(
      std::size_t size, std::size_t alignment)
    requires OverAllocator<allocator>
  {
    auto result =
        allocator_traits::allocate_at_least(m_upstream, size, alignment);
    assert(result.first != nullptr);
    TracyAllocN(result.first, result.second, name.data());
    return result;
  }

  /**
   * \brief Reallocates storage through the upstream allocator.
   * \code{.cpp}
   * void* reallocate(void* ptr, std::size_t old_size,
   *                  std::size_t new_size, std::size_t alignment);
   * \endcode
   *
   * \param ptr Existing allocation.
   * \param old_size Previous allocation size.
   * \param new_size Requested allocation size.
   * \param alignment Allocation alignment.
   * \return Pointer to the resized allocation.
   * \attention 1. The upstream allocator must satisfy ReAllocator.
   */
  void *reallocate(void *ptr, std::size_t old_size, std::size_t new_size,
                   std::size_t alignment)
    requires ReAllocator<allocator>
  {
    return m_upstream.reallocate(ptr, old_size, new_size, alignment);
  }

  /**
   * \brief Releases sized storage through the upstream allocator.
   * \code{.cpp}
   * void deallocate(void* ptr, std::size_t size, std::size_t alignment);
   * \endcode
   *
   * \param ptr Storage to release.
   * \param size Original allocation size.
   * \param alignment Allocation alignment.
   */
  void deallocate(void *ptr, std::size_t size, std::size_t alignment) {
    assert(ptr != nullptr);
    TracyFreeN(ptr, name.data());
    allocator_traits::deallocate(m_upstream, ptr, size, alignment);
  }

  /**
   * \brief Releases storage without requiring its size.
   * \code{.cpp}
   * void deallocate(void* ptr);
   * \endcode
   *
   * \param ptr Storage to release.
   * \attention 1. The upstream allocator must satisfy
   * SizeIndependentAllocator.
   */
  void deallocate(void *ptr)
    requires SizeIndependentAllocator<allocator>
  {
    assert(ptr != nullptr);
    TracyFreeN(ptr, name.data());
    allocator_traits::size_independent_deallocate(m_upstream, ptr);
  }

  /**
   * \brief Reports whether the upstream allocator owns a pointer.
   * \code{.cpp}
   * bool owns(void* ptr);
   * \endcode
   *
   * \param ptr Pointer to test.
   * \return Whether the upstream allocator owns \p ptr.
   * \attention 1. The upstream allocator must satisfy OwningAllocator.
   */
  bool owns(void *ptr)
    requires OwningAllocator<allocator>
  {
    return allocator_traits::owns(m_upstream, ptr);
  }

private:
  [[no_unique_address]] allocator m_upstream;
};

} // namespace strobe
