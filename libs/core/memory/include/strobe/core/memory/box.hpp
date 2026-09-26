#pragma once

#include "strobe/core/memory/allocator_traits.hpp"

#include <cassert>
#include <concepts>
#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

namespace strobe {

/**
 * \ingroup core
 * \brief Nullable, move-only ownership of one allocator-allocated object.
 * \code{.cpp}
 * template<typename T, Allocator A>
 * class Box;
 * \endcode
 *
 * A `Box` owns one object and releases it through its stored allocator. It
 * cannot be copied, but ownership can be transferred by moving the box.
 *
 * \tparam T Object type owned by the box.
 * \tparam A Allocator used to allocate and release the object.
 */
template <typename T, Allocator A> class Box {
public:
  /**
   * \brief Type of the owned object.
   * \code{.cpp}
   * using element_type = T;
   * \endcode
   */
  using element_type = T;

  /**
   * \brief Type of the allocator used by the box.
   * \code{.cpp}
   * using allocator_type = A;
   * \endcode
   */
  using allocator_type = A;

  /**
   * \brief Constructs an empty box.
   * \code{.cpp}
   * Box()
   *   requires std::default_initializable<A>;
   * explicit Box(const A& allocator) noexcept;
   * ~Box() noexcept;
   * Box(const Box&) = delete;
   * Box& operator=(const Box&) = delete;
   * Box(Box&& other) noexcept;
   * Box& operator=(Box&& other) noexcept;
   * \endcode
   *
   * \param allocator Allocator retained for future allocation and release.
   */
  Box()
    requires std::default_initializable<A>
      : Box(A{}) {}

  explicit Box(const A &allocator) noexcept
      : m_allocator(allocator), m_pointer(nullptr) {}

  ~Box() noexcept { reset(); }

  Box(const Box &) = delete;
  Box &operator=(const Box &) = delete;

  Box(Box &&other) noexcept
      : m_allocator(std::move(other.m_allocator)),
        m_pointer(std::exchange(other.m_pointer, nullptr)) {}

  Box &operator=(Box &&other) noexcept {
    if (this == &other) {
      return *this;
    }
    reset();
    m_allocator = std::move(other.m_allocator);
    m_pointer = std::exchange(other.m_pointer, nullptr);
    return *this;
  }

  /**
   * \brief Returns the owned object.
   * \code{.cpp}
   * T& operator*() const noexcept;
   * T* operator->() const noexcept;
   * \endcode
   *
   * \return Reference or pointer to the owned object.
   * \attention 1. The box must contain an object.
   */
  T &operator*() const noexcept {
    assert(m_pointer != nullptr);
    return *m_pointer;
  }

  T *operator->() const noexcept {
    assert(m_pointer != nullptr);
    return m_pointer;
  }

  /**
   * \brief Returns the owned pointer without releasing ownership.
   * \code{.cpp}
   * [[nodiscard]] T* get() const noexcept;
   * \endcode
   *
   * \return The owned pointer, or nullptr when empty.
   */
  [[nodiscard]] T *get() const noexcept { return m_pointer; }

  /**
   * \brief Tests whether the box owns an object.
   * \code{.cpp}
   * explicit operator bool() const noexcept;
   * \endcode
   *
   * \return Whether get() is non-null.
   */
  explicit operator bool() const noexcept { return m_pointer != nullptr; }

  /**
   * \brief Destroys and releases the owned object.
   * \code{.cpp}
   * void reset() noexcept;
   * \endcode
   *
   * After reset(), the box is empty and retains its allocator.
   */
  void reset() noexcept {
    if (m_pointer == nullptr) {
      return;
    }
    std::destroy_at(m_pointer);
    AllocatorTraits<A>::template deallocate<T>(m_allocator, m_pointer);
    m_pointer = nullptr;
  }

  /**
   * \brief Releases ownership without destroying the object.
   * \code{.cpp}
   * [[nodiscard]] T* release() noexcept;
   * \endcode
   *
   * \return The previously owned pointer, or nullptr when empty.
   *
   * \attention 1. The caller becomes responsible for destroying and
   * deallocating the returned object with the box's allocator.
   */
  [[nodiscard]] T *release() noexcept {
    return std::exchange(m_pointer, nullptr);
  }

  /**
   * \brief Accesses the allocator retained by the box.
   * \code{.cpp}
   * A& allocator() noexcept;
   * const A& allocator() const noexcept;
   * \endcode
   *
   * \return The allocator used to release the object.
   */
  A &allocator() noexcept { return m_allocator; }
  const A &allocator() const noexcept { return m_allocator; }

private:
  template <typename U, Allocator B, typename... Args>
    requires std::constructible_from<U, Args...>
  friend Box<U, B> make_box(B allocator, Args &&...args);

  Box(A allocator, T *pointer)
      : m_allocator(std::move(allocator)), m_pointer(pointer) {}

  [[no_unique_address]] A m_allocator;
  T *m_pointer;
};

/**
 * \ingroup core
 * \brief Constructs an allocator-owned object in a box.
 * \code{.cpp}
 * template<typename T, Allocator A, typename... Args>
 * [[nodiscard]] Box<T, A> make_box(A allocator, Args&&... args);
 * \endcode
 *
 * \param allocator Allocator used for the object.
 * \param args Arguments forwarded to T's constructor.
 * \return A box owning the constructed object.
 * \attention 1. T must be constructible from \p args.
 */
template <typename T, Allocator A, typename... Args>
  requires std::constructible_from<T, Args...>
[[nodiscard]] Box<T, A> make_box(A allocator, Args &&...args) {
  T *pointer = AllocatorTraits<A>::template allocate<T>(allocator);
  assert(pointer != nullptr);
  std::construct_at(pointer, std::forward<Args>(args)...);
  return Box<T, A>(std::move(allocator), pointer);
}

} // namespace strobe
