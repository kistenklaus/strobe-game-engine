#pragma once

#include <cassert>
#include <concepts>
#include <cstddef>
#include <fmt/format.h>
#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>

namespace strobe {

/**
 * \ingroup core
 * \brief Fixed-capacity set with linear search.
 * \code{.cpp}
 * template<std::equality_comparable T, size_t Capacity>
 * class InplaceLinearSet;
 * \endcode
 *
 * Stores unique values in inline storage. Erasing an element does not preserve
 * element order.
 *
 * \attention 1. The set cannot contain more than Capacity elements.
 * \attention 2. T must be equality-comparable.
 * \attention 3. Iterators are invalidated by insertion and erasure.
 */
template <std::equality_comparable T, std::size_t Capacity>
class InplaceLinearSet {
public:
  using value_type = T;
  using size_type = std::size_t;
  using iterator = const T *;
  using const_iterator = const T *;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  /**
   * \brief Constructs, copies, or moves a set.
   * \code{.cpp}
   * InplaceLinearSet();
   * InplaceLinearSet(const InplaceLinearSet& other);
   * InplaceLinearSet(InplaceLinearSet&& other);
   * InplaceLinearSet& operator=(const InplaceLinearSet& other);
   * InplaceLinearSet& operator=(InplaceLinearSet&& other);
   * ~InplaceLinearSet();
   * \endcode
   *
   * \param other Set to copy or move from.
   *
   * Moved-from sets are empty.
   */
  InplaceLinearSet() = default;

  InplaceLinearSet(const InplaceLinearSet &other) {
    for (const T &value : other)
      append(value);
  }

  InplaceLinearSet(InplaceLinearSet &&other) {
    for (T &value : other.mutable_values())
      append(std::move(value));

    other.clear();
  }

  ~InplaceLinearSet() { clear(); }

  InplaceLinearSet &operator=(const InplaceLinearSet &other) {
    if (this == &other)
      return *this;

    clear();

    for (const T &value : other)
      append(value);

    return *this;
  }

  InplaceLinearSet &operator=(InplaceLinearSet &&other) {
    if (this == &other)
      return *this;

    clear();

    for (T &value : other.mutable_values())
      append(std::move(value));

    other.clear();
    return *this;
  }

  /**
   * \brief Finds a value.
   * \code{.cpp}
   * template<std::equality_comparable_with<T> K>
   * iterator find(const K& value) const;
   * \endcode
   *
   * \param value Value to find.
   *
   * \return Iterator to the value, or end() if absent.
   */
  template <std::equality_comparable_with<T> K>
  iterator find(const K &value) const {
    for (size_type i = 0; i < m_size; ++i) {
      if (data()[i] == value)
        return data() + i;
    }

    return end();
  }

  /**
   * \brief Checks whether a value exists.
   * \code{.cpp}
   * template<std::equality_comparable_with<T> K>
   * bool contains(const K& value) const;
   * \endcode
   *
   * \param value Value to find.
   *
   * \return Whether the value exists.
   */
  template <std::equality_comparable_with<T> K>
  bool contains(const K &value) const {
    return find(value) != end();
  }

  /**
   * \brief Inserts a value if absent.
   * \code{.cpp}
   * template<typename U>
   * iterator insert(U&& value);
   * \endcode
   *
   * \param value Value to insert.
   *
   * \return Iterator to the existing or inserted value.
   *
   * \attention 1. The set must have fewer than Capacity elements when inserting
   * a new value.
   */
  template <typename U>
    requires std::constructible_from<T, U &&> &&
             std::equality_comparable_with<std::remove_cvref_t<U>, T>
  iterator insert(U &&value) {
    if (auto it = find(value); it != end())
      return it;

    append(std::forward<U>(value));
    return data() + m_size - 1;
  }

  /**
   * \brief Inserts a value without checking for duplicates.
   * \code{.cpp}
   * void insert_unchecked(T&& value);
   * \endcode
   *
   * \param value Value to insert.
   *
   * \attention 1. The value must not already exist.
   * \attention 2. The set must have fewer than Capacity elements.
   */
  void insert_unchecked(T &&value) {
    assert(!contains(value));
    append(std::move(value));
  }

  /**
   * \brief Erases an element by iterator.
   * \code{.cpp}
   * iterator erase(const_iterator position);
   * \endcode
   *
   * \param position Element to erase.
   * \return Iterator following the erased element.
   *
   * \attention 1. Erasing may reorder the remaining elements.
   */
  iterator erase(const_iterator pos) {
    assert(pos != end());

    const size_type index = static_cast<size_type>(pos - begin());
    assert(index < m_size);

    const size_type last = m_size - 1;

    if (index != last)
      data()[index] = std::move(data()[last]);

    std::destroy_at(data() + last);
    --m_size;

    return index == m_size ? end() : data() + index;
  }

  /**
   * \brief Erases an element by value.
   * \code{.cpp}
   * template<std::equality_comparable_with<T> K>
   * bool erase(const K& value);
   * \endcode
   * \param value Value to erase.
   * \return Whether a value was erased.
   * \attention 1. Erasing may reorder the remaining elements.
   */
  template <std::equality_comparable_with<T> K> bool erase(const K &value) {
    auto it = find(value);

    if (it == end())
      return false;

    erase(it);
    return true;
  }

