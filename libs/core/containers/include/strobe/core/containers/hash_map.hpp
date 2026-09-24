#pragma once

#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <iterator>
#include <limits>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>

#include <strobe/memory.hpp>

namespace strobe {

template <typename K, typename V, typename Hash = std::hash<K>,
          typename Equal = std::equal_to<K>, Allocator A = Mallocator>
class HashMap {
  using Entry = std::pair<K, V>;
  using ATraits = AllocatorTraits<A>;

  static constexpr std::uint8_t EMPTY = 0;
  static constexpr std::uint8_t DELETED = 1;

public:
  using size_type = std::size_t;

  template <bool Const> class Iterator {
    using Map = std::conditional_t<Const, const HashMap, HashMap>;
    using Value = std::conditional_t<Const, const V, V>;

    friend class HashMap;
    template <bool> friend class Iterator;

    Iterator(Map *map, size_type index) : m_map(map), m_index(index) {
      skip_empty();
    }

    void skip_empty() {
      while (m_index < m_map->m_capacity &&
             m_map->m_control[m_index] <= DELETED)
        ++m_index;
    }

  public:
    using iterator_category = std::input_iterator_tag;
    using iterator_concept = std::input_iterator_tag;
    using value_type = Entry;
    using difference_type = std::ptrdiff_t;
    using reference = std::pair<const K &, Value &>;

    Iterator() = default;
    Iterator(const Iterator &) = default;
    Iterator &operator=(const Iterator &) = default;

    Iterator(const Iterator<false> &other)
      requires Const
        : m_map(other.m_map), m_index(other.m_index) {}

    reference operator*() const {
      auto &entry = m_map->m_entries[m_index];
      return {entry.first, entry.second};
    }

    Iterator &operator++() {
      ++m_index;
      skip_empty();
      return *this;
    }

    Iterator operator++(int) {
      Iterator old = *this;
      ++*this;
      return old;
    }

    template <bool OtherConst>
    bool operator==(const Iterator<OtherConst> &other) const {
      return m_map == other.m_map && m_index == other.m_index;
    }

  private:
    Map *m_map = nullptr;
    size_type m_index = 0;
  };

  using iterator = Iterator<false>;
  using const_iterator = Iterator<true>;

  explicit HashMap(const A &allocator = {}, const Hash &hash = {},
                   const Equal &equal = {})
      : m_hash(hash), m_equal(equal), m_allocator(allocator) {}

  HashMap(const HashMap &other)
      : m_hash(other.m_hash), m_equal(other.m_equal),
        m_allocator(
            ATraits::select_on_container_copy_construction(other.m_allocator)) {
    reserve(other.m_size);

    for (const auto [key, value] : other)
      try_emplace(key, value);
  }

  HashMap(HashMap &&other)
      : m_hash(std::move(other.m_hash)), m_equal(std::move(other.m_equal)),
        m_allocator(std::move(other.m_allocator)) {
    steal(other);
  }

  ~HashMap() { reset(); }

  HashMap &operator=(const HashMap &other) {
    if (this == &other)
      return *this;

    if constexpr (ATraits::propagate_on_container_copy_assignment) {
      if (!strobe::alloc_equals(m_allocator, other.m_allocator))
        reset();

      m_allocator = other.m_allocator;
    }

    clear();
    m_hash = other.m_hash;
    m_equal = other.m_equal;
    reserve(other.m_size);

    for (const auto [key, value] : other)
      try_emplace(key, value);

    return *this;
  }

  HashMap &operator=(HashMap &&other) {
    if (this == &other)
      return *this;

    if constexpr (ATraits::propagate_on_container_move_assignment) {
      reset();
      m_allocator = std::move(other.m_allocator);
      m_hash = std::move(other.m_hash);
      m_equal = std::move(other.m_equal);
      steal(other);
    } else if (strobe::alloc_equals(m_allocator, other.m_allocator)) {
      reset();
      m_hash = std::move(other.m_hash);
      m_equal = std::move(other.m_equal);
      steal(other);
    } else {
      clear();
      m_hash = std::move(other.m_hash);
      m_equal = std::move(other.m_equal);
      reserve(other.m_size);

      for (size_type i = 0; i < other.m_capacity; ++i) {
        if (other.m_control[i] > DELETED) {
          auto &entry = other.m_entries[i];
          try_emplace(std::move(entry.first), std::move(entry.second));
        }
      }

      other.clear();
    }

    return *this;
  }

