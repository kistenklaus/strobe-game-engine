#pragma once

#include "strobe/core/memory/allocator_traits.hpp"
#include "strobe/core/memory/mallocator.hpp"

#include <cassert>
#include <concepts>
#include <cstddef>
#include <fmt/format.h>
#include <limits>
#include <memory>
#include <type_traits>
#include <utility>

namespace strobe {

/**
 * \ingroup core
 * \brief Double-ended vector with inline storage and dynamic growth.
 * \code{.cpp}
 * template<typename T, size_t InlineCapacity,
 *          Allocator A = Mallocator>
 * class SmallVectorDeque;
 * \endcode
 */
template <typename T, size_t InlineCapacity, Allocator A = Mallocator>
class SmallVectorDeque {
public:
  using value_type = T;
  using allocator_type = std::remove_cvref_t<A>;
  using size_type = size_t;
  using difference_type = std::ptrdiff_t;
  using reference = T &;
  using const_reference = const T &;

private:
  using allocator_traits = AllocatorTraits<allocator_type>;

  struct Slot {
    alignas(T) std::byte storage[sizeof(T)];
  };

  static constexpr size_type inline_storage_count =
      InlineCapacity == 0 ? 1 : InlineCapacity;

  [[nodiscard]] static T *pointer(Slot *slots, size_type index) noexcept {
    return reinterpret_cast<T *>(slots[index].storage);
  }

  [[nodiscard]] static const T *pointer(const Slot *slots,
                                        size_type index) noexcept {
    return reinterpret_cast<const T *>(slots[index].storage);
  }

  [[nodiscard]] Slot *slots() noexcept {
    return m_heap != nullptr ? m_heap : m_inline;
  }

  [[nodiscard]] const Slot *slots() const noexcept {
    return m_heap != nullptr ? m_heap : m_inline;
  }

  [[nodiscard]] size_type physical_index(size_type index) const noexcept {
    const size_type remaining = capacity() - m_begin;
    return index < remaining ? m_begin + index : index - remaining;
  }

  [[nodiscard]] T *element(size_type index) noexcept {
    return pointer(slots(), physical_index(index));
  }

  [[nodiscard]] const T *element(size_type index) const noexcept {
    return pointer(slots(), physical_index(index));
  }

  [[nodiscard]] size_type increment(size_type index) const noexcept {
    return index + 1 == capacity() ? 0 : index + 1;
  }

  [[nodiscard]] size_type decrement(size_type index) const noexcept {
    return index == 0 ? capacity() - 1 : index - 1;
  }

  void release_heap() noexcept {
    if (m_heap != nullptr) {
      allocator_traits::template deallocate<Slot>(
          m_allocator, m_heap, m_heap_capacity);
      m_heap = nullptr;
      m_heap_capacity = 0;
    }
  }

  void grow() {
    const size_type current = capacity();
    assert(current <= std::numeric_limits<size_type>::max() / 2);
    reserve(current == 0 ? 1 : current * 2);
  }

  void move_from(SmallVectorDeque &other) {
    if (other.m_heap != nullptr) {
      m_heap = std::exchange(other.m_heap, nullptr);
      m_heap_capacity = std::exchange(other.m_heap_capacity, 0);
      m_size = std::exchange(other.m_size, 0);
      m_begin = std::exchange(other.m_begin, 0);
      return;
    }

    for (size_type i = 0; i < other.m_size; ++i) {
      std::construct_at(pointer(m_inline, i), std::move(other[i]));
      ++m_size;
    }
    other.clear();
  }

public:
  /**
   * \brief Constructs, copies, moves, assigns, or destroys a deque.
   * \code{.cpp}
   * explicit SmallVectorDeque(const allocator_type& alloc = {});
   * explicit SmallVectorDeque(size_type capacity,
   *                           const allocator_type& alloc = {});
   * SmallVectorDeque(const SmallVectorDeque& other);
   * SmallVectorDeque(SmallVectorDeque&& other);
   * SmallVectorDeque& operator=(const SmallVectorDeque& other);
   * SmallVectorDeque& operator=(SmallVectorDeque&& other);
   * ~SmallVectorDeque() noexcept;
   * \endcode
   * \param capacity Initial capacity.
   * \param alloc Allocator to use.
   * \param other Deque to copy or move from.
   *
   * Moved-from deques are empty.
   */
  explicit SmallVectorDeque(const allocator_type &alloc = {})
      : m_allocator(alloc) {}

  explicit SmallVectorDeque(size_type capacity,
                            const allocator_type &alloc = {})
      : m_allocator(alloc) {
    reserve(capacity);
  }

