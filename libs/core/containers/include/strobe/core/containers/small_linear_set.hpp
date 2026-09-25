#pragma once

#include <algorithm>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <fmt/format.h>
#include <iterator>
#include <limits>
#include <memory>
#include <type_traits>
#include <utility>

#include <strobe/memory.hpp>

namespace strobe {

/**
 * \ingroup core
 * \brief Linear set with small-buffer optimization.
 * \code{.cpp}
 * template<std::equality_comparable T, size_t MinSVOCapacity = 8,
 *          Allocator A = Mallocator>
 * class SmallLinearSet;
 * \endcode
 *
 * Values are stored contiguously and erasure does not preserve order.
 *
 * \attention 1. Insertion and reserve() may invalidate all iterators and
 * references.
 * \attention 2. Erasure moves the last value into the erased position.
 */
template <std::equality_comparable T, std::size_t MinSVOCapacity = 8,
          Allocator A = Mallocator>
class SmallLinearSet {
  using ATraits = AllocatorTraits<A>;

public:
  using value_type = T;
  using size_type = std::size_t;
  using iterator = const T *;
  using const_iterator = const T *;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  /**
   * \brief Constructs, copies, moves, assigns, or destroys a set.
   * \code{.cpp}
   * explicit SmallLinearSet(const A& allocator = {});
   * SmallLinearSet(const SmallLinearSet& other);
   * SmallLinearSet(SmallLinearSet&& other);
   * SmallLinearSet& operator=(const SmallLinearSet& other);
   * SmallLinearSet& operator=(SmallLinearSet&& other);
   * ~SmallLinearSet();
   * \endcode
   * \param allocator Allocator to use.
   * \param other Set to copy or move from.
   *
   * Moved-from sets are empty.
   */
  explicit SmallLinearSet(const A &allocator = {}) : m_allocator(allocator) {}

  SmallLinearSet(const SmallLinearSet &other)
      : m_allocator(
            ATraits::select_on_container_copy_construction(other.m_allocator)) {
    reserve(other.m_size);

    for (const T &value : other)
      append(value);
  }

  SmallLinearSet(SmallLinearSet &&other)
      : m_allocator(std::move(other.m_allocator)) {
    take_or_move(other);
  }

  ~SmallLinearSet() { reset(); }

  SmallLinearSet &operator=(const SmallLinearSet &other) {
    if (this == &other)
      return *this;

    if constexpr (ATraits::propagate_on_container_copy_assignment) {
      if (!strobe::alloc_equals(m_allocator, other.m_allocator))
        reset();

      m_allocator = other.m_allocator;
    }

    clear();
    reserve(other.m_size);

    for (const T &value : other)
      append(value);

    return *this;
  }

  SmallLinearSet &operator=(SmallLinearSet &&other) {
    if (this == &other)
      return *this;

    if constexpr (ATraits::propagate_on_container_move_assignment) {
      reset();
      m_allocator = std::move(other.m_allocator);
      take_or_move(other);
    } else if (strobe::alloc_equals(m_allocator, other.m_allocator)) {
      reset();
      take_or_move(other);
    } else {
      clear();
      reserve(other.m_size);

      for (size_type i = 0; i < other.m_size; ++i)
        append(std::move(other.data()[i]));

      other.clear();
    }

    return *this;
  }

  /**
   * \brief Finds a value.
   * \code{.cpp}
   * template<std::equality_comparable_with<T> K>
   * iterator find(const K& value) const;
   * \endcode
   * \param value Value to find.
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
   * \param value Value to find.
   * \return Whether the value exists.
   */
  template <std::equality_comparable_with<T> K>
  bool contains(const K &value) const {
    return find(value) != end();
  }

  /**
   * \brief Inserts a value if it is absent.
   * \code{.cpp}
   * template<typename U>
   * iterator insert(U&& value);
   * \endcode
   * \param value Value to insert.
   * \return Iterator to the existing or inserted value.
   */
  template <typename U>
    requires std::constructible_from<T, U &&> &&
             std::equality_comparable_with<std::remove_cvref_t<U>, T>
  iterator insert(U &&value) {
    if (auto it = find(value); it != end())
      return it;

    // value may refer into the current storage, which growth would move.
    T staged(std::forward<U>(value));
    append(std::move(staged));
    return data() + m_size - 1;
  }

