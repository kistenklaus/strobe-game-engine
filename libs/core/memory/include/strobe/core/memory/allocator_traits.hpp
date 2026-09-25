#pragma once

#include <cassert>
#include <concepts>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace strobe {
/**
 * \ingroup core
 * \brief Requires the basic allocate and deallocate operations.
 * \code{.cpp}
 * template<typename A>
 * concept Allocator;
 * \endcode
 */
template <typename A>
concept Allocator =
    requires(A &a, std::size_t size, std::size_t align, void *ptr) {
      { a.allocate(size, align) } -> std::same_as<void *>;
      { a.deallocate(ptr, size, align) } -> std::same_as<void>;
    };

/**
 * \ingroup core
 * \brief Identifies allocators that can resize an allocation in place.
 * \code{.cpp}
 * template<typename A>
 * concept ReAllocator;
 * \endcode
 */
template <typename A>
concept ReAllocator =
    Allocator<A> && requires(A a, void *ptr, std::size_t oldSize,
                             std::size_t newSize, std::size_t align) {
      { a.reallocate(ptr, oldSize, newSize, align) } -> std::same_as<void *>;
    };

/**
 * \ingroup core
 * \brief Identifies allocators that can report an expanded allocation size.
 * \code{.cpp}
 * template<typename A>
 * concept OverAllocator;
 * \endcode
 */
template <typename A>
concept OverAllocator =
    Allocator<A> && requires(A a, std::size_t size, std::size_t align) {
      {
        a.allocate_at_least(size, align)
      } -> std::same_as<std::pair<void *, std::size_t>>;
    };

/**
 * \ingroup core
 * \brief Identifies allocators that can test pointer ownership.
 * \code{.cpp}
 * template<typename A>
 * concept OwningAllocator;
 * \endcode
 */
template <typename A>
concept OwningAllocator = Allocator<A> && requires(const A a, void *ptr) {
  { a.owns(ptr) } -> std::same_as<bool>;
};

/**
 * \ingroup core
 * \brief Identifies allocators that can deallocate without size metadata.
 * \code{.cpp}
 * template<typename A>
 * concept SizeIndependentAllocator;
 * \endcode
 */
template <typename A>
concept SizeIndependentAllocator = Allocator<A> && requires(A a, void *ptr) {
  { a.deallocate(ptr) };
};

/**
 * \ingroup core
 * \brief Identifies equality-comparable allocators.
 * \code{.cpp}
 * template<typename A>
 * concept ComparableAllocator;
 * \endcode
 */
template <typename A>
concept ComparableAllocator = Allocator<A> && std::equality_comparable<A>;

namespace details {
template <typename A>
concept DeclaresStateless = requires {
  { A::is_stateless } -> std::convertible_to<bool>;
};
} // namespace details

/**
 * \ingroup core
 * \brief Identifies allocators with no per-instance state.
 * \code{.cpp}
 * template<typename A>
 * concept StatelessAllocator;
 * \endcode
 *
 * Empty allocator types have no non-static data members. An allocator may
 * also explicitly provide a true static `is_stateless` property. Stateless
 * allocators must be default-initializable so callers can create an instance
 * on demand.
 */
template <typename A>
concept StatelessAllocator =
    Allocator<A> && std::default_initializable<A> &&
    (std::is_empty_v<A> ||
     (details::DeclaresStateless<A> && A::is_stateless));

/**
 * \ingroup core
 * \brief Provides common operations and propagation properties for an allocator.
 * \code{.cpp}
 * template<Allocator A>
 * struct AllocatorTraits;
 * \endcode
 *
 * Optional operations are available only when the corresponding allocator
 * concept is satisfied.
 */
