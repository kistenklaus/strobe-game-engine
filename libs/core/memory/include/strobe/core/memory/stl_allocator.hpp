#pragma once

#include "strobe/core/memory/allocator_traits.hpp"

#include <concepts>
#include <cstddef>
#include <limits>
#include <new>
#include <type_traits>
#include <utility>

namespace strobe {

/**
 * \ingroup core
 * \brief Adapts a strobe allocator to the standard allocator interface.
 * \code{.cpp}
 * template<typename T, Allocator A>
 * class StlAllocator;
 * \endcode
 *
 * The adapter translates object counts into byte sizes and supplies the
 * alignment required by \p T. It can therefore be used with standard library
 * containers and \c std::allocator_traits.
 *
 * \tparam T Object type allocated by this adapter.
 * \tparam A Underlying strobe allocator type.
 *
 * \attention 1. Container swap is declared to propagate the allocator. The
 * underlying allocator must therefore support the swap semantics required by
 * the standard container using this adapter.
 */
template <typename T, Allocator A> class StlAllocator {
public:
  /**
   * \brief Type of object allocated by this adapter.
   * \code{.cpp}
   * using value_type = T;
   * \endcode
   */
  using value_type = T;

  /**
   * \brief Unsigned type used for allocation sizes.
   * \code{.cpp}
   * using size_type = std::size_t;
   * \endcode
   */
  using size_type = std::size_t;

  /**
   * \brief Signed type used for pointer differences.
   * \code{.cpp}
   * using difference_type = std::ptrdiff_t;
   * \endcode
   */
  using difference_type = std::ptrdiff_t;

  /**
   * \brief Controls propagation during container copy assignment.
   * \code{.cpp}
   * using propagate_on_container_copy_assignment =
   *     std::bool_constant<AllocatorTraits<A>::propagate_on_container_copy_assignment>;
   * \endcode
   */
  using propagate_on_container_copy_assignment = std::bool_constant<
      AllocatorTraits<A>::propagate_on_container_copy_assignment>;

  /**
   * \brief Controls propagation during container move assignment.
   * \code{.cpp}
   * using propagate_on_container_move_assignment =
   *     std::bool_constant<AllocatorTraits<A>::propagate_on_container_move_assignment>;
   * \endcode
   */
  using propagate_on_container_move_assignment = std::bool_constant<
      AllocatorTraits<A>::propagate_on_container_move_assignment>;

  /**
   * \brief Indicates that container swap propagates this allocator.
   * \code{.cpp}
   * using propagate_on_container_swap = std::true_type;
   * \endcode
   */
  using propagate_on_container_swap = std::true_type;

  /**
   * \brief Indicates whether all adapter instances compare equal.
   * \code{.cpp}
   * using is_always_equal =
   *     std::bool_constant<AllocatorTraits<A>::is_always_equal>;
   * \endcode
   */
  using is_always_equal =
      std::bool_constant<AllocatorTraits<A>::is_always_equal>;

  /**
   * \brief Rebinds the adapter to another object type.
   * \code{.cpp}
   * template<typename U>
   * struct rebind {
   *   using other = StlAllocator<U, A>;
   * };
   * \endcode
   *
   * \tparam U Replacement object type.
   */
  template <typename U> struct rebind {
    using other = StlAllocator<U, A>;
  };

  /**
   * \brief Constructs an adapter with a default-constructed allocator.
   * \code{.cpp}
   * constexpr StlAllocator();
   * \endcode
   *
   * \attention 1. \p A must be default-initializable.
   */
  constexpr StlAllocator()
    requires std::default_initializable<A>
  = default;

  /**
   * \brief Constructs an adapter by copying an allocator.
   * \code{.cpp}
   * constexpr StlAllocator(const A& allocator);
   * \endcode
   *
   * \param allocator Allocator instance to retain.
   */
  constexpr StlAllocator(const A &allocator)
      : m_allocator(allocator) {}

  /**
   * \brief Constructs an adapter by moving an allocator.
   * \code{.cpp}
   * constexpr explicit StlAllocator(A&& allocator);
   * \endcode
   *
   * \param allocator Allocator instance to move into the adapter.
   */
  constexpr explicit StlAllocator(A &&allocator)
      : m_allocator(std::move(allocator)) {}

  /**
   * \brief Constructs an adapter for another object type.
   * \code{.cpp}
   * template<typename U>
   * constexpr StlAllocator(const StlAllocator<U, A>& other);
   * \endcode
   *
   * \param other Adapter whose underlying allocator is copied.
   */
  template <typename U>
  constexpr StlAllocator(const StlAllocator<U, A> &other)
      : m_allocator(other.allocator()) {}

  /**
   * \brief Allocates storage for objects of type \p T.
   * \code{.cpp}
   * [[nodiscard]] T* allocate(size_type count);
   * \endcode
   *
   * \param count Number of \p T objects represented by the storage.
   * \return Pointer to storage aligned for \p T.
   * \throws std::bad_array_new_length If the requested byte size overflows.
   * \throws std::bad_alloc If the underlying allocator returns no storage.
   */
  [[nodiscard]]
  T *allocate(size_type count) {
    if (count > std::numeric_limits<size_type>::max() / sizeof(T)) {
      throw std::bad_array_new_length{};
    }

    void *memory = AllocatorTraits<A>::allocate(m_allocator, count * sizeof(T),
                                                alignof(T));

    if (memory == nullptr) {
      throw std::bad_alloc{};
    }

    return static_cast<T *>(memory);
  }

  /**
   * \brief Releases storage previously returned by allocate.
   * \code{.cpp}
   * void deallocate(T* pointer, size_type count) noexcept;
   * \endcode
   *
   * \param pointer Storage returned by allocate.
   * \param count Object count originally passed to allocate.
   * \attention 1. \p pointer and \p count must describe a prior allocation
   * from this adapter.
   * \attention 2. An exception from the underlying allocator causes
   * termination because this function is \c noexcept.
   */
  void deallocate(T *pointer, size_type count) noexcept {
    AllocatorTraits<A>::deallocate(m_allocator, pointer, count * sizeof(T),
                                   alignof(T));
  }

  /**
   * \brief Accesses the underlying allocator.
   * \code{.cpp}
   * constexpr A& allocator() noexcept;
   * constexpr const A& allocator() const noexcept;
   * \endcode
   *
   * \return Reference to the underlying allocator.
   */
  [[nodiscard]]
  constexpr A &allocator() noexcept {
    return m_allocator;
  }

  [[nodiscard]]
  constexpr const A &allocator() const noexcept {
    return m_allocator;
  }

  /**
   * \brief Selects the allocator used for container copy construction.
   * \code{.cpp}
   * [[nodiscard]] constexpr StlAllocator
   * select_on_container_copy_construction() const;
   * \endcode
   *
   * \return Adapter containing the underlying allocator selected by
   * \c AllocatorTraits.
   */
  [[nodiscard]]
  constexpr StlAllocator select_on_container_copy_construction() const {
    return StlAllocator{
        AllocatorTraits<A>::select_on_container_copy_construction(m_allocator)};
  }

  /**
   * \brief Compares adapters that use the same underlying allocator type.
   * \code{.cpp}
   * template<typename U>
   * constexpr bool operator==(const StlAllocator<U, A>& other) const noexcept;
   * \endcode
   *
   * \param other Adapter to compare.
   * \return Whether both underlying allocators compare equal.
   */
  template <typename U>
  [[nodiscard]]
  constexpr bool operator==(const StlAllocator<U, A> &other) const noexcept {
    return alloc_equals(m_allocator, other.allocator());
  }

private:
  template <typename, Allocator> friend class StlAllocator;

  [[no_unique_address]] A m_allocator{};
};

} // namespace strobe
