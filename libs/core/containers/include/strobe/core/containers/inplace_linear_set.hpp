#pragma once

#include <cassert>
#include <concepts>
#include <cstddef>
#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>

namespace strobe {

template <std::equality_comparable T, std::size_t Capacity>
class InplaceLinearSet {
public:
  using value_type = T;
  using size_type = std::size_t;
  using iterator = const T *;
  using const_iterator = const T *;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

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

  template <std::equality_comparable_with<T> K>
  iterator find(const K &value) const {
    for (size_type i = 0; i < m_size; ++i) {
      if (data()[i] == value)
        return data() + i;
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

    append(std::forward<U>(value));
    return data() + m_size - 1;
  }

  void insert_unchecked(T &&value) {
    assert(!contains(value));
    append(std::move(value));
  }

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

  [[nodiscard]]
  size_type size() const noexcept {
    return m_size;
  }

  [[nodiscard]]
  static constexpr size_type capacity() noexcept {
    return Capacity;
  }

  [[nodiscard]]
  bool empty() const noexcept {
    return m_size == 0;
  }

  void clear() noexcept {
    std::destroy_n(data(), m_size);
    m_size = 0;
  }

  iterator begin() noexcept { return data(); }
  const_iterator begin() const noexcept { return data(); }
  const_iterator cbegin() const noexcept { return begin(); }

  iterator end() noexcept { return data() + m_size; }
  const_iterator end() const noexcept { return data() + m_size; }
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
  T *data() noexcept { return reinterpret_cast<T *>(m_storage); }

  const T *data() const noexcept {
    return reinterpret_cast<const T *>(m_storage);
  }

  // Private mutable access is needed to move elements out of another set.
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

  // A nonzero backing array also permits Capacity == 0 as a type.
  alignas(T) std::byte m_storage[sizeof(T) * (Capacity ? Capacity : 1)];

  size_type m_size = 0;
};

} // namespace strobe
