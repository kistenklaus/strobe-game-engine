#pragma once

#include <cassert>
#include <concepts>
#include <cstddef>
#include <iterator>
#include <functional>
#include <memory>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <fmt/format.h>

namespace strobe {

/**
 * \ingroup core
 * \brief Fixed-capacity vector with inline storage.
 * \code{.cpp}
 * template<typename T, size_t Capacity>
 * class InplaceVector;
 * \endcode
 *
 * Stores up to \p Capacity elements without dynamic allocation.
 *
 * \attention 1. The vector cannot contain more than Capacity elements.
 * \attention 2. Insertions and removals may invalidate iterators and
 * references.
 */
template <typename T, std::size_t Capacity>
class InplaceVector {
public:
  using value_type = T;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;

  using reference = T &;
  using const_reference = const T &;

  using pointer = T *;
  using const_pointer = const T *;

  using iterator = T *;
  using const_iterator = const T *;

  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

private:
  static constexpr size_type STORAGE_CAPACITY = Capacity == 0 ? 1 : Capacity;

  struct Slot {
    alignas(T) std::byte storage[sizeof(T)];
  };

  [[nodiscard]] pointer ptr(size_type index) noexcept {
    return std::launder(reinterpret_cast<pointer>(m_storage[index].storage));
  }

  [[nodiscard]] const_pointer ptr(size_type index) const noexcept {
    return std::launder(
        reinterpret_cast<const_pointer>(m_storage[index].storage));
  }

  void destroy_range(size_type first, size_type last) noexcept {
    if constexpr (!std::is_trivially_destructible_v<T>) {
      for (size_type i = first; i < last; ++i) {
        std::destroy_at(ptr(i));
      }
    }
  }

private:
  Slot m_storage[STORAGE_CAPACITY];
  size_type m_size = 0;

public:
  /**
   * \brief Constructs, copies, or moves a vector.
   * \code{.cpp}
   * constexpr InplaceVector() noexcept;
   * InplaceVector(const InplaceVector& other);
   * InplaceVector(InplaceVector&& other) noexcept(...);
   * InplaceVector& operator=(const InplaceVector& other);
   * InplaceVector& operator=(InplaceVector&& other) noexcept(...);
   * ~InplaceVector();
   * \endcode
   * \param other Vector to copy or move from.
   *
   * Moved-from vectors are empty.
   */
  constexpr InplaceVector() noexcept = default;

  InplaceVector(const InplaceVector &other)
    requires std::copy_constructible<T>
  {
    size_type constructed = 0;
    const size_type other_size = other.size();

    try {
      for (; constructed < other_size; ++constructed) {
        std::construct_at(ptr(constructed), other[constructed]);
      }
    } catch (...) {
      destroy_range(0, constructed);
      throw;
    }

    m_size = other_size;
  }

  InplaceVector(InplaceVector &&other) noexcept(
      std::is_nothrow_move_constructible_v<T>)
    requires std::move_constructible<T>
  {
    size_type constructed = 0;
    const size_type other_size = other.size();

    try {
      for (; constructed < other_size; ++constructed) {
        std::construct_at(ptr(constructed), std::move(other[constructed]));
      }
    } catch (...) {
      destroy_range(0, constructed);
      throw;
    }

    m_size = other_size;
    other.clear();
  }

  InplaceVector &operator=(const InplaceVector &other)
    requires std::copy_constructible<T> && std::is_copy_assignable_v<T>
  {
    if (this == &other) {
      return *this;
    }

    const size_type other_size = other.size();

    if (other_size <= m_size) {
      for (size_type i = 0; i < other_size; ++i) {
        (*this)[i] = other[i];
      }

      destroy_range(other_size, m_size);
      m_size = other_size;
      return *this;
    }

    size_type i = 0;

    for (; i < m_size; ++i) {
      (*this)[i] = other[i];
    }

    try {
      for (; i < other_size; ++i) {
        std::construct_at(ptr(i), other[i]);
      }
    } catch (...) {
      destroy_range(m_size, i);
      throw;
    }

    m_size = other_size;
    return *this;
  }