  /**
   * \brief Inserts a value without searching for a duplicate.
   * \code{.cpp}
   * void insert_unchecked(T&& value);
   * \endcode
   * \param value Value to insert.
   * \attention 1. The set must not already contain \p value.
   */
  void insert_unchecked(T &&value) {
    assert(!contains(value));

    T staged(std::move(value));
    append(std::move(staged));
  }

  /**
   * \brief Erases a value.
   * \code{.cpp}
   * iterator erase(const_iterator pos);
   * template<std::equality_comparable_with<T> K>
   * bool erase(const K& value);
   * \endcode
   * \param pos Value to erase.
   * \param value Value to find and erase.
   * \return The iterator at the erased position, or whether a value was
   * erased.
   * \attention 1. For the iterator overload, \p pos must refer to a value in
   * this set.
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

  template <std::equality_comparable_with<T> K> bool erase(const K &value) {
    auto it = find(value);
    if (it == end())
      return false;

    erase(it);
    return true;
  }

  /**
   * \brief Returns the number of values.
   * \code{.cpp}
   * size_type size() const noexcept;
   * \endcode
   * \return Number of values.
   */
  [[nodiscard]]
  size_type size() const noexcept {
    return m_size;
  }

  /**
   * \brief Checks whether the set is empty.
   * \code{.cpp}
   * bool empty() const noexcept;
   * \endcode
   * \return Whether size() is zero.
   */
  [[nodiscard]]
  bool empty() const noexcept {
    return m_size == 0;
  }

  /**
   * \brief Returns the value capacity.
   * \code{.cpp}
   * size_type capacity() const noexcept;
   * \endcode
   * \return Number of values that fit without allocation.
   */
  [[nodiscard]]
  size_type capacity() const noexcept {
    return m_heapValues ? m_heapCapacity : MinSVOCapacity;
  }

  /**
   * \brief Removes all values.
   * \code{.cpp}
   * void clear() noexcept;
   * \endcode
   *
   * Capacity is retained.
   */
  void clear() noexcept {
    std::destroy_n(data(), m_size);
    m_size = 0;
  }

  /**
   * \brief Reserves capacity for values.
   * \code{.cpp}
   * void reserve(size_type count);
   * \endcode
   * \param count Desired number of values.
   *
   * \attention 1. \p count must not exceed max_size().
   */
  void reserve(size_type count) {
    if (count <= capacity())
      return;

    assert(count <= max_size());

    auto [newValues, newCapacity] =
        ATraits::template allocate_at_least<T>(m_allocator, count);

    assert(newValues != nullptr && newCapacity >= count);

    T *oldValues = data();
    const size_type oldSize = m_size;

    for (size_type i = 0; i < oldSize; ++i)
      std::construct_at(newValues + i, std::move(oldValues[i]));

    clear();

    if (m_heapValues) {
      ATraits::template deallocate<T>(m_allocator, m_heapValues,
                                      m_heapCapacity);
    }

    m_heapValues = newValues;
    m_heapCapacity = newCapacity;
    m_size = oldSize;
  }

  /**
   * \brief Returns an iterator to the first value.
   * \code{.cpp}
   * iterator begin() noexcept;
   * const_iterator begin() const noexcept;
   * \endcode
   * \return Iterator to the first value.
   */
  iterator begin() noexcept { return data(); }
  const_iterator begin() const noexcept { return data(); }

  /**
   * \brief Returns a const iterator to the first value.
   * \code{.cpp}
   * const_iterator cbegin() const noexcept;
   * \endcode
   * \return Const iterator to the first value.
   */
  const_iterator cbegin() const noexcept { return begin(); }

  /**
   * \brief Returns an iterator past the last value.
   * \code{.cpp}
   * iterator end() noexcept;
   * const_iterator end() const noexcept;
   * \endcode
   * \return Iterator past the last value.
   */
  iterator end() noexcept { return data() + m_size; }
  const_iterator end() const noexcept { return data() + m_size; }

