#pragma once

#include <cassert>
#include <concepts>
#include <cstddef>
#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>

#include "strobe/core/containers/span.hpp"

namespace strobe {

template <std::equality_comparable K, typename V, std::size_t Capacity>
class InplaceLinearMap {
public:
  using size_type = std::size_t;

  template <bool Const> class Iterator {
    using Value = std::conditional_t<Const, const V, V>;

    friend class InplaceLinearMap;
    template <bool> friend class Iterator;

    Iterator(const K *key, Value *value) : m_key(key), m_value(value) {}

  public:
    using iterator_category = std::forward_iterator_tag;
    using iterator_concept = std::forward_iterator_tag;
    using value_type = std::pair<K, V>;
    using difference_type = std::ptrdiff_t;
    using reference = std::pair<const K &, Value &>;

    Iterator() = default;
    Iterator(const Iterator &) = default;
    Iterator &operator=(const Iterator &) = default;

    Iterator(const Iterator<false> &other)
      requires Const
        : m_key(other.m_key), m_value(other.m_value) {}

    reference operator*() const { return {*m_key, *m_value}; }

    Iterator &operator++() {
      ++m_key;
      ++m_value;
      return *this;
    }

    Iterator operator++(int) {
      Iterator old = *this;
      ++*this;
      return old;
    }

    template <bool OtherConst>
    bool operator==(const Iterator<OtherConst> &other) const {
      return m_key == other.m_key;
    }

  private:
    const K *m_key = nullptr;
    Value *m_value = nullptr;
  };

  using iterator = Iterator<false>;
  using const_iterator = Iterator<true>;

  InplaceLinearMap() = default;

  InplaceLinearMap(const InplaceLinearMap &other) {
    for (size_type i = 0; i < other.m_size; ++i)
      append(other.key_data()[i], other.value_data()[i]);
  }

  InplaceLinearMap(InplaceLinearMap &&other) {
    for (size_type i = 0; i < other.m_size; ++i)
      append(std::move(other.key_data()[i]), std::move(other.value_data()[i]));

    other.clear();
  }

  ~InplaceLinearMap() { clear(); }

  InplaceLinearMap &operator=(const InplaceLinearMap &other) {
    if (this == &other)
      return *this;

    clear();

    for (size_type i = 0; i < other.m_size; ++i)
      append(other.key_data()[i], other.value_data()[i]);

    return *this;
  }

  InplaceLinearMap &operator=(InplaceLinearMap &&other) {
    if (this == &other)
      return *this;

    clear();

    for (size_type i = 0; i < other.m_size; ++i)
      append(std::move(other.key_data()[i]), std::move(other.value_data()[i]));

    other.clear();
    return *this;
  }

  std::pair<iterator, bool> insert(const K &key, const V &value) {
    const size_type index = find_index(key);
    if (index != m_size)
      return {iterator(key_data() + index, value_data() + index), false};

    append(key, value);
    return {iterator(key_data() + m_size - 1, value_data() + m_size - 1), true};
  }

  std::pair<iterator, bool> insert(const K &key, V &&value) {
    const size_type index = find_index(key);
    if (index != m_size)
      return {iterator(key_data() + index, value_data() + index), false};

    append(key, std::move(value));
    return {iterator(key_data() + m_size - 1, value_data() + m_size - 1), true};
  }

  std::pair<iterator, bool> insert(K &&key, const V &value) {
    const size_type index = find_index(key);
    if (index != m_size)
      return {iterator(key_data() + index, value_data() + index), false};

    append(std::move(key), value);
    return {iterator(key_data() + m_size - 1, value_data() + m_size - 1), true};
  }

  std::pair<iterator, bool> insert(K &&key, V &&value) {
    const size_type index = find_index(key);
    if (index != m_size)
      return {iterator(key_data() + index, value_data() + index), false};

    append(std::move(key), std::move(value));
    return {iterator(key_data() + m_size - 1, value_data() + m_size - 1), true};
  }

  bool erase(const K &key) {
    const size_type index = find_index(key);
    if (index == m_size)
      return false;

    const size_type last = m_size - 1;

    if (index != last) {
      key_data()[index] = std::move(key_data()[last]);
      value_data()[index] = std::move(value_data()[last]);
    }

    std::destroy_at(key_data() + last);
    std::destroy_at(value_data() + last);
    --m_size;
    return true;
  }

  [[nodiscard]]
  bool containsKey(const K &key) const {
    return find_index(key) != m_size;
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

  [[nodiscard]]
  span<const K> keys() const noexcept {
    return {key_data(), m_size};
  }

  [[nodiscard]]
  span<V> values() noexcept {
    return {value_data(), m_size};
  }

  [[nodiscard]]
  span<const V> values() const noexcept {
    return {value_data(), m_size};
  }

  iterator begin() noexcept { return {key_data(), value_data()}; }

  iterator end() noexcept {
    return {key_data() + m_size, value_data() + m_size};
  }

  const_iterator begin() const noexcept { return {key_data(), value_data()}; }

  const_iterator end() const noexcept {
    return {key_data() + m_size, value_data() + m_size};
  }

  const_iterator cbegin() const noexcept { return begin(); }

  const_iterator cend() const noexcept { return end(); }

  void clear() noexcept {
    std::destroy_n(key_data(), m_size);
    std::destroy_n(value_data(), m_size);
    m_size = 0;
  }

private:
  K *key_data() noexcept { return reinterpret_cast<K *>(m_keyStorage); }

  const K *key_data() const noexcept {
    return reinterpret_cast<const K *>(m_keyStorage);
  }

  V *value_data() noexcept { return reinterpret_cast<V *>(m_valueStorage); }

  const V *value_data() const noexcept {
    return reinterpret_cast<const V *>(m_valueStorage);
  }

  size_type find_index(const K &key) const {
    for (size_type i = 0; i < m_size; ++i) {
      if (key_data()[i] == key)
        return i;
    }

    return m_size;
  }

  template <typename Key, typename Value>
  void append(Key &&key, Value &&value) {
    assert(m_size < Capacity);

    std::construct_at(key_data() + m_size, std::forward<Key>(key));
    std::construct_at(value_data() + m_size, std::forward<Value>(value));
    ++m_size;
  }

  // Keep a nonzero backing array so Capacity == 0 remains a valid type.
  alignas(K) std::byte m_keyStorage[sizeof(K) * (Capacity ? Capacity : 1)];
  alignas(V) std::byte m_valueStorage[sizeof(V) * (Capacity ? Capacity : 1)];
  size_type m_size = 0;
};

} // namespace strobe
