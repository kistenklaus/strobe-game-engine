#pragma once

#include "strobe/core/memory/allocator_traits.hpp"
#include "strobe/core/memory/mallocator.hpp"

#include <cassert>
#include <concepts>
#include <cstddef>
#include <memory>
#include <utility>

namespace strobe {

/**
 * \ingroup core
 * \brief Single-threaded reference-counted ownership of one object.
 * \code{.cpp}
 * template<typename T, Allocator A = Mallocator>
 * class Rc;
 * \endcode
 *
 * An Rc stores one pointer to a control block containing the object, allocator,
 * and strong-reference count. It does not support aliasing,
 * polymorphic conversions, or implicit conversion from Box.
 *
 * Copies and destruction of Rc instances sharing an object must be externally
 * synchronized. Use Arc when ownership operations must be thread-safe.
 *
 * \tparam T Object type owned by the Rc.
 * \tparam A Allocator used for the control block.
 */
template <typename T, Allocator A = Mallocator> class Rc {
private:
  struct ControlBlock {
    template <typename... Args>
    explicit ControlBlock(A allocator, Args &&...args)
        : m_value(std::forward<Args>(args)...), m_references(1),
          m_allocator(std::move(allocator)) {}

    T m_value;
    std::size_t m_references;
    [[no_unique_address]] A m_allocator;
  };

public:
  /**
   * \brief Type of the owned object and allocator.
   * \code{.cpp}
   * using element_type = T;
   * using allocator_type = A;
   * \endcode
   */
  using element_type = T;
  using allocator_type = A;

  /**
   * \brief Constructs and assigns Rc ownership.
   * \code{.cpp}
   * Rc() noexcept;
   * ~Rc() noexcept;
   * Rc(const Rc& other) noexcept;
   * Rc& operator=(const Rc& other) noexcept;
   * Rc(Rc&& other) noexcept;
   * Rc& operator=(Rc&& other) noexcept;
   * \endcode
   *
   * \param other Rc whose ownership is copied or transferred.
   */
  Rc() noexcept = default;

  ~Rc() noexcept { reset(); }

  Rc(const Rc &other) noexcept : m_control_block(other.m_control_block) {
    retain();
  }

  Rc &operator=(const Rc &other) noexcept {
    if (this != &other) {
      Rc copy(other);
      swap(copy);
    }
    return *this;
  }

  Rc(Rc &&other) noexcept
      : m_control_block(std::exchange(other.m_control_block, nullptr)) {}

  Rc &operator=(Rc &&other) noexcept {
    if (this != &other) {
      reset();
      m_control_block = std::exchange(other.m_control_block, nullptr);
    }
    return *this;
  }

  template <typename U, typename... Args>
    requires std::constructible_from<U, Args...>
  friend Rc<U> make_rc(Args &&...args);

  template <typename U, Allocator B, typename... Args>
    requires std::constructible_from<U, Args...>
  friend Rc<U, B> alloc_rc(B allocator, Args &&...args);

  /**
   * \brief Returns the owned object.
   * \code{.cpp}
   * T& operator*() const noexcept;
   * T* operator->() const noexcept;
   * \endcode
   *
   * \return Reference or pointer to the shared object.
   * \attention 1. The Rc must contain an object.
   */
  T &operator*() const noexcept {
    assert(m_control_block != nullptr);
    return m_control_block->m_value;
  }

  T *operator->() const noexcept {
    assert(m_control_block != nullptr);
    return &m_control_block->m_value;
  }

  /**
   * \brief Returns the shared object pointer.
   * \code{.cpp}
   * [[nodiscard]] T* get() const noexcept;
   * \endcode
   *
   * \return The shared object pointer, or nullptr when empty.
   */
  [[nodiscard]] T *get() const noexcept {
    return m_control_block == nullptr ? nullptr : &m_control_block->m_value;
  }

  /**
   * \brief Tests whether the Rc owns an object.
   * \code{.cpp}
   * explicit operator bool() const noexcept;
   * \endcode
   *
   * \return Whether get() is non-null.
   */
  explicit operator bool() const noexcept { return m_control_block != nullptr; }

  /**
   * \brief Returns the current strong-owner count.
   * \code{.cpp}
   * [[nodiscard]] std::size_t use_count() const noexcept;
   * \endcode
   *
   * \return Number of Rc instances sharing the object.
   */
  [[nodiscard]] std::size_t use_count() const noexcept {
    return m_control_block == nullptr
               ? 0
               : m_control_block->m_references;
  }

  /**
   * \brief Destroys and releases this Rc's ownership.
   * \code{.cpp}
   * void reset() noexcept;
   * \endcode
   *
   * The object is destroyed when this is the final strong owner.
   */
  void reset() noexcept {
    ControlBlock *control_block = std::exchange(m_control_block, nullptr);
    if (control_block == nullptr || --control_block->m_references != 0) {
      return;
    }

    A allocator = std::move(control_block->m_allocator);
    std::destroy_at(control_block);
    AllocatorTraits<A>::template deallocate<ControlBlock>(allocator,
                                                           control_block);
  }

  /**
   * \brief Exchanges ownership with another Rc.
   * \code{.cpp}
   * void swap(Rc& other) noexcept;
   * \endcode
   *
   * \param other Rc to exchange with.
   */
  void swap(Rc &other) noexcept {
    std::swap(m_control_block, other.m_control_block);
  }

private:
  void retain() noexcept {
    if (m_control_block != nullptr) {
      ++m_control_block->m_references;
    }
  }

  ControlBlock *m_control_block = nullptr;
};

/**
 * \ingroup core
 * \brief Constructs a reference-counted object.
 * \code{.cpp}
 * template<typename T, typename... Args>
 * [[nodiscard]] Rc<T> make_rc(Args&&... args);
 * \endcode
 *
 * \param args Arguments forwarded to T's constructor.
 * \return An Rc owning the constructed object.
 * \attention 1. T must be constructible from \p args.
 */
template <typename T, typename... Args>
  requires std::constructible_from<T, Args...>
[[nodiscard]] Rc<T> make_rc(Args &&...args) {
  using rc_type = Rc<T>;
  using allocator_type = Mallocator;
  using control_block = typename rc_type::ControlBlock;
  allocator_type allocator{};
  control_block *pointer =
      AllocatorTraits<allocator_type>::template allocate<control_block>(allocator);
  assert(pointer != nullptr);
  std::construct_at(pointer, std::move(allocator),
                    std::forward<Args>(args)...);

  rc_type result;
  result.m_control_block = pointer;
  return result;
}

/**
 * \ingroup core
 * \brief Constructs a reference-counted object using an explicit allocator.
 * \code{.cpp}
 * template<typename T, Allocator A, typename... Args>
 * [[nodiscard]] Rc<T, A> alloc_rc(A allocator, Args&&... args);
 * \endcode
 *
 * \param allocator Allocator used for the control block.
 * \param args Arguments forwarded to T's constructor.
 * \return An Rc owning the constructed object.
 * \attention 1. T must be constructible from \p args.
 */
template <typename T, Allocator A, typename... Args>
  requires std::constructible_from<T, Args...>
[[nodiscard]] Rc<T, A> alloc_rc(A allocator, Args &&...args) {
  using rc_type = Rc<T, A>;
  using control_block = typename rc_type::ControlBlock;
  control_block *pointer =
      AllocatorTraits<A>::template allocate<control_block>(allocator);
  assert(pointer != nullptr);
  std::construct_at(pointer, std::move(allocator),
                    std::forward<Args>(args)...);

  rc_type result;
  result.m_control_block = pointer;
  return result;
}

} // namespace strobe