  SmallVectorDeque(const SmallVectorDeque &other)
    requires std::copy_constructible<T>
      : m_allocator(other.m_allocator) {
    reserve(other.m_size);
    for (size_type i = 0; i < other.m_size; ++i) {
      std::construct_at(element(i), other[i]);
      ++m_size;
    }
  }

  SmallVectorDeque(SmallVectorDeque &&other) noexcept(
      std::is_nothrow_move_constructible_v<T> &&
      std::is_nothrow_move_constructible_v<allocator_type>)
      : m_allocator(std::move(other.m_allocator)) {
    move_from(other);
  }

  SmallVectorDeque &operator=(const SmallVectorDeque &other)
    requires std::copy_constructible<T> &&
             std::is_copy_assignable_v<allocator_type>
  {
    if (this != &other) {
      clear();
      release_heap();
      m_allocator = other.m_allocator;
      reserve(other.m_size);
      for (size_type i = 0; i < other.m_size; ++i) {
        std::construct_at(element(i), other[i]);
        ++m_size;
      }
    }
    return *this;
  }

  SmallVectorDeque &operator=(SmallVectorDeque &&other) noexcept(
      std::is_nothrow_move_constructible_v<T> &&
      std::is_nothrow_move_assignable_v<allocator_type>)
    requires std::is_move_assignable_v<allocator_type>
  {
    if (this != &other) {
      clear();
      release_heap();
      m_allocator = std::move(other.m_allocator);
      move_from(other);
    }
    return *this;
  }

  ~SmallVectorDeque() noexcept {
    clear();
    release_heap();
  }

  /**
   * \brief Checks whether the deque is empty.
   * \code{.cpp}
   * bool empty() const noexcept;
   * \endcode
   * \return Whether size() is zero.
   */
  [[nodiscard]] bool empty() const noexcept { return m_size == 0; }
  /**
   * \brief Checks whether the deque is full.
   * \code{.cpp}
   * bool full() const noexcept;
   * \endcode
   * \return Whether size() equals capacity().
   */
  [[nodiscard]] bool full() const noexcept { return m_size == capacity(); }
  /**
   * \brief Returns the number of elements.
   * \code{.cpp}
   * size_type size() const noexcept;
   * \endcode
   * \return Number of elements.
   */
  [[nodiscard]] size_type size() const noexcept { return m_size; }

  /**
   * \brief Returns the element capacity.
   * \code{.cpp}
   * size_type capacity() const noexcept;
   * \endcode
   * \return Number of elements that fit without allocation.
   */
  [[nodiscard]] size_type capacity() const noexcept {
    return m_heap != nullptr ? m_heap_capacity : InlineCapacity;
  }

  /**
   * \brief Reserves capacity for elements.
   * \code{.cpp}
   * void reserve(size_type new_capacity);
   * \endcode
   * \param new_capacity Desired element capacity.
   * \attention 1. Reserving may invalidate references and pointers.
   */
  void reserve(size_type new_capacity) {
    if (new_capacity <= capacity()) {
      return;
    }

    Slot *replacement = allocator_traits::template allocate<Slot>(
        m_allocator, new_capacity);
    assert(replacement != nullptr);

    for (size_type i = 0; i < m_size; ++i) {
      std::construct_at(pointer(replacement, i), std::move((*this)[i]));
    }

    const size_type old_size = m_size;
    clear();
    release_heap();

    m_heap = replacement;
    m_heap_capacity = new_capacity;
    m_size = old_size;
  }

  /**
   * \brief Inserts an element at the back.
   * \code{.cpp}
   * template<typename U>
   * void push_back(U&& value);
   * \endcode
   * \param value Element to insert.
   */
  template <typename U>
    requires std::constructible_from<T, U &&>
  void push_back(U &&value) {
    emplace_back(std::forward<U>(value));
  }

  /**
   * \brief Constructs an element at the back.
   * \code{.cpp}
   * template<typename... Args>
   * T& emplace_back(Args&&... args);
   * \endcode
   * \param args Arguments forwarded to the element constructor.
   * \return Reference to the new element.
   */
  template <typename... Args> T &emplace_back(Args &&...args) {
    if (full()) {
      T temporary(std::forward<Args>(args)...);
      grow();
      std::construct_at(element(m_size), std::move(temporary));
    } else {
      std::construct_at(element(m_size), std::forward<Args>(args)...);
    }

    ++m_size;
    return back();
  }

  /**
   * \brief Inserts an element at the front.
   * \code{.cpp}
   * template<typename U>
   * void push_front(U&& value);
   * \endcode
   * \param value Element to insert.
   */
  template <typename U>
    requires std::constructible_from<T, U &&>
  void push_front(U &&value) {
    emplace_front(std::forward<U>(value));
  }