  template <typename... Args>
  std::pair<iterator, bool> try_emplace(const K &key, Args &&...args) {
    return try_emplace_impl(key, std::forward<Args>(args)...);
  }

  template <typename... Args>
  std::pair<iterator, bool> try_emplace(K &&key, Args &&...args) {
    return try_emplace_impl(std::move(key), std::forward<Args>(args)...);
  }

  std::pair<iterator, bool> insert(const K &key, const V &value) {
    return try_emplace(key, value);
  }

  std::pair<iterator, bool> insert(const K &key, V &&value) {
    return try_emplace(key, std::move(value));
  }

  std::pair<iterator, bool> insert(K &&key, const V &value) {
    return try_emplace(std::move(key), value);
  }

  std::pair<iterator, bool> insert(K &&key, V &&value) {
    return try_emplace(std::move(key), std::move(value));
  }

  V &operator[](const K &key)
    requires std::default_initializable<V>
  {
    return (*try_emplace(key).first).second;
  }

  V &operator[](K &&key)
    requires std::default_initializable<V>
  {
    return (*try_emplace(std::move(key)).first).second;
  }

  iterator find(const K &key) { return iterator(this, find_index(key)); }

  const_iterator find(const K &key) const {
    return const_iterator(this, find_index(key));
  }

  [[nodiscard]]
  bool contains(const K &key) const {
    return find_index(key) != m_capacity;
  }

  bool erase(const K &key) {
    const size_type index = find_index(key);
    if (index == m_capacity)
      return false;

    erase_index(index);
    return true;
  }

  iterator erase(const_iterator pos) {
    assert(pos.m_map == this);
    assert(pos.m_index < m_capacity);
    assert(m_control[pos.m_index] > DELETED);

    const size_type index = pos.m_index;
    erase_index(index);
    return iterator(this, index + 1);
  }

  void clear() noexcept {
    for (size_type i = 0; i < m_capacity; ++i) {
      if (m_control[i] > DELETED)
        std::destroy_at(m_entries + i);
    }

    if (m_control)
      std::memset(m_control, EMPTY, m_capacity);

    m_size = 0;
    m_deleted = 0;
  }

  // count is the desired number of elements, not the number of slots.
  void reserve(size_type count) {
    if (count <= max_load(m_capacity))
      return;

    size_type newCapacity = m_capacity ? m_capacity : 8;

    while (count > max_load(newCapacity)) {
      assert(newCapacity <= max_slot_count() / 2);
      newCapacity *= 2;
    }

    rehash(newCapacity);
  }

  [[nodiscard]] size_type size() const noexcept { return m_size; }
  [[nodiscard]] size_type capacity() const noexcept { return m_capacity; }
  [[nodiscard]] bool empty() const noexcept { return m_size == 0; }

  iterator begin() noexcept { return iterator(this, 0); }
  iterator end() noexcept { return iterator(this, m_capacity); }

  const_iterator begin() const noexcept { return const_iterator(this, 0); }

  const_iterator end() const noexcept {
    return const_iterator(this, m_capacity);
  }

