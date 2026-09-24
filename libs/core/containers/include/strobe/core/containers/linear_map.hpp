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

template <std::equality_comparable K, typename V, Allocator A = Mallocator>
class LinearMap {
  using ATraits = AllocatorTraits<A>;

 public:
  using key_type = K;
  using mapped_type = V;
  using value_type = std::pair<const K, V>;
  using size_type = std::size_t;

  template <bool Const>
  class Iterator {
    using Value = std::conditional_t<Const, const V, V>;

    friend class LinearMap;
    template <bool>
    friend class Iterator;

    Iterator(const K* key, Value* value)
        : m_key(key), m_value(value) {}

   public:
    using iterator_category = std::input_iterator_tag;
    using iterator_concept = std::input_iterator_tag;
    using value_type = LinearMap::value_type;
    using difference_type = std::ptrdiff_t;
    using reference = std::pair<const K&, Value&>;

    Iterator() = default;
    Iterator(const Iterator&) = default;
    Iterator& operator=(const Iterator&) = default;

    Iterator(const Iterator<false>& other)
      requires Const
        : m_key(other.m_key), m_value(other.m_value) {}

    reference operator*() const {
      return {*m_key, *m_value};
    }

    Iterator& operator++() {
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
    bool operator==(const Iterator<OtherConst>& other) const {
      return m_key == other.m_key;
    }

   private:
    const K* m_key = nullptr;
    Value* m_value = nullptr;
  };

  using iterator = Iterator<false>;
  using const_iterator = Iterator<true>;

  explicit LinearMap(const A& allocator = {})
      : m_allocator(allocator) {}

  LinearMap(const LinearMap& other)
      : m_allocator(
            ATraits::select_on_container_copy_construction(other.m_allocator)) {
    reserve(other.m_size);

    for (size_type i = 0; i < other.m_size; ++i)
      append(other.m_keys[i], other.m_values[i]);
  }

  LinearMap(LinearMap&& other)
      noexcept(std::is_nothrow_move_constructible_v<A>)
      : m_keys(std::exchange(other.m_keys, nullptr)),
        m_values(std::exchange(other.m_values, nullptr)),
        m_size(std::exchange(other.m_size, 0)),
        m_keyCapacity(std::exchange(other.m_keyCapacity, 0)),
        m_valueCapacity(std::exchange(other.m_valueCapacity, 0)),
        m_allocator(std::move(other.m_allocator)) {}

  ~LinearMap() {
    reset();
  }

  LinearMap& operator=(const LinearMap& other) {
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
      append(other.m_keys[i], other.m_values[i]);

    return *this;
  }

  LinearMap& operator=(LinearMap&& other) {
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
        append(std::move(other.m_keys[i]),
               std::move(other.m_values[i]));

      other.clear();
    }

    return *this;
  }

  iterator find(const K& key) {
    return iterator_at(find_index(key));
  }

  const_iterator find(const K& key) const {
    return iterator_at(find_index(key));
  }

  [[nodiscard]]
  bool contains(const K& key) const {
    return find_index(key) != m_size;
  }

  // Keep this spelling if existing Strobe callers use it.
  [[nodiscard]]
  bool containsKey(const K& key) const {
    return contains(key);
  }

  template <typename Key, typename... Args>
    requires std::same_as<std::remove_cvref_t<Key>, K> &&
             std::constructible_from<K, Key&&> &&
             std::constructible_from<V, Args&&...>
  std::pair<iterator, bool> try_emplace(Key&& key, Args&&... args) {
    const size_type index = find_index(key);
    if (index != m_size)
      return {iterator_at(index), false};

    append(std::forward<Key>(key), std::forward<Args>(args)...);
    return {iterator_at(m_size - 1), true};
  }

  std::pair<iterator, bool> insert(const K& key, const V& value) {
    return try_emplace(key, value);
  }

  std::pair<iterator, bool> insert(const K& key, V&& value) {
    return try_emplace(key, std::move(value));
  }

  std::pair<iterator, bool> insert(K&& key, const V& value) {
    return try_emplace(std::move(key), value);
  }

  std::pair<iterator, bool> insert(K&& key, V&& value) {
    return try_emplace(std::move(key), std::move(value));
  }

  template <typename Key, typename M>
    requires std::same_as<std::remove_cvref_t<Key>, K> &&
             std::constructible_from<K, Key&&> &&
             std::constructible_from<V, M&&> &&
             std::assignable_from<V&, M&&>
  std::pair<iterator, bool> insert_or_assign(Key&& key, M&& value) {
    const size_type index = find_index(key);

    if (index != m_size) {
      m_values[index] = std::forward<M>(value);
      return {iterator_at(index), false};
    }

    append(std::forward<Key>(key), std::forward<M>(value));
    return {iterator_at(m_size - 1), true};
  }

  V& operator[](const K& key)
    requires std::default_initializable<V>
  {
    return (*try_emplace(key).first).second;
  }

  V& operator[](K&& key)
    requires std::default_initializable<V>
  {
    return (*try_emplace(std::move(key)).first).second;
  }