  InplaceVector &operator=(InplaceVector &&other) noexcept(
      std::is_nothrow_move_constructible_v<T> &&
      std::is_nothrow_move_assignable_v<T>)
    requires std::move_constructible<T> && std::is_move_assignable_v<T>
  {
    if (this == &other) {
      return *this;
    }

    const size_type other_size = other.size();

    if (other_size <= m_size) {
      for (size_type i = 0; i < other_size; ++i) {
        (*this)[i] = std::move(other[i]);
      }

      destroy_range(other_size, m_size);
      m_size = other_size;
      other.clear();
      return *this;
    }

    size_type i = 0;

    for (; i < m_size; ++i) {
      (*this)[i] = std::move(other[i]);
    }

    try {
      for (; i < other_size; ++i) {
        std::construct_at(ptr(i), std::move(other[i]));
      }
    } catch (...) {
      destroy_range(m_size, i);
      throw;
    }

    m_size = other_size;
    other.clear();
    return *this;
  }

  ~InplaceVector() noexcept { clear(); }

  /**
   * \brief Checks whether the vector is empty.
   * \code{.cpp}
   * constexpr bool empty() const noexcept;
   * \endcode
   * \return Whether size() is zero.
   */
  [[nodiscard]] constexpr bool empty() const noexcept {
    return m_size == 0;
  }

  /**
   * \brief Returns the number of elements.
   * \code{.cpp}
   * constexpr size_type size() const noexcept;
   * \endcode
   * \return Number of active elements.
   */
  [[nodiscard]] constexpr size_type size() const noexcept {
    return m_size;
  }

  /**
   * \brief Returns the element capacity.
   * \code{.cpp}
   * static constexpr size_type capacity() noexcept;
   * \endcode
   * \return Number of elements that fit.
   */
  [[nodiscard]] static constexpr size_type capacity() noexcept {
    return Capacity;
  }

  /**
   * \brief Returns the maximum element count.
   * \code{.cpp}
   * static constexpr size_type max_size() noexcept;
   * \endcode
   * \return Capacity.
   */
  [[nodiscard]] static constexpr size_type max_size() noexcept {
    return Capacity;
  }

  /**
   * \brief Returns the element storage.
   * \code{.cpp}
   * pointer data() noexcept;
   * const_pointer data() const noexcept;
   * \endcode
   *
   * \return Pointer to the first element.
   */
  [[nodiscard]] pointer data() noexcept {
    return ptr(0);
  }

  [[nodiscard]] const_pointer data() const noexcept {
    return ptr(0);
  }

  /**
   * \brief Returns an iterator to the first element.
   * \code{.cpp}
   * iterator begin() noexcept;
   * const_iterator begin() const noexcept;
   * \endcode
   * \return Iterator to the first element.
   */
  [[nodiscard]] iterator begin() noexcept {
    return data();
  }

  [[nodiscard]] const_iterator begin() const noexcept {
    return data();
  }

  /**
   * \brief Returns a const iterator to the first element.
   * \code{.cpp}
   * const_iterator cbegin() const noexcept;
   * \endcode
   * \return Const iterator to the first element.
   */
  [[nodiscard]] const_iterator cbegin() const noexcept {
    return data();
  }

  /**
   * \brief Returns an iterator past the last element.
   * \code{.cpp}
   * iterator end() noexcept;
   * const_iterator end() const noexcept;
   * \endcode
   * \return Iterator past the last element.
   */
  [[nodiscard]] iterator end() noexcept {
    return data() + m_size;
  }

  [[nodiscard]] const_iterator end() const noexcept {
    return data() + m_size;
  }

  /**
   * \brief Returns a const iterator past the last element.
   * \code{.cpp}
   * const_iterator cend() const noexcept;
   * \endcode
   * \return Const iterator past the last element.
   */
  [[nodiscard]] const_iterator cend() const noexcept {
    return data() + m_size;
  }

  /**
   * \brief Returns a reverse iterator to the last element.
   * \code{.cpp}
   * reverse_iterator rbegin() noexcept;
   * const_reverse_iterator rbegin() const noexcept;
   * \endcode
   * \return Reverse iterator to the last element.
   */
  [[nodiscard]] reverse_iterator rbegin() noexcept {
    return reverse_iterator{end()};
  }