  /**
   * \brief Returns the number of stored values.
   * \code{.cpp}
   * size_type size() const noexcept;
   * \endcode
   * \return Number of stored values.
   */
  [[nodiscard]]
  size_type size() const noexcept {
    return m_size;
  }

  /**
   * \brief Returns the maximum number of values.
   * \code{.cpp}
   * static constexpr size_type capacity() noexcept;
   * \endcode
   *
   * \return Maximum number of values.
   */
  [[nodiscard]] static constexpr size_type capacity() noexcept {
    return Capacity;
  }

  /**
   * \brief Checks whether the set is empty.
   * \code{.cpp}
   * bool empty() const noexcept;
   * \endcode
   *
   * \return Whether size() is zero.
   */
  [[nodiscard]] bool empty() const noexcept { return m_size == 0; }

  /**
   * \brief Removes all values.
   * \code{.cpp}
   * void clear() noexcept;
   * \endcode
   */
  void clear() noexcept {
    std::destroy_n(data(), m_size);
    m_size = 0;
  }

  /**
   * \brief Returns the first mutable iterator.
   * \code{.cpp}
   * iterator begin() noexcept;
   * \endcode
   */
  iterator begin() noexcept { return data(); }

  /**
   * \brief Returns the first read-only iterator.
   * \code{.cpp}
   * const_iterator begin() const noexcept;
   * \endcode
   */
  const_iterator begin() const noexcept { return data(); }

  /**
   * \brief Returns the first read-only iterator.
   * \code{.cpp}
   * const_iterator cbegin() const noexcept;
   * \endcode
   */
  const_iterator cbegin() const noexcept { return begin(); }

  /**
   * \brief Returns the mutable end iterator.
   * \code{.cpp}
   * iterator end() noexcept;
   * \endcode
   */
  iterator end() noexcept { return data() + m_size; }

  /**
   * \brief Returns the read-only end iterator.
   * \code{.cpp}
   * const_iterator end() const noexcept;
   * \endcode
   */
  const_iterator end() const noexcept { return data() + m_size; }

  /**
   * \brief Returns the read-only end iterator.
   * \code{.cpp}
   * const_iterator cend() const noexcept;
   * \endcode
   */
  const_iterator cend() const noexcept { return end(); }

  /**
   * \brief Returns the mutable reverse-begin iterator.
   * \code{.cpp}
   * reverse_iterator rbegin() noexcept;
   * \endcode
   */
  reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }

  /**
   * \brief Returns the read-only reverse-begin iterator.
   * \code{.cpp}
   * const_reverse_iterator rbegin() const noexcept;
   * \endcode
   */
  const_reverse_iterator rbegin() const noexcept {
    return const_reverse_iterator(end());
  }

  /**
   * \brief Returns the read-only reverse-begin iterator.
   * \code{.cpp}
   * const_reverse_iterator crbegin() const noexcept;
   * \endcode
   */
  const_reverse_iterator crbegin() const noexcept {
    return const_reverse_iterator(cend());
  }

  /**
   * \brief Returns the mutable reverse-end iterator.
   * \code{.cpp}
   * reverse_iterator rend() noexcept;
   * \endcode
   */
  reverse_iterator rend() noexcept { return reverse_iterator(begin()); }

  /**
   * \brief Returns the read-only reverse-end iterator.
   * \code{.cpp}
   * const_reverse_iterator rend() const noexcept;
   * \endcode
   */
  const_reverse_iterator rend() const noexcept {
    return const_reverse_iterator(begin());
  }

  /**
   * \brief Returns the read-only reverse-end iterator.
   * \code{.cpp}
   * const_reverse_iterator crend() const noexcept;
   * \endcode
   */
  const_reverse_iterator crend() const noexcept {
    return const_reverse_iterator(cbegin());
  }

private:
  T *data() noexcept { return reinterpret_cast<T *>(m_storage); }

  const T *data() const noexcept {
    return reinterpret_cast<const T *>(m_storage);
  }

  struct MutableValues {
    T *first;
    T *last;

    T *begin() const { return first; }
    T *end() const { return last; }
  };

  MutableValues mutable_values() noexcept { return {data(), data() + m_size}; }

  template <typename U> void append(U &&value) {
    assert(m_size < Capacity);
    std::construct_at(data() + m_size, std::forward<U>(value));
    ++m_size;
  }

  alignas(T) std::byte m_storage[sizeof(T) * (Capacity ? Capacity : 1)];
  size_type m_size = 0;
};

} // namespace strobe

namespace fmt {

template <typename T, std::size_t Capacity>
struct formatter<strobe::InplaceLinearSet<T, Capacity>> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }

  template <typename FormatContext>
  auto format(const strobe::InplaceLinearSet<T, Capacity> &set,
              FormatContext &ctx) const {
    auto out = ctx.out();
    *out++ = '{';
    bool first = true;
    for (const T &value : set) {
      if (!first) {
        *out++ = ',';
        *out++ = ' ';
      }
      first = false;
      out = fmt::format_to(out, "{}", value);
    }
    *out++ = '}';
    return out;
  }
};

} // namespace fmt