  // Erasure is unordered: the last entry moves into the erased position.
  iterator erase(const_iterator pos) {
    assert(m_size != 0);
    assert(pos.m_key >= m_keys && pos.m_key < m_keys + m_size);
    assert(pos.m_value == m_values + (pos.m_key - m_keys));

    const size_type index =
        static_cast<size_type>(pos.m_key - m_keys);

    erase_index(index);
    return iterator_at(index);
  }

  size_type erase(const K& key) {
    const size_type index = find_index(key);
    if (index == m_size)
      return 0;

    erase_index(index);
    return 1;
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
    return std::min(m_keyCapacity, m_valueCapacity);
  }

  [[nodiscard]]
  span<const K> keys() const noexcept {
    return {m_keys, m_size};
  }

  [[nodiscard]]
  span<V> values() noexcept {
    return {m_values, m_size};
  }

  [[nodiscard]]
  span<const V> values() const noexcept {
    return {m_values, m_size};
  }

  iterator begin() noexcept {
    return {m_keys, m_values};
  }

  iterator end() noexcept {
    return iterator_at(m_size);
  }

  const_iterator begin() const noexcept {
    return {m_keys, m_values};
  }

  const_iterator end() const noexcept {
    return iterator_at(m_size);
  }

  const_iterator cbegin() const noexcept {
    return begin();
  }

  const_iterator cend() const noexcept {
    return end();
  }

  void clear() noexcept {
    if (m_size != 0) {
      std::destroy_n(m_keys, m_size);
      std::destroy_n(m_values, m_size);
    }

    m_size = 0;
  }

  // count is an element count, as with std::unordered_map::reserve.
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

    const size_type oldSize = m_size;

    for (size_type i = 0; i < oldSize; ++i) {
      std::construct_at(newKeys + i, std::move(m_keys[i]));
      std::construct_at(newValues + i, std::move(m_values[i]));
    }

    clear();

    if (m_keys)
      ATraits::template deallocate<K>(
          m_allocator, m_keys, m_keyCapacity);

    if (m_values)
      ATraits::template deallocate<V>(
          m_allocator, m_values, m_valueCapacity);

    m_keys = newKeys;
    m_values = newValues;
    m_size = oldSize;
    m_keyCapacity = keyCapacity;
    m_valueCapacity = valueCapacity;
  }

 private:
  static constexpr size_type max_size() noexcept {
    return std::min(
        std::numeric_limits<size_type>::max() / sizeof(K),
        std::numeric_limits<size_type>::max() / sizeof(V));
  }

  size_type find_index(const K& key) const {
    for (size_type i = 0; i < m_size; ++i) {
      if (m_keys[i] == key)
        return i;
    }

    return m_size;
  }

  iterator iterator_at(size_type index) noexcept {
    return {
        m_keys ? m_keys + index : nullptr,
        m_values ? m_values + index : nullptr,
    };
  }

  const_iterator iterator_at(size_type index) const noexcept {
    return {
        m_keys ? m_keys + index : nullptr,
        m_values ? m_values + index : nullptr,
    };
  }

  template <typename Key, typename... Args>
  void append(Key&& key, Args&&... args) {
    assert(m_size < max_size());

    if (m_size == capacity()) {
      // The arguments may refer to elements that reserve() will move.
      K stagedKey(std::forward<Key>(key));
      V stagedValue(std::forward<Args>(args)...);

      const size_type current = capacity();
      const size_type next = current <= max_size() / 2
          ? std::max<size_type>(1, current * 2)
          : max_size();

      reserve(next);

      std::construct_at(m_keys + m_size, std::move(stagedKey));
      std::construct_at(m_values + m_size, std::move(stagedValue));
    } else {
      std::construct_at(
          m_keys + m_size, std::forward<Key>(key));
      std::construct_at(
          m_values + m_size, std::forward<Args>(args)...);
    }

    ++m_size;
  }

  void erase_index(size_type index) {
    const size_type last = m_size - 1;

    if (index != last) {
      m_keys[index] = std::move(m_keys[last]);
      m_values[index] = std::move(m_values[last]);
    }

    std::destroy_at(m_keys + last);
    std::destroy_at(m_values + last);
    --m_size;
  }

  void steal(LinearMap& other) noexcept {
    m_keys = std::exchange(other.m_keys, nullptr);
    m_values = std::exchange(other.m_values, nullptr);
    m_size = std::exchange(other.m_size, 0);
    m_keyCapacity = std::exchange(other.m_keyCapacity, 0);
    m_valueCapacity = std::exchange(other.m_valueCapacity, 0);
  }

  void reset() noexcept {
    clear();

    if (m_keys)
      ATraits::template deallocate<K>(
          m_allocator, m_keys, m_keyCapacity);

    if (m_values)
      ATraits::template deallocate<V>(
          m_allocator, m_values, m_valueCapacity);

    m_keys = nullptr;
    m_values = nullptr;
    m_keyCapacity = 0;
    m_valueCapacity = 0;
  }

  K* m_keys = nullptr;
  V* m_values = nullptr;
  size_type m_size = 0;
  size_type m_keyCapacity = 0;
  size_type m_valueCapacity = 0;

  [[no_unique_address]]
  A m_allocator;
};

} // namespace strobe