  [[nodiscard]] const_reverse_iterator rbegin() const noexcept {
    return const_reverse_iterator{end()};
  }

  /**
   * \brief Returns a const reverse iterator to the last element.
   * \code{.cpp}
   * const_reverse_iterator crbegin() const noexcept;
   * \endcode
   * \return Const reverse iterator to the last element.
   */
  [[nodiscard]] const_reverse_iterator crbegin() const noexcept {
    return const_reverse_iterator{cend()};
  }

  /**
   * \brief Returns a reverse iterator before the first element.
   * \code{.cpp}
   * reverse_iterator rend() noexcept;
   * const_reverse_iterator rend() const noexcept;
   * \endcode
   * \return Reverse iterator before the first element.
   */
  [[nodiscard]] reverse_iterator rend() noexcept {
    return reverse_iterator{begin()};
  }

  [[nodiscard]] const_reverse_iterator rend() const noexcept {
    return const_reverse_iterator{begin()};
  }

  /**
   * \brief Returns a const reverse iterator before the first element.
   * \code{.cpp}
   * const_reverse_iterator crend() const noexcept;
   * \endcode
   * \return Const reverse iterator before the first element.
   */
  [[nodiscard]] const_reverse_iterator crend() const noexcept {
    return const_reverse_iterator{cbegin()};
  }

  /**
   * \brief Returns a view over the active elements.
   * \code{.cpp}
   * std::span<T> span() noexcept;
   * std::span<const T> span() const noexcept;
   * \endcode
   *
   * \return A span over the active elements.
   */
  [[nodiscard]] std::span<T> span() noexcept {
    return {data(), m_size};
  }

  [[nodiscard]] std::span<const T> span() const noexcept {
    return {data(), m_size};
  }

  /**
   * \brief Accesses an element without bounds checking.
   * \code{.cpp}
   * reference operator[](size_type index) noexcept;
   * const_reference operator[](size_type index) const noexcept;
   * \endcode
   * \param index Element index.
   * \return Reference to the element.
   * \attention 1. \p index must be less than size().
   */
  [[nodiscard]] reference operator[](size_type index) noexcept {
    assert(index < m_size);
    return data()[index];
  }

  [[nodiscard]] const_reference operator[](size_type index) const noexcept {
    assert(index < m_size);
    return data()[index];
  }

  /**
   * \brief Accesses an element with bounds checking.
   * \code{.cpp}
   * reference at(size_type index);
   * const_reference at(size_type index) const;
   * \endcode
   * \param index Element index.
   * \return Reference to the element.
   * \throws std::out_of_range when \p index is not less than size().
   */
  [[nodiscard]] reference at(size_type index) {
    if (index >= m_size) {
      throw std::out_of_range{"InplaceVector::at"};
    }

    return data()[index];
  }

  [[nodiscard]] const_reference at(size_type index) const {
    if (index >= m_size) {
      throw std::out_of_range{"InplaceVector::at"};
    }

    return data()[index];
  }

  /**
   * \brief Returns the first element.
   * \code{.cpp}
   * reference front() noexcept;
   * const_reference front() const noexcept;
   * \endcode
   * \return Reference to the first element.
   * \attention 1. The vector must not be empty.
   */
  [[nodiscard]] reference front() noexcept {
    assert(m_size != 0);
    return data()[0];
  }

  [[nodiscard]] const_reference front() const noexcept {
    assert(m_size != 0);
    return data()[0];
  }

  /**
   * \brief Returns the last element.
   * \code{.cpp}
   * reference back() noexcept;
   * const_reference back() const noexcept;
   * \endcode
   * \return Reference to the last element.
   * \attention 1. The vector must not be empty.
   */
  [[nodiscard]] reference back() noexcept {
    assert(m_size != 0);
    return data()[m_size - 1];
  }

  [[nodiscard]] const_reference back() const noexcept {
    assert(m_size != 0);
    return data()[m_size - 1];
  }

