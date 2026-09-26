#pragma once

#include "strobe/core/memory/allocator_traits.hpp"
#include "strobe/core/memory/mallocator.hpp"

#include <atomic>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <memory>
#include <utility>

namespace strobe {

/**
 * \ingroup core
 * \brief Atomically reference-counted ownership of one allocator-managed object.
 * \code{.cpp}
 * template<typename T, Allocator A = Mallocator>
 * class Arc;
 * \endcode
 *
 * An Arc stores one pointer to a control block. The control block owns the
 * object, allocator state, and a type-erased destruction function. Arc does
 * not support aliasing or polymorphic conversions.
 *
 * Different Arc instances sharing an object may be copied and destroyed
 * concurrently. Concurrent access to the same Arc instance still requires
 * external synchronization.
 *
 * \tparam T Object type owned by the arc.
 */
template <typename T, Allocator A = Mallocator> class Arc {
private:
  struct ControlBlockBase {
    using destroy_function = void (*)(ControlBlockBase *) noexcept;

    template <typename... Args>
    ControlBlockBase(destroy_function destroy, Args &&...args)
        : m_value(std::forward<Args>(args)...), m_destroy(destroy) {}
    ~ControlBlockBase() = default;

    T m_value;
    std::atomic<std::size_t> m_references = 1;
    destroy_function m_destroy;
  };

  struct ControlBlock final : ControlBlockBase {
    template <typename... Args>
    explicit ControlBlock(A allocator, Args &&...args)
        : ControlBlockBase(&destroy, std::forward<Args>(args)...),
          m_allocator(std::move(allocator)) {}

    static void destroy(ControlBlockBase *base) noexcept {
      auto *block = static_cast<ControlBlock *>(base);
      A allocator = std::move(block->m_allocator);
      std::destroy_at(block);
      AllocatorTraits<A>::template deallocate<ControlBlock>(allocator, block);
    }

    [[no_unique_address]] A m_allocator;
  };

public:
  /**
   * \brief Type of the owned object.
   * \code{.cpp}
   * using element_type = T;
   * \endcode
   */
  using element_type = T;

  /**
   * \brief Constructs and assigns arc ownership.
   * \code{.cpp}
   * Arc() noexcept;
   * ~Arc() noexcept;
   * Arc(const Arc& other) noexcept;
   * Arc& operator=(const Arc& other) noexcept;
   * Arc(Arc&& other) noexcept;
   * Arc& operator=(Arc&& other) noexcept;
   * \endcode
   *
   * \param other Arc whose ownership is copied or transferred.
   */
  Arc() noexcept = default;
  ~Arc() noexcept { reset(); }

  Arc(const Arc &other) noexcept : m_control_block(other.m_control_block) {
    retain();
  }

  Arc &operator=(const Arc &other) noexcept {
    if (this != &other) {
      Arc copy(other);
      swap(copy);
    }
    return *this;
  }

  Arc(Arc &&other) noexcept
      : m_control_block(std::exchange(other.m_control_block, nullptr)) {}

  Arc &operator=(Arc &&other) noexcept {
    if (this != &other) {
      reset();
      m_control_block = std::exchange(other.m_control_block, nullptr);
    }
    return *this;
  }

  template <typename U, typename... Args>
    requires std::constructible_from<U, Args...>
  friend Arc<U> make_arc(Args &&...args);

  template <typename U, Allocator B, typename... Args>
    requires std::constructible_from<U, Args...>
  friend Arc<U, B> alloc_arc(B allocator, Args &&...args);

  /**
   * \brief Returns the owned object.
   * \code{.cpp}
   * T& operator*() const noexcept;
   * T* operator->() const noexcept;
   * \endcode
   *
   * \return Reference or pointer to the shared object.
   * \attention 1. The arc must contain an object.
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
   * \brief Tests whether the arc owns an object.
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
   * \return Number of Arc instances sharing the object.
   */
  [[nodiscard]] std::size_t use_count() const noexcept {
    return m_control_block == nullptr
               ? 0
               : m_control_block->m_references.load(std::memory_order_relaxed);
  }

  /**
   * \brief Destroys and releases this arc's ownership.
   * \code{.cpp}
   * void reset() noexcept;
   * \endcode
   *
   * The object and its allocator resource are destroyed when this is the final
   * strong owner.
   */
  void reset() noexcept {
    ControlBlockBase *control_block =
        std::exchange(m_control_block, nullptr);
    if (control_block == nullptr ||
        control_block->m_references.fetch_sub(1, std::memory_order_acq_rel) !=
            1) {
      return;
    }
    control_block->m_destroy(control_block);
  }

  /**
   * \brief Exchanges ownership with another arc.
   * \code{.cpp}
   * void swap(Arc& other) noexcept;
   * \endcode
   *
   * \param other Arc to exchange with.
   */
  void swap(Arc &other) noexcept {
    std::swap(m_control_block, other.m_control_block);
  }

private:
  void retain() noexcept {
    if (m_control_block != nullptr) {
      m_control_block->m_references.fetch_add(1, std::memory_order_relaxed);
    }
  }

  ControlBlockBase *m_control_block = nullptr;
};

/**
 * \ingroup core
 * \brief Constructs an atomically shared object with the default allocator.
 * \code{.cpp}
 * template<typename T, typename... Args>
 * [[nodiscard]] Arc<T> make_arc(Args&&... args);
 * \endcode
 *
 * \param args Arguments forwarded to T's constructor.
 * \return An Arc owning the constructed object.
 * \attention 1. T must be constructible from \p args.
 */
template <typename T, typename... Args>
  requires std::constructible_from<T, Args...>
[[nodiscard]] Arc<T> make_arc(Args &&...args) {
  using arc_type = Arc<T>;
  using allocator_type = Mallocator;
  using control_block = typename arc_type::ControlBlock;
  allocator_type allocator{};
  control_block *pointer =
      AllocatorTraits<allocator_type>::template allocate<control_block>(allocator);
  assert(pointer != nullptr);
  std::construct_at(pointer, std::move(allocator),
                    std::forward<Args>(args)...);

  arc_type result;
  result.m_control_block = pointer;
  return result;
}

/**
 * \ingroup core
 * \brief Constructs an arc using an explicit allocator.
 * \code{.cpp}
 * template<typename T, Allocator A, typename... Args>
 * [[nodiscard]] Arc<T, A> alloc_arc(A allocator, Args&&... args);
 * \endcode
 *
 * \param allocator Allocator retained in the control block.
 * \param args Arguments forwarded to T's constructor.
 * \return An Arc owning the constructed object.
 * \attention 1. T must be constructible from \p args.
 */
template <typename T, Allocator A, typename... Args>
  requires std::constructible_from<T, Args...>
[[nodiscard]] Arc<T, A> alloc_arc(A allocator, Args &&...args) {
  using arc_type = Arc<T, A>;
  using control_block = typename arc_type::ControlBlock;
  control_block *pointer =
      AllocatorTraits<A>::template allocate<control_block>(allocator);
  assert(pointer != nullptr);
  std::construct_at(pointer, std::move(allocator),
                    std::forward<Args>(args)...);

  arc_type result;
  result.m_control_block = pointer;
  return result;
}

} // namespace strobe
