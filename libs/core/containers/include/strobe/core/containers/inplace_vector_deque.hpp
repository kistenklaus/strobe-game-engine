#pragma once

#include <cassert>
#include <cstddef>
#include <fmt/format.h>
#include <memory>
#include <type_traits>
#include <utility>

namespace strobe {

/**
 * \ingroup core
 * \brief Fixed-capacity double-ended vector with inline storage.
 * \code{.cpp}
 * template<typename T, size_t Capacity>
 * class InplaceVectorDeque;
 * \endcode
 *
 * Stores elements in a circular inline buffer and supports insertion and
 * removal at both ends.
 *
 * \attention 1. The deque cannot contain more than Capacity elements.
 * \attention 2. Element order is preserved by push and pop operations.
 * \attention 3. References and pointers remain valid until the referenced
 * element is removed or the deque is destroyed.
 */
template <typename T, size_t Capacity> class InplaceVectorDeque {
public:
  using value_type = T;
  using size_type = size_t;
  using difference_type = std::ptrdiff_t;
  using reference = T &;
  using const_reference = const T &;

  /**
   * \brief Constructs, copies, or moves a deque.
   * \code{.cpp}
   * InplaceVectorDeque() noexcept;
   * InplaceVectorDeque(const InplaceVectorDeque& other);
   * InplaceVectorDeque(InplaceVectorDeque&& other);
   * InplaceVectorDeque& operator=(const InplaceVectorDeque& other);
   * InplaceVectorDeque& operator=(InplaceVectorDeque&& other);
   * ~InplaceVectorDeque() noexcept;
   * \endcode
   *
   * \param other Deque to copy or move from.
   *
   * Moved-from deques are empty.
   */
  InplaceVectorDeque() noexcept = default;

  InplaceVectorDeque(const InplaceVectorDeque &other)
    requires std::is_copy_constructible_v<T>
  {
    for (size_type i = 0; i < other.m_size; ++i)
      emplace_back(other[i]);
  }

  InplaceVectorDeque(InplaceVectorDeque &&other) noexcept(
      std::is_nothrow_move_constructible_v<T>)
    requires std::is_move_constructible_v<T>
  {
    for (size_type i = 0; i < other.m_size; ++i)
      emplace_back(std::move(other[i]));

    other.clear();
  }

  InplaceVectorDeque &operator=(const InplaceVectorDeque &other)
    requires std::is_copy_constructible_v<T>
  {
    if (this != &other) {
      clear();

      for (size_type i = 0; i < other.m_size; ++i)
        emplace_back(other[i]);
    }

    return *this;
  }

  InplaceVectorDeque &operator=(InplaceVectorDeque &&other) noexcept(
      std::is_nothrow_move_constructible_v<T>)
    requires std::is_move_constructible_v<T>
  {
    if (this != &other) {
      clear();

      for (size_type i = 0; i < other.m_size; ++i)
        emplace_back(std::move(other[i]));

      other.clear();
    }

    return *this;
  }

  ~InplaceVectorDeque() noexcept { clear(); }

  /**
   * \brief Checks whether the deque is empty.
   * \code{.cpp}
   * bool empty() const noexcept;
   * \endcode
   *
   * \return Whether size() is zero.
   */
  [[nodiscard]] bool empty() const noexcept { return m_size == 0; }

  /**
   * \brief Checks whether the deque is full.
   * \code{.cpp}
   * bool full() const noexcept;
   * \endcode
   *
   * \return Whether size() equals capacity().
   */
  [[nodiscard]] bool full() const noexcept { return m_size == Capacity; }

  /**
   * \brief Returns the number of elements.
   * \code{.cpp}
   * size_type size() const noexcept;
   * \endcode
   *
   * \return Number of elements.
   */
  [[nodiscard]] size_type size() const noexcept { return m_size; }

  /**
   * \brief Returns the maximum number of elements.
   * \code{.cpp}
   * static constexpr size_type capacity() noexcept;
   * \endcode
   *
   * \return Maximum number of elements.
   */
  [[nodiscard]] static constexpr size_type capacity() noexcept {
    return Capacity;
  }

  /**
   * \brief Constructs an element at the back.
   * \code{.cpp}
   * template<typename... Args>
   * T& emplace_back(Args&&... args);
   * \endcode
   *
   * \param args Arguments forwarded to the element constructor.
   *
   * \return Reference to the newly constructed element.
   *
   * \attention 1. The deque must not be full.
   */
  template <typename... Args> T &emplace_back(Args &&...args) {
    assert(!full() && "InplaceVectorDeque is full");

    const size_type index = physical_index(m_size);
    T *value = std::construct_at(ptr(index), std::forward<Args>(args)...);
    ++m_size;
    return *value;
  }

  /**
   * \brief Constructs an element at the front.
   * \code{.cpp}
   * template<typename... Args>
   * T& emplace_front(Args&&... args);
   * \endcode
   *
   * \param args Arguments forwarded to the element constructor.
   * \return Reference to the newly constructed element.
   * \attention 1. The deque must not be full.
   */
  template <typename... Args> T &emplace_front(Args &&...args) {
    assert(!full() && "InplaceVectorDeque is full");

    const size_type index = decrement(m_begin);
    T *value = std::construct_at(ptr(index), std::forward<Args>(args)...);
    m_begin = index;
    ++m_size;
    return *value;
  }

  /**
   * \brief Inserts an element at the back.
   * \code{.cpp}
   * void push_back(const T& value);
   * void push_back(T&& value);
   * \endcode
   *
   * \param value Element to insert.
   *
   * \attention 1. The deque must not be full.
   */
  void push_back(const T &value) { emplace_back(value); }
  void push_back(T &&value) { emplace_back(std::move(value)); }

  /**
   * \brief Inserts an element at the front.
   * \code{.cpp}
   * void push_front(const T& value);
   * void push_front(T&& value);
   * \endcode
   *
   * \param value Element to insert.
   * \attention 1. The deque must not be full.
   */
  void push_front(const T &value) { emplace_front(value); }
  void push_front(T &&value) { emplace_front(std::move(value)); }

  /**
   * \brief Removes the last element.
   * \code{.cpp}
   * void pop_back() noexcept;
   * \endcode
   *
   * \attention 1. The deque must not be empty.
   */
  void pop_back() noexcept {
    assert(!empty() && "InplaceVectorDeque is empty");

    std::destroy_at(ptr(physical_index(m_size - 1)));
    --m_size;

    if (empty())
      m_begin = 0;
  }

  /**
   * \brief Removes the first element.
   * \code{.cpp}
   * void pop_front() noexcept;
   * \endcode
   *
   * \attention 1. The deque must not be empty.
   */
  void pop_front() noexcept {
    assert(!empty() && "InplaceVectorDeque is empty");

    std::destroy_at(ptr(m_begin));
    m_begin = increment(m_begin);
    --m_size;

    if (empty())
      m_begin = 0;
  }

  /**
   * \brief Returns the first element.
   * \code{.cpp}
   * reference front() noexcept;
   * const_reference front() const noexcept;
   * \endcode
   *
   * \return Reference to the requested element.
   *
   * \attention 1. The deque must not be empty.
   */
  [[nodiscard]] T &front() noexcept {
    assert(!empty() && "InplaceVectorDeque is empty");
    return *std::launder(ptr(m_begin));
  }

  [[nodiscard]] const T &front() const noexcept {
    assert(!empty() && "InplaceVectorDeque is empty");
    return *std::launder(ptr(m_begin));
  }

  /**
   * \brief Returns the last element.
   * \code{.cpp}
   * reference back() noexcept;
   * const_reference back() const noexcept;
   * \endcode
   *
   * \return Reference to the last element.
   * \attention 1. The deque must not be empty.
   */
  [[nodiscard]] T &back() noexcept {
    assert(!empty() && "InplaceVectorDeque is empty");
    return (*this)[m_size - 1];
  }

  [[nodiscard]] const T &back() const noexcept {
    assert(!empty() && "InplaceVectorDeque is empty");
    return (*this)[m_size - 1];
  }

  /**
   * \brief Accesses an element by logical index.
   * \code{.cpp}
   * reference operator[](size_type index) noexcept;
   * const_reference operator[](size_type index) const noexcept;
   * \endcode
   *
   * \param index Element index.
   *
   * \return Reference to the requested element.
   *
   * \attention 1. \p index must be less than size().
   */
  [[nodiscard]] T &operator[](size_type index) noexcept {
    assert(index < m_size);
    return *std::launder(ptr(physical_index(index)));
  }

  [[nodiscard]] const T &operator[](size_type index) const noexcept {
    assert(index < m_size);
    return *std::launder(ptr(physical_index(index)));
  }

  /**
   * \brief Removes all elements.
   * \code{.cpp}
   * void clear() noexcept;
   * \endcode
   */
  void clear() noexcept {
    for (size_type i = 0; i < m_size; ++i)
      std::destroy_at(ptr(physical_index(i)));

    m_size = 0;
    m_begin = 0;
  }

private:
  struct Slot {
    alignas(T) std::byte storage[sizeof(T)];
  };

  static constexpr size_type storage_capacity = Capacity == 0 ? 1 : Capacity;

  [[nodiscard]] T *ptr(size_type index) noexcept {
    return reinterpret_cast<T *>(m_storage[index].storage);
  }

  [[nodiscard]] const T *ptr(size_type index) const noexcept {
    return reinterpret_cast<const T *>(m_storage[index].storage);
  }

  [[nodiscard]] size_type physical_index(size_type index) const noexcept {
    const size_type remaining = Capacity - m_begin;
    return index < remaining ? m_begin + index : index - remaining;
  }

  [[nodiscard]] size_type increment(size_type index) const noexcept {
    return index + 1 == Capacity ? 0 : index + 1;
  }

  [[nodiscard]] size_type decrement(size_type index) const noexcept {
    return index == 0 ? Capacity - 1 : index - 1;
  }

  Slot m_storage[storage_capacity];
  size_type m_size = 0;
  size_type m_begin = 0;
};

} // namespace strobe

namespace fmt {

/**
 * \brief Formats an inline deque in logical element order.
 * \code{.cpp}
 * template<typename T, size_t Capacity>
 * struct formatter<strobe::InplaceVectorDeque<T, Capacity>>;
 * \endcode
 */
template <typename T, size_t Capacity>
struct formatter<strobe::InplaceVectorDeque<T, Capacity>> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }

  template <typename FormatContext>
  auto format(const strobe::InplaceVectorDeque<T, Capacity> &value,
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