  /**
   * \brief Constructs an element at the back.
   * \code{.cpp}
   * template<typename... Args>
   * reference emplace_back(Args&&... args);
   * \endcode
   * \param args Arguments forwarded to T.
   * \return Reference to the appended element.
   * \attention 1. The vector must have fewer than Capacity elements.
   */
  template <typename... Args>
  reference emplace_back(Args &&...args) {
    assert(m_size < Capacity);

    std::construct_at(ptr(m_size), std::forward<Args>(args)...);
    ++m_size;

    return back();
  }

  /**
   * \brief Appends a copy of an element.
   * \code{.cpp}
   * void push_back(const T& value);
   * \endcode
   * \param value Element to append.
   * \attention 1. The vector must have fewer than Capacity elements.
   */
  void push_back(const T &value) {
    emplace_back(value);
  }

  /**
   * \brief Appends an element by move construction.
   * \code{.cpp}
   * void push_back(T&& value);
   * \endcode
   * \param value Element to append.
   * \attention 1. The vector must have fewer than Capacity elements.
   */
  void push_back(T &&value) {
    emplace_back(std::move(value));
  }

  /**
   * \brief Removes the last element.
   * \code{.cpp}
   * void pop_back() noexcept;
   * \endcode
   *
   * \attention 1. The vector must not be empty.
   */
  void pop_back() noexcept {
    assert(m_size != 0);

    --m_size;

    if constexpr (!std::is_trivially_destructible_v<T>) {
      std::destroy_at(ptr(m_size));
    }
  }

  /**
   * \brief Removes all elements.
   * \code{.cpp}
   * void clear() noexcept;
   * \endcode
   */
  void clear() noexcept {
    destroy_range(0, m_size);
    m_size = 0;
  }

  /**
   * \brief Changes the number of elements.
   * \code{.cpp}
   * void resize(size_type new_size);
   * void resize(size_type new_size, const T& value);
   * \endcode
   *
   * \param new_size New number of elements.
   * \param value Value used to initialize appended elements.
   *
   * \attention 1. \p new_size must not exceed Capacity.
   * \attention 2. The first overload requires T to be default-initializable.
   */
  void resize(size_type new_size)
    requires std::default_initializable<T>
  {
    assert(new_size <= Capacity);

    if (new_size < m_size) {
      destroy_range(new_size, m_size);
      m_size = new_size;
      return;
    }

    while (m_size < new_size) {
      emplace_back();
    }
  }

  void resize(size_type new_size, const T &value) {
    assert(new_size <= Capacity);

    if (new_size < m_size) {
      destroy_range(new_size, m_size);
      m_size = new_size;
      return;
    }

    while (m_size < new_size) {
      emplace_back(value);
    }
  }
};

} // namespace strobe

namespace std {

/**
 * \brief Hashes an inline vector when its element type is hashable.
 * \code{.cpp}
 * template<typename T, size_t Capacity>
 * struct hash<strobe::InplaceVector<T, Capacity>>;
 * \endcode
 */
template <typename T, size_t Capacity>
  requires requires(const T &value) {
    { std::hash<T>{}(value) } -> std::convertible_to<size_t>;
  }
struct hash<strobe::InplaceVector<T, Capacity>> {
  size_t operator()(const strobe::InplaceVector<T, Capacity> &value) const
      noexcept(noexcept(std::hash<T>{}(*value.begin()))) {
    size_t result = static_cast<size_t>(1469598103934665603ull);
    for (const T &element : value) {
      result ^= std::hash<T>{}(element) +
                static_cast<size_t>(0x9e3779b97f4a7c15ull) +
                (result << 6) + (result >> 2);
    }
    return result;
  }
};

} // namespace std

namespace fmt {

/**
 * \brief Formats an inline vector as a bracketed list.
 * \code{.cpp}
 * template<typename T, size_t Capacity>
 * struct formatter<strobe::InplaceVector<T, Capacity>>;
 * \endcode
 */
template <typename T, size_t Capacity>
struct formatter<strobe::InplaceVector<T, Capacity>> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }

  template <typename FormatContext>
  auto format(const strobe::InplaceVector<T, Capacity> &value,
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
