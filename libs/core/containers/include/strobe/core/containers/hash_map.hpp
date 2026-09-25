#pragma once

#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <fmt/format.h>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <memory>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>

#include <strobe/memory.hpp>

namespace strobe {

/**
 * \ingroup core
 * \brief Hash table with open addressing and tombstones.
 * \code{.cpp}
 * template<typename K, typename V, typename Hash = std::hash<K>,
 *          typename Equal = std::equal_to<K>, Allocator A = Mallocator>
 * class HashMap;
 * \endcode
 *
 * Stores key-value pairs using linear probing. Iteration order is unspecified.
 *
 * \attention 1. Keys must remain compatible with Hash and Equal while stored.
 * \attention 2. HashMap operations invalidate iterators when rehashing occurs.
 * \attention 3. Binary capacity is managed internally and may exceed size().
 */
template <typename K, typename V, typename Hash = std::hash<K>,
          typename Equal = std::equal_to<K>, Allocator A = Mallocator>
class HashMap {
  using Entry = std::pair<K, V>;
  using ATraits = AllocatorTraits<A>;

  static constexpr std::uint8_t EMPTY = 0;
  static constexpr std::uint8_t DELETED = 1;

public:
  using key_type = K;
  using mapped_type = V;
  using value_type = std::pair<const K, V>;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using hasher = Hash;
  using key_equal = Equal;
  using allocator_type = A;

  /**
   * \brief Iterator over map entries.
   * \code{.cpp}
   * template<bool Const>
   * class Iterator;
   * \endcode
   *
   * Iterators expose entries as a pair of key and value references.
   *
   * \attention 1. Iterator validity follows the invalidation rules of HashMap.
   */
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

    /**
     * \brief Constructs an iterator.
     * \code{.cpp}
     * Iterator() = default;
     * Iterator(const Iterator&) = default;
     * Iterator& operator=(const Iterator&) = default;
     * Iterator(const Iterator<false>& other) requires Const;
     * \endcode
     *
     * \param other Iterator to copy or convert.
     */
    Iterator() = default;
    Iterator(const Iterator &) = default;
    Iterator &operator=(const Iterator &) = default;

    Iterator(const Iterator<false> &other)
      requires Const
        : m_map(other.m_map), m_index(other.m_index) {}

    /**
     * \brief Accesses the current entry.
     * \code{.cpp}
     * reference operator*() const;
     * \endcode
     *
     * \return The current key-value pair.
     *
     * \attention 1. The iterator must refer to an element.
     */
    reference operator*() const {
      auto &entry = m_map->m_entries[m_index];
      return {entry.first, entry.second};
    }

    /**
     * \brief Advances the iterator.
     * \code{.cpp}
     * Iterator& operator++();
     * Iterator operator++(int);
     * \endcode
     *
     * \return The advanced iterator or its previous value.
     */
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

    /**
     * \brief Compares two iterators.
     * \code{.cpp}
     * template<bool OtherConst>
     * bool operator==(const Iterator<OtherConst>& other) const;
     * \endcode
     *
     * \param other Iterator to compare with.
     *
     * \return Whether both iterators refer to the same position.
     */
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

  /**
   * \brief Constructs, copies, or moves a map.
   * \code{.cpp}
   * explicit HashMap(const A& allocator = {}, const Hash& hash = {},
   *                  const Equal& equal = {});
   * HashMap(const HashMap& other);
   * HashMap(HashMap&& other);
   * HashMap& operator=(const HashMap& other);
   * HashMap& operator=(HashMap&& other);
   * ~HashMap();
   * \endcode
   *
   * \param allocator Allocator to use.
   * \param hash Hash function.
   * \param equal Key equality predicate.
   * \param other Map to copy or move from.
   *
   * Moved-from maps are empty.
   */
  explicit HashMap(const A &allocator = {}, const Hash &hash = {},
                   const Equal &equal = {})
      : m_hash(hash), m_equal(equal), m_allocator(allocator) {}

