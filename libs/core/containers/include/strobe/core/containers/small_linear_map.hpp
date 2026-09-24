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

#include "strobe/core/containers/span.hpp"

namespace strobe {

template <std::equality_comparable K, typename V,
          std::size_t MinSVOCapacity = 8, Allocator A = Mallocator>
class SmallLinearMap {
  using ATraits = AllocatorTraits<A>;

public:
  using size_type = std::size_t;

  template <bool Const> class Iterator {
    using Value = std::conditional_t<Const, const V, V>;

    friend class SmallLinearMap;
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

  explicit SmallLinearMap(const A &allocator = {}) : m_allocator(allocator) {}

  SmallLinearMap(const SmallLinearMap &other)
      : m_allocator(
            ATraits::select_on_container_copy_construction(other.m_allocator)) {
    reserve(other.m_size);

    for (size_type i = 0; i < other.m_size; ++i)
      append(other.key_data()[i], other.value_data()[i]);
  }

  SmallLinearMap(SmallLinearMap &&other)
      : m_allocator(std::move(other.m_allocator)) {
    take_or_move(other);
  }

  ~SmallLinearMap() { reset(); }

  SmallLinearMap &operator=(const SmallLinearMap &other) {
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
      append(other.key_data()[i], other.value_data()[i]);

    return *this;
  }

  SmallLinearMap &operator=(SmallLinearMap &&other) {
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
        append(std::move(other.key_data()[i]),
               std::move(other.value_data()[i]));

      other.clear();
    }

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
  bool empty() const noexcept {
    return m_size == 0;
  }

  [[nodiscard]]
  size_type capacity() const noexcept {
    if (!m_heapKeys)
      return MinSVOCapacity;

    return std::min(m_keyCapacity, m_valueCapacity);
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

  void reserve(size_type count) {
    if (count <= capacity())
      return;

    assert(count <= max_size());

    auto [newKeys, keyCapacity] =
        ATraits::template allocate_at_least<K>(m_allocator, count);

    auto [newValues, valueCapacity] =
        ATraits::template allocate_at_least<V>(m_allocator, count);

    assert(newKeys != nullptr && keyCapacity >= count);
    assert(newValues != nullptr && valueCapacity >= count);

    K *oldKeys = key_data();
    V *oldValues = value_data();
    const size_type oldSize = m_size;

    for (size_type i = 0; i < oldSize; ++i) {
      std::construct_at(newKeys + i, std::move(oldKeys[i]));
      std::construct_at(newValues + i, std::move(oldValues[i]));
    }

    clear();

    if (m_heapKeys) {
      ATraits::template deallocate<K>(m_allocator, m_heapKeys, m_keyCapacity);
      ATraits::template deallocate<V>(m_allocator, m_heapValues,
                                      m_valueCapacity);
    }

    m_heapKeys = newKeys;
    m_heapValues = newValues;
    m_keyCapacity = keyCapacity;
    m_valueCapacity = valueCapacity;
    m_size = oldSize;
  }

private:
  static constexpr size_type max_size() noexcept {
    return std::min(std::numeric_limits<size_type>::max() / sizeof(K),
                    std::numeric_limits<size_type>::max() / sizeof(V));
  }

  K *inline_keys() noexcept { return reinterpret_cast<K *>(m_inlineKeys); }

  const K *inline_keys() const noexcept {
    return reinterpret_cast<const K *>(m_inlineKeys);
  }

  V *inline_values() noexcept { return reinterpret_cast<V *>(m_inlineValues); }

  const V *inline_values() const noexcept {
    return reinterpret_cast<const V *>(m_inlineValues);
  }

  K *key_data() noexcept { return m_heapKeys ? m_heapKeys : inline_keys(); }

  const K *key_data() const noexcept {
    return m_heapKeys ? m_heapKeys : inline_keys();
  }

  V *value_data() noexcept {
    return m_heapKeys ? m_heapValues : inline_values();
  }

  const V *value_data() const noexcept {
    return m_heapKeys ? m_heapValues : inline_values();
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
    assert(m_size < max_size());

    // Either argument may refer to an existing element that reserve moves.
    K stagedKey(std::forward<Key>(key));
    V stagedValue(std::forward<Value>(value));

    if (m_size == capacity()) {
      const size_type next = capacity() <= max_size() / 2
                                 ? std::max<size_type>(1, capacity() * 2)
                                 : max_size();

      reserve(next);
    }

    std::construct_at(key_data() + m_size, std::move(stagedKey));
    std::construct_at(value_data() + m_size, std::move(stagedValue));
    ++m_size;
  }

  void take_or_move(SmallLinearMap &other) {
    if (other.m_heapKeys) {
      m_heapKeys = std::exchange(other.m_heapKeys, nullptr);
      m_heapValues = std::exchange(other.m_heapValues, nullptr);
      m_keyCapacity = std::exchange(other.m_keyCapacity, 0);
      m_valueCapacity = std::exchange(other.m_valueCapacity, 0);
      m_size = std::exchange(other.m_size, 0);
      return;
    }

    for (size_type i = 0; i < other.m_size; ++i)
      append(std::move(other.key_data()[i]), std::move(other.value_data()[i]));

    other.clear();
  }

  void reset() noexcept {
    clear();

    if (m_heapKeys) {
      ATraits::template deallocate<K>(m_allocator, m_heapKeys, m_keyCapacity);
      ATraits::template deallocate<V>(m_allocator, m_heapValues,
                                      m_valueCapacity);
    }

    m_heapKeys = nullptr;
    m_heapValues = nullptr;
    m_keyCapacity = 0;
    m_valueCapacity = 0;
  }

  alignas(K) std::byte
      m_inlineKeys[sizeof(K) * (MinSVOCapacity ? MinSVOCapacity : 1)];

  alignas(V) std::byte
      m_inlineValues[sizeof(V) * (MinSVOCapacity ? MinSVOCapacity : 1)];

  K *m_heapKeys = nullptr;
  V *m_heapValues = nullptr;
  size_type m_size = 0;
  size_type m_keyCapacity = 0;
  size_type m_valueCapacity = 0;

  [[no_unique_address]]
  A m_allocator;
};

} // namespace strobe