  /**
   * \brief Returns a const iterator past the last value.
   * \code{.cpp}
   * const_iterator cend() const noexcept;
   * \endcode
   * \return Const iterator past the last value.
   */
  const_iterator cend() const noexcept { return end(); }

  /**
   * \brief Returns a reverse iterator to the last value.
   * \code{.cpp}
   * reverse_iterator rbegin() noexcept;
   * const_reverse_iterator rbegin() const noexcept;
   * \endcode
   * \return Reverse iterator to the last value.
   */
  reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }

  const_reverse_iterator rbegin() const noexcept {
    return const_reverse_iterator(end());
  }

  /**
   * \brief Returns a const reverse iterator to the last value.
   * \code{.cpp}
   * const_reverse_iterator crbegin() const noexcept;
   * \endcode
   * \return Const reverse iterator to the last value.
   */
  const_reverse_iterator crbegin() const noexcept {
    return const_reverse_iterator(cend());
  }

  /**
   * \brief Returns a reverse iterator before the first value.
   * \code{.cpp}
   * reverse_iterator rend() noexcept;
   * const_reverse_iterator rend() const noexcept;
   * \endcode
   * \return Reverse iterator before the first value.
   */
  reverse_iterator rend() noexcept { return reverse_iterator(begin()); }

  const_reverse_iterator rend() const noexcept {
    return const_reverse_iterator(begin());
  }

  /**
   * \brief Returns a const reverse iterator before the first value.
   * \code{.cpp}
   * const_reverse_iterator crend() const noexcept;
   * \endcode
   * \return Const reverse iterator before the first value.
   */
  const_reverse_iterator crend() const noexcept {
    return const_reverse_iterator(cbegin());
  }

private:
  static constexpr size_type max_size() noexcept {
    return std::numeric_limits<size_type>::max() / sizeof(T);
  }

  T *inline_data() noexcept { return reinterpret_cast<T *>(m_inlineStorage); }

  const T *inline_data() const noexcept {
    return reinterpret_cast<const T *>(m_inlineStorage);
  }

  T *data() noexcept { return m_heapValues ? m_heapValues : inline_data(); }

  const T *data() const noexcept {
    return m_heapValues ? m_heapValues : inline_data();
  }

  template <typename U> void append(U &&value) {
    assert(m_size < max_size());

    if (m_size == capacity()) {
      const size_type next = capacity() <= max_size() / 2
                                 ? std::max<size_type>(1, capacity() * 2)
                                 : max_size();

      reserve(next);
    }

    std::construct_at(data() + m_size, std::forward<U>(value));
    ++m_size;
  }

  void take_or_move(SmallLinearSet &other) {
    if (other.m_heapValues) {
      m_heapValues = std::exchange(other.m_heapValues, nullptr);
      m_heapCapacity = std::exchange(other.m_heapCapacity, 0);
      m_size = std::exchange(other.m_size, 0);
      return;
    }

    for (size_type i = 0; i < other.m_size; ++i)
      append(std::move(other.data()[i]));

    other.clear();
  }

  void reset() noexcept {
    clear();

    if (m_heapValues) {
      ATraits::template deallocate<T>(m_allocator, m_heapValues,
                                      m_heapCapacity);
    }

    m_heapValues = nullptr;
    m_heapCapacity = 0;
  }

  alignas(T) std::byte
      m_inlineStorage[sizeof(T) * (MinSVOCapacity ? MinSVOCapacity : 1)];

  T *m_heapValues = nullptr;
  size_type m_size = 0;
  size_type m_heapCapacity = 0;

  [[no_unique_address]]
  A m_allocator;
};

} // namespace strobe

namespace fmt {

/**
 * \brief Formats a small linear set as a brace-enclosed list.
 * \code{.cpp}
 * template<std::equality_comparable T, size_t MinSVOCapacity,
 *          strobe::Allocator A>
 * struct formatter<strobe::SmallLinearSet<T, MinSVOCapacity, A>>;
 * \endcode
 */
template <std::equality_comparable T, size_t MinSVOCapacity,
          strobe::Allocator A>
struct formatter<strobe::SmallLinearSet<T, MinSVOCapacity, A>> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }

  template <typename FormatContext>
  auto format(const strobe::SmallLinearSet<T, MinSVOCapacity, A> &set,
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