  HashMap(const HashMap &other)
      : m_hash(other.m_hash), m_equal(other.m_equal),
        m_allocator(
            ATraits::select_on_container_copy_construction(other.m_allocator)),
        m_maxLoadFactor(other.m_maxLoadFactor) {
    reserve(other.m_size);

    for (const auto [key, value] : other)
      try_emplace(key, value);
  }

  HashMap(HashMap &&other)
      : m_hash(std::move(other.m_hash)), m_equal(std::move(other.m_equal)),
        m_allocator(std::move(other.m_allocator)),
        m_maxLoadFactor(other.m_maxLoadFactor) {
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
    m_maxLoadFactor = other.m_maxLoadFactor;
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
      m_maxLoadFactor = other.m_maxLoadFactor;
      steal(other);
    } else if (strobe::alloc_equals(m_allocator, other.m_allocator)) {
      reset();
      m_hash = std::move(other.m_hash);
      m_equal = std::move(other.m_equal);
      m_maxLoadFactor = other.m_maxLoadFactor;
      steal(other);
    } else {
      clear();
      m_hash = std::move(other.m_hash);
      m_equal = std::move(other.m_equal);
      m_maxLoadFactor = other.m_maxLoadFactor;
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

  /**
   * \brief Inserts a value if the key is absent.
   * \code{.cpp}
   * template<typename... Args>
   * std::pair<iterator, bool> try_emplace(const K& key, Args&&... args);
   * template<typename... Args>
   * std::pair<iterator, bool> try_emplace(K&& key, Args&&... args);
   * \endcode
   *
   * \param key Key to insert.
   * \param args Arguments forwarded to construct the value.
   *
   * \return Iterator to the entry and whether insertion occurred.
   */
  template <typename... Args>
  std::pair<iterator, bool> try_emplace(const K &key, Args &&...args) {
    return try_emplace_impl(key, std::forward<Args>(args)...);
  }

  template <typename... Args>
  std::pair<iterator, bool> try_emplace(K &&key, Args &&...args) {
    return try_emplace_impl(std::move(key), std::forward<Args>(args)...);
  }

  /**
   * \brief Inserts a key-value pair if the key is absent.
   * \code{.cpp}
   * std::pair<iterator, bool> insert(const K& key, const V& value);
   * std::pair<iterator, bool> insert(const K& key, V&& value);
   * std::pair<iterator, bool> insert(K&& key, const V& value);
   * std::pair<iterator, bool> insert(K&& key, V&& value);
   * std::pair<iterator, bool> insert(const value_type& value);
   * std::pair<iterator, bool> insert(value_type&& value);
   * template<class InputIt> void insert(InputIt first, InputIt last);
   * void insert(std::initializer_list<value_type> values);
   * \endcode
   *
   * \param key Key to insert.
   * \param value Value to insert.
   *
   * \return Iterator to the entry and whether insertion occurred.
   */
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

  std::pair<iterator, bool> insert(const value_type &value) {
    return try_emplace(value.first, value.second);
  }

  std::pair<iterator, bool> insert(value_type &&value) {
    return try_emplace(value.first, std::move(value.second));
  }

  template <std::input_iterator InputIt>
  void insert(InputIt first, InputIt last) {
    for (; first != last; ++first)
      insert(*first);
  }

  void insert(std::initializer_list<value_type> values) {
    insert(values.begin(), values.end());
  }

  /**
   * \brief Constructs an entry if its key is absent.
   * \code{.cpp}
   * template<class... Args> std::pair<iterator, bool> emplace(Args&&... args);
   * \endcode
   * \param args Arguments used to construct value_type.
   * \return Iterator to the entry and whether insertion occurred.
   */
  template <typename... Args>
    requires std::constructible_from<value_type, Args &&...>
  std::pair<iterator, bool> emplace(Args &&...args) {
    value_type value(std::forward<Args>(args)...);
    return insert(std::move(value));
  }

  /**
   * \brief Inserts a value or assigns it to an existing key.
   * \code{.cpp}
   * template<class M> std::pair<iterator, bool> insert_or_assign(const K& key, M&& value);
   * template<class M> std::pair<iterator, bool> insert_or_assign(K&& key, M&& value);
   * \endcode
   */
  template <typename M>
  std::pair<iterator, bool> insert_or_assign(const K &key, M &&value) {
    auto [it, inserted] = try_emplace(key, std::forward<M>(value));
    if (!inserted)
      (*it).second = std::forward<M>(value);
    return {it, inserted};
  }

  template <typename M>
  std::pair<iterator, bool> insert_or_assign(K &&key, M &&value) {
    auto [it, inserted] = try_emplace(std::move(key), std::forward<M>(value));
    if (!inserted)
      (*it).second = std::forward<M>(value);
    return {it, inserted};
  }

  /**
   * \brief Accesses or inserts a value by key.
   * \code{.cpp}
   * V& operator[](const K& key);
   * V& operator[](K&& key);
   * \endcode
   *
   * \param key Key to access.
   *
   * \return The mapped value.
   *
   * \attention 1. V must be default-initializable.
   */
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

  /**
   * \brief Accesses an existing mapped value.
   * \code{.cpp}
   * V& at(const K& key); const V& at(const K& key) const;
   * \endcode
   * \param key Key to access.
   * \return The mapped value.
   * \throws std::out_of_range if the key is absent.
   */
  V &at(const K &key) {
    auto it = find(key);
    if (it == end())
      throw std::out_of_range{"HashMap::at"};
    return (*it).second;
  }

  const V &at(const K &key) const {
    auto it = find(key);
    if (it == end())
      throw std::out_of_range{"HashMap::at"};
    return (*it).second;
  }

  /**
   * \brief Finds an entry by key.
   * \code{.cpp}
   * iterator find(const K& key);
   * const_iterator find(const K& key) const;
   * \endcode
   *
   * \param key Key to find.
   *
   * \return Iterator to the entry, or end() if absent.
   */
  iterator find(const K &key) {
    return iterator(this, find_index(key));
  }

  const_iterator find(const K &key) const {
    return const_iterator(this, find_index(key));
  }

  /**
   * \brief Checks whether a key exists.
   * \code{.cpp}
   * bool contains(const K& key) const;
   * \endcode
   *
   * \param key Key to find.
   *
   * \return Whether the key exists.
   */
  [[nodiscard]]
  bool contains(const K &key) const {
    return find_index(key) != m_capacity;
  }

  /**
   * \brief Returns the number of entries with a key.
   * \code{.cpp}
   * size_type count(const K& key) const;
   * \endcode
   */
  [[nodiscard]] size_type count(const K &key) const {
    return contains(key) ? 1 : 0;
  }

  /**
   * \brief Returns the range containing a key.
   * \code{.cpp}
   * std::pair<iterator, iterator> equal_range(const K& key);
   * std::pair<const_iterator, const_iterator> equal_range(const K& key) const;
   * \endcode
   */
  std::pair<iterator, iterator> equal_range(const K &key) {
    auto first = find(key);
    auto last = first;
    if (last != end())
      ++last;
    return {first, last};
  }

  std::pair<const_iterator, const_iterator> equal_range(const K &key) const {
    auto first = find(key);
    auto last = first;
    if (last != end())
      ++last;
    return {first, last};
  }

  /**
   * \brief Erases an entry by key.
   * \code{.cpp}
   * bool erase(const K& key);
   * \endcode
   * \param key Key to erase.
   * \return Whether a key was erased.
   */
  bool erase(const K &key) {
    const size_type index = find_index(key);

    if (index == m_capacity)
      return false;

    erase_index(index);
    return true;
  }

  /**
   * \brief Erases an entry by iterator.
   * \code{.cpp}
   * iterator erase(const_iterator position);
   * iterator erase(const_iterator first, const_iterator last);
   * \endcode
   * \param position Entry to erase.
   * \return Iterator following the erased entry.
   * \attention 1. The iterator must refer to an entry in this map.
   */
  iterator erase(const_iterator pos) {
    assert(pos.m_map == this);
    assert(pos.m_index < m_capacity);
    assert(m_control[pos.m_index] > DELETED);

    const size_type index = pos.m_index;
    erase_index(index);
    return iterator(this, index + 1);
  }

  iterator erase(const_iterator first, const_iterator last) {
    while (first != last)
      first = erase(first);
    return iterator(this, first.m_index);
  }

  /**
   * \brief Removes all entries.
   * \code{.cpp}
   * void clear() noexcept;
   * \endcode
   *
   * Capacity is retained.
   */
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

  /**
   * \brief Reserves capacity for entries.
   * \code{.cpp}
   * void reserve(size_type count);
   * \endcode
   *
   * \param count Desired number of elements.
   *
   * The requested count refers to elements rather than storage slots.
   */
  void reserve(size_type count) {
    if (count <= load_limit(m_capacity))
      return;

    size_type newCapacity = m_capacity ? m_capacity : 8;

    while (count > load_limit(newCapacity)) {
      assert(newCapacity <= max_slot_count() / 2);
      newCapacity *= 2;
    }

    rehash_slots(newCapacity);
  }

  /**
   * \brief Rebuilds the table with at least the requested slot count.
   * \code{.cpp}
   * void rehash(size_type count);
   * \endcode
   * \param count Minimum number of storage slots.
   */
  void rehash(size_type count) {
    if (count == 0 && m_size == 0) {
      reset();
      return;
    }
    size_type requested = 8;
    while (requested < count || m_size > load_limit(requested)) {
      assert(requested <= max_slot_count() / 2);
      requested *= 2;
    }
    if (requested != m_capacity || m_deleted != 0)
      rehash_slots(requested);
  }

  /**
   * \brief Reduces storage to the minimum capacity needed for the entries.
   * \code{.cpp}
   * void shrink_to_fit();
   * \endcode
   */
  void shrink_to_fit() { rehash(0); }

  /**
   * \brief Returns the number of entries.
   * \code{.cpp}
   * size_type size() const noexcept;
   * \endcode
   *
   * \return Number of entries.
   */
  [[nodiscard]] size_type size() const noexcept {
    return m_size;
  }

  /**
   * \brief Returns the number of storage slots.
   * \code{.cpp}
   * size_type capacity() const noexcept;
   * \endcode
   *
   * \return Number of storage slots.
   */
  [[nodiscard]] size_type capacity() const noexcept {
    return m_capacity;
  }

  /**
   * \brief Returns the number of storage buckets.
   * \code{.cpp}
   * size_type bucket_count() const noexcept;
   * \endcode
   * \return Number of buckets.
   */
  [[nodiscard]] size_type bucket_count() const noexcept { return m_capacity; }

  /**
   * \brief Returns the maximum supported bucket count.
   * \code{.cpp}
   * static constexpr size_type max_bucket_count() noexcept;
   * \endcode
   * \return Maximum bucket count.
   */
  [[nodiscard]] static constexpr size_type max_bucket_count() noexcept {
    return max_slot_count();
  }

  /**
   * \brief Returns the initial bucket for a key.
   * \code{.cpp}
   * size_type bucket(const K& key) const;
   * \endcode
   * \param key Key to hash.
   * \return Initial probe bucket.
   */
  [[nodiscard]] size_type bucket(const K &key) const {
    return m_capacity == 0 ? 0 : mixed_hash(key) & (m_capacity - 1);
  }

  /**
   * \brief Returns whether a storage bucket contains an entry.
   * \code{.cpp}
   * size_type bucket_size(size_type index) const;
   * \endcode
   * \param index Bucket index.
   * \return Zero or one.
   * \attention 1. \p index must be less than bucket_count().
   */
  [[nodiscard]] size_type bucket_size(size_type index) const {
    assert(index < m_capacity);
    return m_control[index] > DELETED ? 1 : 0;
  }

  /**
   * \brief Returns the current load factor.
   * \code{.cpp}
   * float load_factor() const noexcept;
   * \endcode
   * \return size() divided by bucket_count(), or zero when empty.
   */
  [[nodiscard]] float load_factor() const noexcept {
    return m_capacity == 0 ? 0.0f
                           : static_cast<float>(m_size) /
                                 static_cast<float>(m_capacity);
  }

  /**
   * \brief Accesses or changes the maximum load factor.
   * \code{.cpp}
   * float max_load_factor() const noexcept;
   * void max_load_factor(float value);
   * \endcode
   * \param value New load factor.
   * \return Current maximum load factor.
   * \attention 1. \p value must be greater than zero and at most one.
   */
  [[nodiscard]] float max_load_factor() const noexcept {
    return m_maxLoadFactor;
  }

  void max_load_factor(float value) {
    assert(value > 0.0f && value <= 1.0f);
    m_maxLoadFactor = value;
    if (m_size > load_limit(m_capacity))
      reserve(m_size);
  }

  /**
   * \brief Returns the maximum supported entry count.
   * \code{.cpp}
   * size_type max_size() const noexcept;
   * \endcode
   * \return Maximum entry count.
   */
  [[nodiscard]] size_type max_size() const noexcept {
    return load_limit(max_slot_count());
  }

  /**
   * \brief Returns the allocator.
   * \code{.cpp}
   * A get_allocator() const;
   * \endcode
   * \return A copy of the allocator.
   */
  [[nodiscard]] A get_allocator() const { return m_allocator; }

  /**
   * \brief Returns the hash function.
   * \code{.cpp}
   * Hash hash_function() const;
   * \endcode
   * \return A copy of the hash function.
   */
  [[nodiscard]] Hash hash_function() const { return m_hash; }

  /**
   * \brief Returns the key equality predicate.
   * \code{.cpp}
   * Equal key_eq() const;
   * \endcode
   * \return A copy of the predicate.
   */
  [[nodiscard]] Equal key_eq() const { return m_equal; }

  /**
   * \brief Exchanges two maps.
   * \code{.cpp}
   * void swap(HashMap& other) noexcept(...);
   * \endcode
   * \param other Map to exchange with.
   */
  void swap(HashMap &other) noexcept(
      std::is_nothrow_swappable_v<Hash> &&
      std::is_nothrow_swappable_v<Equal> &&
      std::is_nothrow_swappable_v<A>) {
    using std::swap;
    swap(m_control, other.m_control);
    swap(m_entries, other.m_entries);
    swap(m_capacity, other.m_capacity);
    swap(m_size, other.m_size);
    swap(m_deleted, other.m_deleted);
    swap(m_controlAllocation, other.m_controlAllocation);
    swap(m_entryAllocation, other.m_entryAllocation);
    swap(m_hash, other.m_hash);
    swap(m_equal, other.m_equal);
    swap(m_allocator, other.m_allocator);
    swap(m_maxLoadFactor, other.m_maxLoadFactor);
  }

  /**
   * \brief Checks whether the map is empty.
   * \code{.cpp}
   * bool empty() const noexcept;
   * \endcode
   *
   * \return Whether size() is zero.
   */
  [[nodiscard]] bool empty() const noexcept {
    return m_size == 0;
  }

  /**
   * \brief Returns an iterator to the first entry.
   * \code{.cpp}
   * iterator begin() noexcept;
   * const_iterator begin() const noexcept;
   * \endcode
   *
   * \return Iterator to the first entry.
   */
  iterator begin() noexcept {
    return iterator(this, 0);
  }

  /**
   * \brief Returns an iterator past the last entry.
   * \code{.cpp}
   * iterator end() noexcept;
   * const_iterator end() const noexcept;
   * \endcode
   *
   * \return Iterator past the last entry.
   */
  iterator end() noexcept {
    return iterator(this, m_capacity);
  }

  const_iterator begin() const noexcept {
    return const_iterator(this, 0);
  }

  const_iterator end() const noexcept {
    return const_iterator(this, m_capacity);
  }

  /**
   * \brief Returns a const iterator to the first entry.
   * \code{.cpp}
   * const_iterator cbegin() const noexcept;
   * \endcode
   *
   * \return Const iterator to the first entry.
   */
  const_iterator cbegin() const noexcept {
    return begin();
  }

  /**
   * \brief Returns a const iterator past the last entry.
   * \code{.cpp}
   * const_iterator cend() const noexcept;
   * \endcode
   *
   * \return Const iterator past the last entry.
   */
  const_iterator cend() const noexcept {
    return end();
  }

  /**
   * \brief Compares maps by key-value contents.
   * \code{.cpp}
   * friend bool operator==(const HashMap& lhs, const HashMap& rhs);
   * \endcode
   * \param lhs Left map.
   * \param rhs Right map.
   * \return Whether both maps contain equal entries.
   */
  friend bool operator==(const HashMap &lhs, const HashMap &rhs) {
    if (lhs.size() != rhs.size())
      return false;
    for (const auto [key, value] : lhs) {
      auto it = rhs.find(key);
      if (it == rhs.end() || !((*it).second == value))
        return false;
    }
    return true;
  }

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

  size_type load_limit(size_type capacity) const noexcept {
    return static_cast<size_type>(static_cast<long double>(capacity) *
                                  m_maxLoadFactor);
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

  std::pair<size_type, bool> find_or_slot(const K &key,
                                          size_type hash) const {
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

    if (m_size == 0) {
      std::memset(m_control, EMPTY, m_capacity);
      m_deleted = 0;
    }
  }

  bool needs_rehash() const noexcept {
    return m_capacity == 0 ||
           m_size + m_deleted + 1 > load_limit(m_capacity);
  }

  void prepare_insert() {
    if (m_capacity == 0) {
      rehash_slots(8);
    } else if (m_size + 1 > load_limit(m_capacity)) {
      assert(m_capacity <= max_slot_count() / 2);
      rehash_slots(m_capacity * 2);
    } else {
      rehash_slots(m_capacity);
    }
  }

  void rehash_slots(size_type newCapacity) {
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
      ATraits::template deallocate<std::uint8_t>(
          m_allocator, m_control, m_controlAllocation);

      ATraits::template deallocate<Entry>(
          m_allocator, m_entries, m_entryAllocation);
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
      ATraits::template deallocate<std::uint8_t>(
          m_allocator, m_control, m_controlAllocation);

      ATraits::template deallocate<Entry>(
          m_allocator, m_entries, m_entryAllocation);
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
  float m_maxLoadFactor = 0.75f;
};

} // namespace strobe

namespace fmt {

template <typename K, typename V, typename Hash, typename Equal,
          strobe::Allocator Alloc>
struct formatter<strobe::HashMap<K, V, Hash, Equal, Alloc>> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }

  template <typename FormatContext>
  auto format(const strobe::HashMap<K, V, Hash, Equal, Alloc> &map,
              FormatContext &ctx) const {
    auto out = ctx.out();
    *out++ = '{';
    bool first = true;
    for (const auto [key, value] : map) {
      if (!first) {
        *out++ = ',';
        *out++ = ' ';
      }
      first = false;
      out = fmt::format_to(out, "{}: {}", key, value);
    }
    *out++ = '}';
    return out;
  }
};

} // namespace fmt