  const_iterator cbegin() const noexcept { return begin(); }
  const_iterator cend() const noexcept { return end(); }

private:
  template <typename Key, typename... Args>
    requires std::same_as<std::remove_cvref_t<Key>, K> &&
             std::constructible_from<V, Args &&...>
  std::pair<iterator, bool> try_emplace_impl(Key &&key, Args &&...args) {
    const size_type hash = mixed_hash(key);
    size_type index;

    if (m_capacity != 0) {
      auto [slot, found] = find_or_slot(key, hash);
      if (found)
        return {iterator(this, slot), false};

      index = slot;
    }

    if (needs_rehash()) {
      // Arguments may refer to entries that rehash moves.
      K stagedKey(std::forward<Key>(key));
      V stagedValue(std::forward<Args>(args)...);

      prepare_insert();
      index = first_available(hash);
      construct_entry(index, hash, std::move(stagedKey),
                      std::move(stagedValue));
    } else {
      construct_entry(index, hash, std::forward<Key>(key),
                      std::forward<Args>(args)...);
    }

    return {iterator(this, index), true};
  }

  static constexpr size_type max_slot_count() noexcept {
    return std::numeric_limits<size_type>::max() / sizeof(Entry);
  }

  static constexpr size_type max_load(size_type capacity) noexcept {
    // At most 75% occupied. There is always an empty slot.
    return capacity - capacity / 4;
  }

  static size_type mix(size_type hash) noexcept {
    if constexpr (sizeof(size_type) == 8) {
      hash ^= hash >> 33;
      hash *= UINT64_C(0xff51afd7ed558ccd);
      hash ^= hash >> 33;
      hash *= UINT64_C(0xc4ceb9fe1a85ec53);
      hash ^= hash >> 33;
    } else {
      hash ^= hash >> 16;
      hash *= UINT32_C(0x85ebca6b);
      hash ^= hash >> 13;
      hash *= UINT32_C(0xc2b2ae35);
      hash ^= hash >> 16;
    }

    return hash;
  }

  size_type mixed_hash(const K &key) const {
    return mix(static_cast<size_type>(m_hash(key)));
  }

  static std::uint8_t fingerprint(size_type hash) noexcept {
    constexpr unsigned shift = std::numeric_limits<size_type>::digits - 7;
    return static_cast<std::uint8_t>(2 + ((hash >> shift) & 0x7f));
  }

  size_type find_index(const K &key) const {
    if (m_capacity == 0)
      return 0;

    const size_type hash = mixed_hash(key);
    const std::uint8_t tag = fingerprint(hash);
    size_type index = hash & (m_capacity - 1);

    for (;;) {
      const std::uint8_t control = m_control[index];

      if (control == EMPTY)
        return m_capacity;

      if (control == tag && m_equal(m_entries[index].first, key))
        return index;

      index = (index + 1) & (m_capacity - 1);
    }
  }

  // Returns an existing entry, or a slot for a new entry.
  std::pair<size_type, bool> find_or_slot(const K &key, size_type hash) const {
    size_type index = hash & (m_capacity - 1);
    size_type deleted = m_capacity;
    const std::uint8_t tag = fingerprint(hash);

    for (;;) {
      const std::uint8_t control = m_control[index];

      if (control == EMPTY)
        return {deleted == m_capacity ? index : deleted, false};

      if (control == DELETED) {
        if (deleted == m_capacity)
          deleted = index;
      } else if (control == tag && m_equal(m_entries[index].first, key)) {
        return {index, true};
      }

      index = (index + 1) & (m_capacity - 1);
    }
  }

  size_type first_available(size_type hash) const {
    size_type index = hash & (m_capacity - 1);
    size_type deleted = m_capacity;

    for (;;) {
      const std::uint8_t control = m_control[index];

      if (control == EMPTY)
        return deleted == m_capacity ? index : deleted;

      if (control == DELETED && deleted == m_capacity)
        deleted = index;

      index = (index + 1) & (m_capacity - 1);
    }
  }

  template <typename Key, typename... Args>
  void construct_entry(size_type index, size_type hash, Key &&key,
                       Args &&...args) {
    assert(m_control[index] <= DELETED);

    const bool reusedDeleted = m_control[index] == DELETED;

    std::construct_at(m_entries + index, std::piecewise_construct,
                      std::forward_as_tuple(std::forward<Key>(key)),
                      std::forward_as_tuple(std::forward<Args>(args)...));

    m_control[index] = fingerprint(hash);
    ++m_size;

    if (reusedDeleted)
      --m_deleted;
  }