  /**
   * \brief Constructs an element at the front.
   * \code{.cpp}
   * template<typename... Args>
   * T& emplace_front(Args&&... args);
   * \endcode
   * \param args Arguments forwarded to the element constructor.
   * \return Reference to the new element.
   */
  template <typename... Args> T &emplace_front(Args &&...args) {
    if (full()) {
      T temporary(std::forward<Args>(args)...);
      grow();
      m_begin = decrement(m_begin);
      std::construct_at(pointer(slots(), m_begin), std::move(temporary));
    } else {
      m_begin = decrement(m_begin);
      std::construct_at(pointer(slots(), m_begin),
                        std::forward<Args>(args)...);
    }

    ++m_size;
    return front();
  }

  /**
   * \brief Removes the last element.
   * \code{.cpp}
   * void pop_back() noexcept;
   * \endcode
   * \attention 1. The deque must not be empty.
   */
  void pop_back() noexcept {
    assert(!empty());
    std::destroy_at(element(m_size - 1));
    --m_size;
    if (empty()) {
      m_begin = 0;
    }
  }

  /**
   * \brief Removes the first element.
   * \code{.cpp}
   * void pop_front() noexcept;
   * \endcode
   * \attention 1. The deque must not be empty.
   */
  void pop_front() noexcept {
    assert(!empty());
    std::destroy_at(pointer(slots(), m_begin));
    m_begin = increment(m_begin);
    --m_size;
    if (empty()) {
      m_begin = 0;
    }
  }

  /**
   * \brief Returns the first element.
   * \code{.cpp}
   * reference front() noexcept;
   * const_reference front() const noexcept;
   * \endcode
   * \return Reference to the first element.
   * \attention 1. The deque must not be empty.
   */
  [[nodiscard]] T &front() noexcept {
    assert(!empty());
    return *std::launder(pointer(slots(), m_begin));
  }

  [[nodiscard]] const T &front() const noexcept {
    assert(!empty());
    return *std::launder(pointer(slots(), m_begin));
  }

  /**
   * \brief Returns the last element.
   * \code{.cpp}
   * reference back() noexcept;
   * const_reference back() const noexcept;
   * \endcode
   * \return Reference to the last element.
   * \attention 1. The deque must not be empty.
   */
  [[nodiscard]] T &back() noexcept {
    assert(!empty());
    return (*this)[m_size - 1];
  }

  [[nodiscard]] const T &back() const noexcept {
    assert(!empty());
    return (*this)[m_size - 1];
  }

  /**
   * \brief Accesses an element by logical index.
   * \code{.cpp}
   * reference operator[](size_type index) noexcept;
   * const_reference operator[](size_type index) const noexcept;
   * \endcode
   * \param index Element index.
   * \return Reference to the requested element.
   * \attention 1. \p index must be less than size().
   */
  [[nodiscard]] T &operator[](size_type index) noexcept {
    assert(index < m_size);
    return *std::launder(element(index));
  }

  [[nodiscard]] const T &operator[](size_type index) const noexcept {
    assert(index < m_size);
    return *std::launder(element(index));
  }

  /**
   * \brief Removes all elements.
   * \code{.cpp}
   * void clear() noexcept;
   * \endcode
   */
  void clear() noexcept {
    for (size_type i = 0; i < m_size; ++i) {
      std::destroy_at(element(i));
    }
    m_size = 0;
    m_begin = 0;
  }

private:
  Slot m_inline[inline_storage_count];
  Slot *m_heap = nullptr;
  size_type m_heap_capacity = 0;
  size_type m_size = 0;
  size_type m_begin = 0;
  [[no_unique_address]] allocator_type m_allocator;
};

} // namespace strobe

namespace fmt {

/**
 * \brief Formats a deque in logical element order.
 * \code{.cpp}
 * template<typename T, size_t InlineCapacity, strobe::Allocator A>
 * struct formatter<strobe::SmallVectorDeque<T, InlineCapacity, A>>;
 * \endcode
 */
template <typename T, size_t InlineCapacity, strobe::Allocator A>
struct formatter<strobe::SmallVectorDeque<T, InlineCapacity, A>> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }

  template <typename FormatContext>
  auto format(const strobe::SmallVectorDeque<T, InlineCapacity, A> &value,
              FormatContext &ctx) const {
    auto out = ctx.out();
    *out++ = '[';
    for (std::size_t i = 0; i < value.size(); ++i) {
      if (i != 0) {
        *out++ = ',';
        *out++ = ' ';
      }
      out = fmt::format_to(out, "{}", value[i]);
    }
    *out++ = ']';
    return out;
  }
};

} // namespace fmt
