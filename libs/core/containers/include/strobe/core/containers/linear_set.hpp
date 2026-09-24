#pragma once

#include <algorithm>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <iterator>
#include <limits>
#include <memory>
#include <type_traits>
#include <utility>

#include <strobe/memory.hpp>

namespace strobe {

template <std::equality_comparable T, Allocator A = Mallocator>
class LinearSet {
  using ATraits = AllocatorTraits<A>;

public:
  using value_type = T;
  using size_type = std::size_t;
  using iterator = const T *;
  using const_iterator = const T *;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  explicit LinearSet(const A &allocator = {}) : m_allocator(allocator) {}

  LinearSet(const LinearSet &other)
      : m_allocator(
            ATraits::select_on_container_copy_construction(other.m_allocator)) {
    reserve(other.m_size);

    for (size_type i = 0; i < other.m_size; ++i)
      append(other.m_values[i]);
  }

  LinearSet(LinearSet &&other) noexcept(std::is_nothrow_move_constructible_v<A>)
      : m_values(std::exchange(other.m_values, nullptr)),
        m_size(std::exchange(other.m_size, 0)),
        m_capacity(std::exchange(other.m_capacity, 0)),
        m_allocator(std::move(other.m_allocator)) {}

  ~LinearSet() { reset(); }

  LinearSet &operator=(const LinearSet &other) {
    if (this == &other)
      return *this;

    if constexpr (ATraits::propagate_on_container_copy_assignment) {
      if (!strobe::alloc_equals(m_allocator, other.m_allocator))
        reset();

      m_allocator = other.m_allocator;
    }

    clear();
    reserve(other.m_size);

    for (size_type i = 0; i < other.m_size; ++i)
      append(other.m_values[i]);

    return *this;
  }

  LinearSet &operator=(LinearSet &&other) {
    if (this == &other)
      return *this;

    if constexpr (ATraits::propagate_on_container_move_assignment) {
      reset();
      m_allocator = std::move(other.m_allocator);
      steal(other);
    } else if (strobe::alloc_equals(m_allocator, other.m_allocator)) {
      reset();
      steal(other);
    } else {
      clear();
      reserve(other.m_size);

      for (size_type i = 0; i < other.m_size; ++i)
        append(std::move(other.m_values[i]));

      other.clear();
    }

    return *this;
  }

  template <std::equality_comparable_with<T> K>
  iterator find(const K &value) const {
    for (size_type i = 0; i < m_size; ++i) {
      if (m_values[i] == value)
        return m_values + i;
    }

    return end();
  }

  template <std::equality_comparable_with<T> K>
  bool contains(const K &value) const {
    return find(value) != end();
  }

  template <typename U>
    requires std::constructible_from<T, U &&> &&
             std::equality_comparable_with<std::remove_cvref_t<U>, T>
  iterator insert(U &&value) {
    if (auto it = find(value); it != end())
      return it;

    // value may refer into this set, and reserve may move its storage.
    T staged(std::forward<U>(value));
    append(std::move(staged));
    return m_values + m_size - 1;
  }

  void insert_unchecked(T &&value) {
    assert(!contains(value));

    T staged(std::move(value));
    append(std::move(staged));
  }

  iterator erase(const_iterator pos) {
    assert(pos != end());
    assert(m_values != nullptr);

    const size_type index = static_cast<size_type>(pos - begin());
    assert(index < m_size);

    const size_type last = m_size - 1;
    if (index != last)
      m_values[index] = std::move(m_values[last]);

    std::destroy_at(m_values + last);
    --m_size;

    return index == m_size ? end() : m_values + index;
  }

  template <std::equality_comparable_with<T> K> bool erase(const K &value) {
    auto it = find(value);
    if (it == end())
      return false;

    erase(it);
    return true;
  }

  [[nodiscard]]
  size_type size() const noexcept {
    return m_size;
  }

  [[nodiscard]]
  size_type capacity() const noexcept {
    return m_capacity;
  }

  [[nodiscard]]
  bool empty() const noexcept {
    return m_size == 0;
  }

  void clear() noexcept {
    if (m_size != 0)
      std::destroy_n(m_values, m_size);

    m_size = 0;
  }

  void reserve(size_type count) {
    if (count <= m_capacity)
      return;

    assert(count <= max_size());

    auto [newValues, newCapacity] =
        ATraits::template allocate_at_least<T>(m_allocator, count);

    assert(newValues != nullptr && newCapacity >= count);

    const size_type oldSize = m_size;

    for (size_type i = 0; i < oldSize; ++i)
      std::construct_at(newValues + i, std::move(m_values[i]));

    clear();

    if (m_values)
      ATraits::template deallocate<T>(m_allocator, m_values, m_capacity);

    m_values = newValues;
    m_size = oldSize;
    m_capacity = newCapacity;
  }

  iterator begin() noexcept { return m_values; }
  const_iterator begin() const noexcept { return m_values; }
  const_iterator cbegin() const noexcept { return begin(); }

  iterator end() noexcept { return m_values ? m_values + m_size : nullptr; }

  const_iterator end() const noexcept {
    return m_values ? m_values + m_size : nullptr;
  }

  const_iterator cend() const noexcept { return end(); }

  reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }

  const_reverse_iterator rbegin() const noexcept {
    return const_reverse_iterator(end());
  }

  const_reverse_iterator crbegin() const noexcept {
    return const_reverse_iterator(cend());
  }

  reverse_iterator rend() noexcept { return reverse_iterator(begin()); }

  const_reverse_iterator rend() const noexcept {
    return const_reverse_iterator(begin());
  }

  const_reverse_iterator crend() const noexcept {
    return const_reverse_iterator(cbegin());
  }

private:
  static constexpr size_type max_size() noexcept {
    return std::numeric_limits<size_type>::max() / sizeof(T);
  }

  void append(T &&value) {
    assert(m_size < max_size());

    if (m_size == m_capacity) {
      const size_type next = m_capacity <= max_size() / 2
                                 ? std::max<size_type>(1, m_capacity * 2)
                                 : max_size();

      reserve(next);
    }

    std::construct_at(m_values + m_size, std::move(value));
    ++m_size;
  }

  void append(const T &value) {
    assert(m_size < max_size());

    if (m_size == m_capacity) {
      const size_type next = m_capacity <= max_size() / 2
                                 ? std::max<size_type>(1, m_capacity * 2)
                                 : max_size();

      reserve(next);
    }

    std::construct_at(m_values + m_size, value);
    ++m_size;
  }

  void steal(LinearSet &other) noexcept {
    m_values = std::exchange(other.m_values, nullptr);
    m_size = std::exchange(other.m_size, 0);
    m_capacity = std::exchange(other.m_capacity, 0);
  }

  void reset() noexcept {
    clear();

    if (m_values)
      ATraits::template deallocate<T>(m_allocator, m_values, m_capacity);

    m_values = nullptr;
    m_capacity = 0;
  }

  T *m_values = nullptr;
  size_type m_size = 0;
  size_type m_capacity = 0;

  [[no_unique_address]]
  A m_allocator;
};

} // namespace strobe