  void erase_index(size_type index) {
    std::destroy_at(m_entries + index);
    m_control[index] = DELETED;
    --m_size;
    ++m_deleted;

    // An empty table does not need tombstones.
    if (m_size == 0) {
      std::memset(m_control, EMPTY, m_capacity);
      m_deleted = 0;
    }
  }

  bool needs_rehash() const noexcept {
    return m_capacity == 0 || m_size + m_deleted + 1 > max_load(m_capacity);
  }

  void prepare_insert() {
    if (m_capacity == 0) {
      rehash(8);
    } else if (m_size + 1 > max_load(m_capacity)) {
      assert(m_capacity <= max_slot_count() / 2);
      rehash(m_capacity * 2);
    } else {
      // Tombstones, rather than live entries, caused the threshold.
      rehash(m_capacity);
    }
  }

  void rehash(size_type newCapacity) {
    assert(newCapacity >= 8);
    assert((newCapacity & (newCapacity - 1)) == 0);
    assert(newCapacity <= max_slot_count());

    auto [newControl, controlAllocation] =
        ATraits::template allocate_at_least<std::uint8_t>(m_allocator,
                                                          newCapacity);

    auto [newEntries, entryAllocation] =
        ATraits::template allocate_at_least<Entry>(m_allocator, newCapacity);

    assert(newControl != nullptr && controlAllocation >= newCapacity);
    assert(newEntries != nullptr && entryAllocation >= newCapacity);

    std::memset(newControl, EMPTY, newCapacity);

    for (size_type i = 0; i < m_capacity; ++i) {
      if (m_control[i] <= DELETED)
        continue;

      const size_type hash = mixed_hash(m_entries[i].first);
      size_type index = hash & (newCapacity - 1);

      while (newControl[index] != EMPTY)
        index = (index + 1) & (newCapacity - 1);

      std::construct_at(newEntries + index, std::move(m_entries[i]));
      newControl[index] = fingerprint(hash);
      std::destroy_at(m_entries + i);
    }

    if (m_control) {
      ATraits::template deallocate<std::uint8_t>(m_allocator, m_control,
                                                 m_controlAllocation);

      ATraits::template deallocate<Entry>(m_allocator, m_entries,
                                          m_entryAllocation);
    }

    m_control = newControl;
    m_entries = newEntries;
    m_capacity = newCapacity;
    m_controlAllocation = controlAllocation;
    m_entryAllocation = entryAllocation;
    m_deleted = 0;
  }

  void steal(HashMap &other) noexcept {
    m_control = std::exchange(other.m_control, nullptr);
    m_entries = std::exchange(other.m_entries, nullptr);
    m_capacity = std::exchange(other.m_capacity, 0);
    m_size = std::exchange(other.m_size, 0);
    m_deleted = std::exchange(other.m_deleted, 0);
    m_controlAllocation = std::exchange(other.m_controlAllocation, 0);
    m_entryAllocation = std::exchange(other.m_entryAllocation, 0);
  }

  void reset() noexcept {
    clear();

    if (m_control) {
      ATraits::template deallocate<std::uint8_t>(m_allocator, m_control,
                                                 m_controlAllocation);

      ATraits::template deallocate<Entry>(m_allocator, m_entries,
                                          m_entryAllocation);
    }

    m_control = nullptr;
    m_entries = nullptr;
    m_capacity = 0;
    m_controlAllocation = 0;
    m_entryAllocation = 0;
  }

  std::uint8_t *m_control = nullptr;
  Entry *m_entries = nullptr;

  size_type m_capacity = 0;
  size_type m_size = 0;
  size_type m_deleted = 0;
  size_type m_controlAllocation = 0;
  size_type m_entryAllocation = 0;

  [[no_unique_address]] Hash m_hash;
  [[no_unique_address]] Equal m_equal;
  [[no_unique_address]] A m_allocator;
};

} // namespace strobe