template <Allocator A> struct AllocatorTraits {
  /**
   * \brief Pointer type for an allocated object.
   * \code{.cpp}
   * template<typename T>
   * using pointer = T*;
   * \endcode
   */
  template <typename T> using pointer = T *;

  /**
   * \brief Const pointer type for an allocated object.
   * \code{.cpp}
   * template<typename T>
   * using const_pointer = const T*;
   * \endcode
   */
  template <typename T> using const_pointer = const T *;

  /**
   * \brief Allocates raw storage.
   * \code{.cpp}
   * static void* allocate(A& a, std::size_t size,
   *                       std::size_t align);
   * template<typename T>
   * static T* allocate(A& a, std::size_t n = 1);
   * \endcode
   *
   * \param a Allocator instance to use.
   * \param size Number of bytes to allocate.
   * \param align Required alignment.
   * \param n Number of objects to allocate.
   * \return Pointer to allocated storage.
   */
  static inline void *allocate(A &a, std::size_t size, std::size_t align) {
    return a.allocate(size, align);
  }

  template <typename T> static inline T *allocate(A &a, std::size_t n = 1) {
    return static_cast<T *>(allocate(a, n * sizeof(T), alignof(T)));
  }

  /**
   * \brief Releases raw storage.
   * \code{.cpp}
   * static void deallocate(A& a, void* ptr,
   *                        std::size_t size, std::size_t align);
   * template<typename T>
   * static void deallocate(A& a, T* ptr, std::size_t n = 1);
   * \endcode
   *
   * \param a Allocator instance to use.
   * \param ptr Storage returned by allocate.
   * \param size Number of bytes originally allocated.
   * \param align Alignment used for the allocation.
   * \param n Number of objects represented by the storage.
   * \attention 1. The pointer, size, alignment, and n must describe a
   * prior allocation from \p a.
   */
  static inline void deallocate(A &a, void *ptr, std::size_t size,
                                std::size_t align) {
    a.deallocate(ptr, size, align);
  }

  template <typename T>
  static inline void deallocate(A &a, T *ptr, std::size_t n = 1) {
    deallocate(a, ptr, n * sizeof(T), alignof(T));
  }

  /**
   * \brief Releases storage without requiring its size.
   * \code{.cpp}
   * static void size_independent_deallocate(A& a, void* ptr);
   * \endcode
   *
   * \param a Allocator instance to use.
   * \param ptr Storage to release.
   * \attention 1. A must satisfy SizeIndependentAllocator.
   */
  static inline void size_independent_deallocate(A &a, void *ptr)
    requires SizeIndependentAllocator<A>
  {
    return a.deallocate(ptr);
  }

  /**
   * \brief Reports whether an allocator owns a pointer.
   * \code{.cpp}
   * static bool owns(A& a, void* ptr);
   * \endcode
   *
   * \param a Allocator instance to query.
   * \param ptr Pointer to test.
   * \return Whether \p ptr belongs to \p a.
   * \attention 1. A must satisfy OwningAllocator.
   */
  static bool owns(A &a, void *ptr)
    requires OwningAllocator<A>
  {
    return a.owns(ptr);
  }

  /**
   * \brief Selects an allocator for container copy construction.
   * \code{.cpp}
   * static A select_on_container_copy_construction(const A& a);
   * \endcode
   *
   * \param a Source allocator.
   * \return The allocator selected for the copied container.
   */
  static inline A select_on_container_copy_construction(const A &a) {
    if constexpr (requires { a.select_on_container_copy_construction(); }) {
      return a.select_on_container_copy_construction();
    } else {
      return a;
    }
  }

  /**
   * \brief Allocates storage and reports the actual available amount.
   * \code{.cpp}
   * static std::pair<void*, std::size_t>
   * allocate_at_least(A& a, std::size_t size, std::size_t align);
   * template<typename T>
   * static std::pair<T*, std::size_t>
   * allocate_at_least(A& a, std::size_t n);
   * \endcode
   *
   * \param a Allocator instance to use.
   * \param size Minimum number of bytes.
   * \param align Required alignment.
   * \param n Minimum number of objects.
   * \return A pointer and the number of bytes or objects available.
   */
  static inline std::pair<void *, std::size_t>
  allocate_at_least(A &a, std::size_t size, std::size_t align) {
    if constexpr (OverAllocator<A>) {
      return a.allocate_at_least(size, align);
    } else {
      return {
          allocate(a, size, align),
          size,
      };
    }
  }

  template <typename T>
  static inline std::pair<T *, std::size_t> allocate_at_least(A &a,
                                                              std::size_t n) {
    auto [ptr, bytes] = allocate_at_least(a, n * sizeof(T), alignof(T));

    assert(bytes >= n * sizeof(T));
    assert(bytes % sizeof(T) == 0);

    return {
        static_cast<T *>(ptr),
        bytes / sizeof(T),
    };
  }

private:
  // --- propagate_on_container_copy_assignment (POCCA) ---
  template <typename U, typename = void> struct has_pocca : std::false_type {};

  template <typename U>
  struct has_pocca<
      U, std::void_t<decltype(U::propagate_on_container_copy_assignment)>>
      : std::is_convertible<decltype(U::propagate_on_container_copy_assignment),
                            bool> {};

  template <typename U> static constexpr bool pocca_value() noexcept {
    if constexpr (has_pocca<U>::value) {
      return static_cast<bool>(U::propagate_on_container_copy_assignment);
    } else {
      return true;
    }
  }

  // --- propagate_on_container_move_assignment (POMCA) ---
  template <typename U, typename = void> struct has_pomca : std::false_type {};

  template <typename U>
  struct has_pomca<
      U, std::void_t<decltype(U::propagate_on_container_move_assignment)>>
      : std::is_convertible<decltype(U::propagate_on_container_move_assignment),
                            bool> {};

  template <typename U> static constexpr bool pomca_value() noexcept {
    if constexpr (has_pomca<U>::value) {
      return static_cast<bool>(U::propagate_on_container_move_assignment);
    } else {
      return true;
    }
  }

  // --- is_always_equal ---
  template <typename U, typename = void>
  struct has_is_always_equal : std::false_type {};

  template <typename U>
  struct has_is_always_equal<U, std::void_t<decltype(U::is_always_equal)>>
      : std::is_convertible<decltype(U::is_always_equal), bool> {};

  template <typename U> static constexpr bool is_always_equal_value() noexcept {
    if constexpr (has_is_always_equal<U>::value) {
      return static_cast<bool>(U::is_always_equal);
    } else {
      return false;
    }
  }

public:
  /**
   * \brief Reports copy-assignment propagation.
   * \code{.cpp}
   * static constexpr bool propagate_on_container_copy_assignment;
   * \endcode
   *
   * \return Whether a container copy assignment propagates its allocator.
   */
  static constexpr bool propagate_on_container_copy_assignment =
      pocca_value<A>();

  /**
   * \brief Reports move-assignment propagation.
   * \code{.cpp}
   * static constexpr bool propagate_on_container_move_assignment;
   * \endcode
   *
   * \return Whether a container move assignment propagates its allocator.
   */
  static constexpr bool propagate_on_container_move_assignment =
      pomca_value<A>();

  /**
   * \brief Reports whether all allocator instances compare equal.
   * \code{.cpp}
   * static constexpr bool is_always_equal;
   * \endcode
   *
   * \return Whether allocator instances can be treated as interchangeable.
   */
  static constexpr bool is_always_equal =
      !ComparableAllocator<A> && is_always_equal_value<A>();

  /**
   * \brief Reports whether the allocator is stateless.
   * \code{.cpp}
   * static constexpr bool is_stateless;
   * \endcode
   *
   * \return Whether `A` satisfies StatelessAllocator.
   */
  static constexpr bool is_stateless = StatelessAllocator<A>;
};

/**
 * \ingroup core
 * \brief Compares allocator instances when their types match.
 * \code{.cpp}
 * template<typename L, typename R>
 * constexpr bool alloc_equals(const L& lhs, const R& rhs) noexcept;
 * \endcode
 *
 * \param lhs Left allocator.
 * \param rhs Right allocator.
 * \return Whether the allocators are interchangeable.
 */
template <class L, class R>
constexpr bool alloc_equals(const L &lhs, const R &rhs) noexcept {
  /* ---------- same type ------------------------------------------------- */
  if constexpr (std::same_as<L, R>) {
    if constexpr (AllocatorTraits<L>::is_always_equal) {
      return true;
    } else if constexpr (ComparableAllocator<L>) {
      return lhs == rhs; // (1)
    };
  }
  return false;
}
} // namespace strobe
