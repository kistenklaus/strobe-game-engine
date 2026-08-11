#pragma once

#include "strobe/core/memory/AllocatorTraits.hpp"

#include <concepts>
#include <cstddef>
#include <limits>
#include <new>
#include <string>
#include <type_traits>
#include <utility>

namespace strobe {

template <typename T, Allocator A> class StlAllocator {
public:
  using value_type = T;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;

  using propagate_on_container_copy_assignment = std::bool_constant<
      AllocatorTraits<A>::propagate_on_container_copy_assignment>;

  using propagate_on_container_move_assignment = std::bool_constant<
      AllocatorTraits<A>::propagate_on_container_move_assignment>;

  using propagate_on_container_swap = std::true_type;

  using is_always_equal =
      std::bool_constant<AllocatorTraits<A>::is_always_equal>;

  template <typename U> struct rebind {
    using other = StlAllocator<U, A>;
  };

  constexpr StlAllocator()
    requires std::default_initializable<A>
  = default;

  constexpr StlAllocator(const A &allocator)
      : m_allocator(allocator) {}

  constexpr explicit StlAllocator(A &&allocator)
      : m_allocator(std::move(allocator)) {}

  template <typename U>
  constexpr StlAllocator(const StlAllocator<U, A> &other)
      : m_allocator(other.allocator()) {}

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

  void deallocate(T *pointer, size_type count) noexcept {
    AllocatorTraits<A>::deallocate(m_allocator, pointer, count * sizeof(T),
                                   alignof(T));
  }

  [[nodiscard]]
  constexpr A &allocator() noexcept {
    return m_allocator;
  }

  [[nodiscard]]
  constexpr const A &allocator() const noexcept {
    return m_allocator;
  }

  [[nodiscard]]
  constexpr StlAllocator select_on_container_copy_construction() const {
    return StlAllocator{
        AllocatorTraits<A>::select_on_container_copy_construction(m_allocator)};
  }

  template <typename U>
  [[nodiscard]]
  constexpr bool operator==(const StlAllocator<U, A> &other) const noexcept {
    return alloc_equals(m_allocator, other.allocator());
  }

private:
  template <typename, Allocator> friend class StlAllocator;

  [[no_unique_address]]
  A m_allocator{};
};

} // namespace strobe
