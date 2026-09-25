#pragma once

#include <algorithm>
#include <bit>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <fmt/format.h>
#include <iterator>
#include <limits>
#include <memory>
#include <type_traits>
#include <utility>

#include <strobe/memory.hpp>

#if defined(__SSE2__)
#include <immintrin.h>
#else
#error "SwizzHashMap requires x86 SSE2."
#endif

namespace strobe::detail {

template <std::size_t Width> struct SwizzGroup;

template <> struct SwizzGroup<16> {
  __m128i bytes;
  explicit SwizzGroup(const std::uint8_t *p) noexcept
      : bytes(_mm_loadu_si128(reinterpret_cast<const __m128i *>(p))) {}

  std::uint32_t match(std::uint8_t tag) const noexcept {
    return static_cast<std::uint32_t>(_mm_movemask_epi8(
        _mm_cmpeq_epi8(bytes, _mm_set1_epi8(static_cast<char>(tag)))));
  }
  std::uint32_t available() const noexcept {
    return static_cast<std::uint32_t>(_mm_movemask_epi8(bytes));
  }
  std::uint32_t full() const noexcept { return available() ^ 0xffffu; }
};

#if defined(__AVX2__)
template <> struct SwizzGroup<32> {
  __m256i bytes;
  explicit SwizzGroup(const std::uint8_t *p) noexcept
      : bytes(_mm256_loadu_si256(reinterpret_cast<const __m256i *>(p))) {}

  std::uint32_t match(std::uint8_t tag) const noexcept {
    return static_cast<std::uint32_t>(_mm256_movemask_epi8(
        _mm256_cmpeq_epi8(bytes, _mm256_set1_epi8(static_cast<char>(tag)))));
  }
  std::uint32_t available() const noexcept {
    return static_cast<std::uint32_t>(_mm256_movemask_epi8(bytes));
  }
  std::uint32_t full() const noexcept { return ~available(); }
};
#endif

template <typename Hash, typename Equal, typename K, typename Q>
concept SwizzTransparentLookup = requires(const Hash &hash, const Equal &equal,
                                          const K &key, const Q &query) {
  typename Hash::is_transparent;
  typename Equal::is_transparent;
  { hash(query) } -> std::convertible_to<std::size_t>;
  { equal(key, query) } -> std::convertible_to<bool>;
};

// This is an explicit opt-in by a hasher, never inferred from its key type.
template <typename Hash>
inline constexpr bool swizz_hash_is_avalanching = [] {
  if constexpr (requires { typename Hash::is_avalanching; })
    return static_cast<bool>(Hash::is_avalanching::value);
  else
    return false;
}();

} // namespace strobe::detail

namespace strobe {

/**
 * \ingroup core
 * \brief SIMD-accelerated hash table with open addressing.
 *
 * This map is intended for workloads with small keys and frequent misses.
 * GroupWidth 32 requires AVX2 support; mutation and concurrent access require
 * external synchronization.
 *
 * \attention 1. Allocation, construction, movement, and destruction must not
 * throw.
 */
template <typename K, typename V, typename Hash = std::hash<K>,
          typename Equal = std::equal_to<K>, Allocator A = Mallocator,
          std::size_t GroupWidth = 16>
class SwizzHashMap {
  static_assert(GroupWidth == 16 || GroupWidth == 32);
#if !defined(__AVX2__)
  static_assert(GroupWidth == 16, "GroupWidth=32 requires -mavx2.");
#endif
  static_assert(std::is_object_v<K> && !std::is_const_v<K>);
  static_assert(std::is_object_v<V> && !std::is_const_v<V>);

  using ATraits = AllocatorTraits<A>;
  using Group = detail::SwizzGroup<GroupWidth>;
  static constexpr std::uint8_t EMPTY = 0x80;
  static constexpr std::uint8_t DELETED = 0xfe;
  static constexpr std::size_t NPOS = std::numeric_limits<std::size_t>::max();

  struct alignas(64) ControlBlock {
    std::uint8_t bytes[64];
  };
  struct Storage {
    ControlBlock *blocks = nullptr;
    K *keys = nullptr;
    V *values = nullptr;
    std::size_t capacity = 0;
    std::size_t blockAllocation = 0;
    std::size_t keyAllocation = 0;
    std::size_t valueAllocation = 0;

    std::uint8_t *control() const noexcept {
      return reinterpret_cast<std::uint8_t *>(blocks);
    }
  };

  struct ProbeResult {
    std::size_t index;
    bool found;
  };

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
   * \brief Number of control bytes probed as one SIMD group.
   * \code{.cpp}
   * static constexpr size_type group_width = GroupWidth;
   * \endcode
   */
  static constexpr size_type group_width = GroupWidth;

  template <bool Const> class Iterator {
    using Map = std::conditional_t<Const, const SwizzHashMap, SwizzHashMap>;
    using Value = std::conditional_t<Const, const V, V>;
    friend class SwizzHashMap;
    template <bool> friend class Iterator;
    Iterator(Map *map, size_type index) noexcept : m_map(map), m_index(index) {}

  public:
    using iterator_category = std::input_iterator_tag;
    using iterator_concept = std::input_iterator_tag;
    using value_type = std::pair<K, V>;
    using difference_type = std::ptrdiff_t;
    using reference = std::pair<const K &, Value &>;
    struct ArrowProxy {
      reference refs;
      const reference *operator->() const noexcept { return &refs; }
    };
    using pointer = ArrowProxy;

    /**
     * \brief Constructs or copies an iterator.
     * \code{.cpp}
     * Iterator() = default;
     * Iterator(const Iterator&) = default;
     * Iterator& operator=(const Iterator&) = default;
     * template<bool Other> Iterator(const Iterator<Other>& other) requires Const;
     * \endcode
     * \param other Iterator to copy or convert.
     */
    Iterator() = default;
    Iterator(const Iterator &) = default;
    Iterator &operator=(const Iterator &) = default;
    template <bool Other>
      requires(Const && !Other)
    Iterator(const Iterator<Other> &other) noexcept
        : m_map(other.m_map), m_index(other.m_index) {}

    /**
     * \brief Accesses the current entry.
     * \code{.cpp}
     * reference operator*() const noexcept;
     * \endcode
     * \return The current key-value pair.
     * \attention 1. The iterator must refer to an entry.
     */
    reference operator*() const noexcept {
      assert(m_map && m_index < m_map->capacity());
      assert(m_map->m_storage.control()[m_index] < EMPTY);
      return {m_map->m_storage.keys[m_index], m_map->m_storage.values[m_index]};
    }
    /**
     * \brief Accesses the current entry through an arrow proxy.
     * \code{.cpp}
     * pointer operator->() const noexcept;
     * \endcode
     * \return Proxy for the current key-value pair.
     * \attention 1. The iterator must refer to an entry.
     */
    ArrowProxy operator->() const noexcept { return {**this}; }
    /**
     * \brief Advances the iterator.
     * \code{.cpp}
     * Iterator& operator++() noexcept;
     * Iterator operator++(int) noexcept;
     * \endcode
     * \return The advanced iterator or its previous value.
     */
    Iterator &operator++() noexcept {
      assert(m_map && m_index < m_map->capacity());
      m_index = m_map->next_full(m_index + 1);
      return *this;
    }
    Iterator operator++(int) noexcept {
      auto old = *this;
      ++*this;
      return old;
    }
    /**
     * \brief Compares two iterators.
     * \code{.cpp}
     * template<bool Other> bool operator==(const Iterator<Other>& other) const noexcept;
     * \endcode
     * \param other Iterator to compare with.
     * \return Whether both iterators refer to the same position.
     */
    template <bool Other>
    bool operator==(const Iterator<Other> &other) const noexcept {
      return m_map == other.m_map && m_index == other.m_index;
    }

  private:
    Map *m_map = nullptr;
    size_type m_index = 0;
  };

  using iterator = Iterator<false>;
  using const_iterator = Iterator<true>;

  /**
   * \brief Constructs, copies, moves, assigns, or destroys a map.
   * \code{.cpp}
   * explicit SwizzHashMap(const A& allocator = {}, const Hash& hash = {}, const Equal& equal = {});
   * SwizzHashMap(const SwizzHashMap& other);
   * SwizzHashMap(SwizzHashMap&& other) noexcept;
   * SwizzHashMap& operator=(const SwizzHashMap& other);
   * SwizzHashMap& operator=(SwizzHashMap&& other) noexcept;
   * ~SwizzHashMap();
   * \endcode
   * \param allocator Allocator to use.
   * \param hash Hash function.
   * \param equal Key equality predicate.
   * \param other Map to copy or move from.
   * Moved-from maps are empty.
   */
  explicit SwizzHashMap(const A &allocator = {}, const Hash &hash = {},
                        const Equal &equal = {})
      : m_hash(hash), m_equal(equal), m_allocator(allocator) {}

  SwizzHashMap(const SwizzHashMap &other)
    requires(std::copy_constructible<K> && std::copy_constructible<V>)
      : m_hash(other.m_hash), m_equal(other.m_equal),
        m_allocator(
            ATraits::select_on_container_copy_construction(other.m_allocator)) {
    copy_entries(other);
  }

  SwizzHashMap(SwizzHashMap &&other) noexcept
      : m_storage(std::exchange(other.m_storage, {})),
        m_size(std::exchange(other.m_size, 0)),
        m_deleted(std::exchange(other.m_deleted, 0)),
        m_hash(std::move(other.m_hash)), m_equal(std::move(other.m_equal)),
        m_allocator(std::move(other.m_allocator)) {}

  ~SwizzHashMap() { reset(); }

  SwizzHashMap &operator=(const SwizzHashMap &other)
    requires(std::copy_constructible<K> && std::copy_constructible<V>)
  {
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
    copy_entries(other);
    return *this;
  }

  SwizzHashMap &operator=(SwizzHashMap &&other) noexcept {
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
      for_each_full(other.m_storage, [&](size_type i) {
        const size_type hash = mixed_hash(other.m_storage.keys[i]);
        const size_type slot = first_available(m_storage, hash);
        construct_entry(slot, hash, std::move(other.m_storage.keys[i]),
                        std::move(other.m_storage.values[i]));
      });
      other.clear();
    }
    return *this;
  }

  /**
   * \brief Inserts a value if the key is absent.
   * \code{.cpp}
   * template<typename... Args> std::pair<iterator, bool> try_emplace(const K& key, Args&&... args);
   * template<typename... Args> std::pair<iterator, bool> try_emplace(K&& key, Args&&... args);
   * \endcode
   * \param key Key to insert.
   * \param args Arguments forwarded to construct the mapped value.
   * \return Iterator to the entry and whether insertion occurred.
   */
  template <typename... Args>
    requires std::constructible_from<V, Args &&...>
  std::pair<iterator, bool> try_emplace(const K &key, Args &&...args) {
    return try_emplace_impl(key, std::forward<Args>(args)...);
  }
  template <typename... Args>
    requires std::constructible_from<V, Args &&...>
  std::pair<iterator, bool> try_emplace(K &&key, Args &&...args) {
    return try_emplace_impl(std::move(key), std::forward<Args>(args)...);
  }

  /**
   * \brief Inserts a key-value entry if the key is absent.
   * \code{.cpp}
   * std::pair<iterator, bool> insert(const K& key, const V& value);
   * std::pair<iterator, bool> insert(const K& key, V&& value);
   * std::pair<iterator, bool> insert(K&& key, const V& value);
   * std::pair<iterator, bool> insert(K&& key, V&& value);
   * std::pair<iterator, bool> insert(const value_type& entry);
   * std::pair<iterator, bool> insert(value_type&& entry);
   * template<typename Key, typename Value> std::pair<iterator, bool> insert(const std::pair<Key, Value>& entry);
   * template<typename Key, typename Value> std::pair<iterator, bool> insert(std::pair<Key, Value>&& entry);
   * \endcode
   * \param key Key to insert.
   * \param value Mapped value to insert.
   * \param entry Entry to insert.
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
  std::pair<iterator, bool> insert(const value_type &entry) {
    return try_emplace(entry.first, entry.second);
  }
  std::pair<iterator, bool> insert(value_type &&entry) {
    return try_emplace(entry.first, std::move(entry.second));
  }
  template <typename Key, typename Value>
  std::pair<iterator, bool> insert(const std::pair<Key, Value> &entry) {
    return try_emplace(entry.first, entry.second);
  }
  template <typename Key, typename Value>
  std::pair<iterator, bool> insert(std::pair<Key, Value> &&entry) {
    return try_emplace(std::forward<Key>(entry.first),
                       std::forward<Value>(entry.second));
  }

  /**
   * \brief Inserts or assigns a mapped value.
   * \code{.cpp}
   * template<typename M> std::pair<iterator, bool> insert_or_assign(const K& key, M&& value);
   * template<typename M> std::pair<iterator, bool> insert_or_assign(K&& key, M&& value);
   * \endcode
   * \param key Key to insert or find.
   * \param value Value to insert or assign.
   * \return Iterator to the entry and whether insertion occurred.
   */
  template <typename M>
    requires(std::constructible_from<V, M &&> &&
             std::is_assignable_v<V &, M &&>)
  std::pair<iterator, bool> insert_or_assign(const K &key, M &&value) {
    return insert_or_assign_impl(key, std::forward<M>(value));
  }
  template <typename M>
    requires(std::constructible_from<V, M &&> &&
             std::is_assignable_v<V &, M &&>)
  std::pair<iterator, bool> insert_or_assign(K &&key, M &&value) {
    return insert_or_assign_impl(std::move(key), std::forward<M>(value));
  }

  /**
   * \brief Accesses or inserts a mapped value.
   * \code{.cpp}
   * V& operator[](const K& key);
   * V& operator[](K&& key);
   * \endcode
   * \param key Key to access.
   * \return Reference to the mapped value.
   * \attention 1. V must be default-initializable.
   */
  V &operator[](const K &key)
    requires std::default_initializable<V>
  {
    return try_emplace(key).first->second;
  }
  V &operator[](K &&key)
    requires std::default_initializable<V>
  {
    return try_emplace(std::move(key)).first->second;
  }

  /**
   * \brief Finds an entry by key.
   * \code{.cpp}
   * iterator find(const K& key);
   * const_iterator find(const K& key) const;
   * template<typename Q> iterator find(const Q& key);
   * template<typename Q> const_iterator find(const Q& key) const;
   * \endcode
   * \param key Key or transparent lookup key.
   * \return Iterator to the entry, or end() if absent.
   */
  iterator find(const K &key) { return iterator(this, find_index(key)); }
  const_iterator find(const K &key) const {
    return const_iterator(this, find_index(key));
  }
  template <typename Q>
    requires detail::SwizzTransparentLookup<Hash, Equal, K, Q>
  iterator find(const Q &key) {
    return iterator(this, find_index(key));
  }
  template <typename Q>
    requires detail::SwizzTransparentLookup<Hash, Equal, K, Q>
  const_iterator find(const Q &key) const {
    return const_iterator(this, find_index(key));
  }

  /**
   * \brief Finds a mapped value by key.
   * \code{.cpp}
   * V* find_value(const K& key);
   * const V* find_value(const K& key) const;
   * template<typename Q> V* find_value(const Q& key);
   * template<typename Q> const V* find_value(const Q& key) const;
   * \endcode
   * \param key Key or transparent lookup key.
   * \return Pointer to the mapped value, or nullptr if absent.
   */
  [[nodiscard]] V *find_value(const K &key) {
    const size_type i = find_index(key);
    return i == capacity() ? nullptr : m_storage.values + i;
  }
  [[nodiscard]] const V *find_value(const K &key) const {
    const size_type i = find_index(key);
    return i == capacity() ? nullptr : m_storage.values + i;
  }
  template <typename Q>
    requires detail::SwizzTransparentLookup<Hash, Equal, K, Q>
  [[nodiscard]] V *find_value(const Q &key) {
    const size_type i = find_index(key);
    return i == capacity() ? nullptr : m_storage.values + i;
  }
  template <typename Q>
    requires detail::SwizzTransparentLookup<Hash, Equal, K, Q>
  [[nodiscard]] const V *find_value(const Q &key) const {
    const size_type i = find_index(key);
    return i == capacity() ? nullptr : m_storage.values + i;
  }

  /**
   * \brief Checks whether a key exists.
   * \code{.cpp}
   * bool contains(const K& key) const;
   * template<typename Q> bool contains(const Q& key) const;
   * \endcode
   * \param key Key or transparent lookup key.
   * \return Whether the key exists.
   */
  [[nodiscard]] bool contains(const K &key) const {
    return find_index(key) != capacity();
  }
  template <typename Q>
    requires detail::SwizzTransparentLookup<Hash, Equal, K, Q>
  [[nodiscard]] bool contains(const Q &key) const {
    return find_index(key) != capacity();
  }
  /**
   * \brief Checks whether a key exists.
   * \code{.cpp}
   * bool containsKey(const K& key) const;
   * \endcode
   * \param key Key to find.
   * \return Whether the key exists.
   */
  [[nodiscard]] bool containsKey(const K &key) const { return contains(key); }
  /**
   * \brief Counts entries matching a key.
   * \code{.cpp}
   * size_type count(const K& key) const;
   * template<typename Q> size_type count(const Q& key) const;
   * \endcode
   * \param key Key or transparent lookup key.
   * \return 1 if present, otherwise 0.
   */
  [[nodiscard]] size_type count(const K &key) const {
    return contains(key) ? 1 : 0;
  }
  template <typename Q>
    requires detail::SwizzTransparentLookup<Hash, Equal, K, Q>
  [[nodiscard]] size_type count(const Q &key) const {
    return contains(key) ? 1 : 0;
  }

  /**
   * \brief Accesses a mapped value with assertion-based checking.
   * \code{.cpp}
   * V& at(const K& key);
   * const V& at(const K& key) const;
   * \endcode
   * \param key Key to access.
   * \return Reference to the mapped value.
   * \attention 1. The key must exist; failure triggers an assertion.
   */
  V &at(const K &key) {
    V *value = find_value(key);
    assert(value != nullptr);
    return *value;
  }
  const V &at(const K &key) const {
    const V *value = find_value(key);
    assert(value != nullptr);
    return *value;
  }

  /**
   * \brief Erases entries by key or iterator.
   * \code{.cpp}
   * size_type erase(const K& key);
   * template<typename Q> size_type erase(const Q& key);
   * iterator erase(const_iterator pos);
   * iterator erase(iterator pos);
   * \endcode
   * \param key Key or transparent lookup key.
   * \param pos Entry to erase.
   * \return Number of erased entries, or iterator following the erased entry.
   * \attention 1. Iterator arguments must refer to an entry in this map.
   */
  size_type erase(const K &key) { return erase_key(key); }
  template <typename Q>
    requires detail::SwizzTransparentLookup<Hash, Equal, K, Q>
  size_type erase(const Q &key) {
    return erase_key(key);
  }
  iterator erase(const_iterator pos) {
    assert(pos.m_map == this && pos.m_index < capacity());
    assert(m_storage.control()[pos.m_index] < EMPTY);
    const size_type i = pos.m_index;
    erase_index(i);
    return iterator(this, next_full(i + 1));
  }
  iterator erase(iterator pos) { return erase(const_iterator(pos)); }

  /**
   * \brief Removes all entries.
   * \code{.cpp}
   * void clear() noexcept;
   * \endcode
   * Capacity is retained.
   */
  void clear() noexcept {
    destroy_elements(m_storage);
    if (m_storage.blocks)
      std::memset(m_storage.control(), EMPTY, capacity() + GroupWidth - 1);
    m_size = 0;
    m_deleted = 0;
  }

  /**
   * \brief Reserves capacity for entries.
   * \code{.cpp}
   * void reserve(size_type count);
   * \endcode
   * \param count Desired number of entries.
   * \attention 1. \p count must not exceed max_size().
   */
  void reserve(size_type count) {
    assert(count <= max_size());
    if (count > load_limit(capacity())) {
      rehash_slots(capacity_for(count));
    } else if (m_deleted > load_limit(capacity()) - std::max(count, m_size)) {
      rehash_slots(capacity());
    }
  }
  /**
   * \brief Rebuilds the table with at least the requested slot count.
   * \code{.cpp}
   * void rehash(size_type slots);
   * \endcode
   * \param slots Requested slot count.
   * \attention 1. \p slots must not exceed the maximum slot count.
   */
  void rehash(size_type slots) {
    assert(slots <= max_slot_count());
    if (slots == 0 && m_size == 0) {
      reset();
      return;
    }
    size_type requested = std::max(slots, capacity_for(m_size));
    requested = std::max(requested, GroupWidth);
    const size_type next = std::bit_ceil(requested);
    if (next != capacity() || m_deleted != 0)
      rehash_slots(next);
  }
  /**
   * \brief Removes deleted-slot tombstones.
   * \code{.cpp}
   * void compact();
   * \endcode
   */
  void compact() {
    if (m_deleted != 0)
      rehash_slots(capacity());
  }
  /**
   * \brief Shrinks the table to the current element count.
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
   * \return Number of entries.
   */
  [[nodiscard]] size_type size() const noexcept { return m_size; }

  /**
   * \brief Checks whether the map is empty.
   * \code{.cpp}
   * bool empty() const noexcept;
   * \endcode
   * \return Whether size() is zero.
   */
  [[nodiscard]] bool empty() const noexcept { return m_size == 0; }

  /**
   * \brief Returns the slot capacity.
   * \code{.cpp}
   * size_type capacity() const noexcept;
   * \endcode
   * \return Number of hash-table slots.
   */
  [[nodiscard]] size_type capacity() const noexcept {
    return m_storage.capacity;
  }
  /**
   * \brief Returns the slot count.
   * \code{.cpp}
   * size_type bucket_count() const noexcept;
   * \endcode
   * \return Number of slots.
   */
  [[nodiscard]] size_type bucket_count() const noexcept { return capacity(); }

  /**
   * \brief Returns the current load factor.
   * \code{.cpp}
   * float load_factor() const noexcept;
   * \endcode
   * \return Ratio of entries to slots.
   */
  [[nodiscard]] float load_factor() const noexcept {
    return capacity() ? float(m_size) / float(capacity()) : 0.0f;
  }
  /**
   * \brief Returns the maximum load factor.
   * \code{.cpp}
   * static constexpr float max_load_factor() noexcept;
   * \endcode
   * \return Maximum supported load factor.
   */
  [[nodiscard]] static constexpr float max_load_factor() noexcept {
    return 0.875f;
  }
  /**
   * \brief Returns the maximum entry count.
   * \code{.cpp}
   * static constexpr size_type max_size() noexcept;
   * \endcode
   * \return Maximum number of entries.
   */
  [[nodiscard]] static constexpr size_type max_size() noexcept {
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
   * \return A copy of the equality predicate.
   */
  [[nodiscard]] Equal key_eq() const { return m_equal; }

  /**
   * \brief Returns the allocated storage size.
   * \code{.cpp}
   * size_type allocated_bytes() const noexcept;
   * \endcode
   * \return Allocated bytes used by controls, keys, and values.
   */
  [[nodiscard]] size_type allocated_bytes() const noexcept {
    return m_storage.blockAllocation * sizeof(ControlBlock) +
           m_storage.keyAllocation * sizeof(K) +
           m_storage.valueAllocation * sizeof(V);
  }

  /**
   * \brief Returns an iterator to the first entry.
   * \code{.cpp}
   * iterator begin() noexcept;
   * const_iterator begin() const noexcept;
   * \endcode
   * \return Iterator to the first entry.
   */
  iterator begin() noexcept { return iterator(this, next_full(0)); }
  const_iterator begin() const noexcept {
    return const_iterator(this, next_full(0));
  }

  /**
   * \brief Returns an iterator past the last entry.
   * \code{.cpp}
   * iterator end() noexcept;
   * const_iterator end() const noexcept;
   * \endcode
   * \return Iterator past the last entry.
   */
  iterator end() noexcept { return iterator(this, capacity()); }
  const_iterator end() const noexcept {
    return const_iterator(this, capacity());
  }

  /**
   * \brief Returns a const iterator to the first entry.
   * \code{.cpp}
   * const_iterator cbegin() const noexcept;
   * \endcode
   * \return Const iterator to the first entry.
   */
  const_iterator cbegin() const noexcept { return begin(); }

  /**
   * \brief Returns a const iterator past the last entry.
   * \code{.cpp}
   * const_iterator cend() const noexcept;
   * \endcode
   * \return Const iterator past the last entry.
   */
  const_iterator cend() const noexcept { return end(); }

private:
  static constexpr size_type load_limit(size_type slots) noexcept {
    return slots - slots / 8;
  }
  static constexpr size_type max_slot_count() noexcept {
    return std::bit_floor(
        std::min({std::numeric_limits<size_type>::max() / sizeof(K),
                  std::numeric_limits<size_type>::max() / sizeof(V),
                  std::numeric_limits<size_type>::max() - (GroupWidth - 1)}));
  }
  static size_type capacity_for(size_type elements) noexcept {
    if (elements == 0)
      return 0;
    assert(elements <= max_size());
    size_type result = GroupWidth;
    while (elements > load_limit(result))
      result *= 2;
    return result;
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
  template <typename Q> size_type mixed_hash(const Q &key) const {
    const size_type hash = static_cast<size_type>(m_hash(key));
    if constexpr (detail::swizz_hash_is_avalanching<Hash>)
      return hash;
    else
      return mix(hash);
  }
  static std::uint8_t fingerprint(size_type hash) noexcept {
    return static_cast<std::uint8_t>(
        hash >> (std::numeric_limits<size_type>::digits - 7));
  }
  static void set_control(Storage &storage, size_type i,
                          std::uint8_t tag) noexcept {
    auto *control = storage.control();
    control[i] = tag;
    if (i < GroupWidth - 1)
      control[storage.capacity + i] = tag;
  }

  template <typename Q> size_type find_index(const Q &key) const {
    if (m_size == 0)
      return capacity();
    const size_type hash = mixed_hash(key);
    const std::uint8_t tag = fingerprint(hash);
    const size_type mask = capacity() - 1;
    size_type offset = hash & mask;
    // Most successful lookups at moderate load stop in their initial slot.
    if (m_storage.control()[offset] == tag &&
        m_equal(m_storage.keys[offset], key))
      return offset;
    size_type step = 0;
    for (;;) {
      Group group(m_storage.control() + offset);
      std::uint32_t candidates = group.match(tag);
      while (candidates) {
        const size_type i = (offset + std::countr_zero(candidates)) & mask;
        if (m_equal(m_storage.keys[i], key))
          return i;
        candidates &= candidates - 1;
      }
      if (group.match(EMPTY))
        return capacity();
      step += GroupWidth;
      offset = (offset + step) & mask;
    }
  }

  template <typename Q>
  ProbeResult find_or_slot(const Q &key, size_type hash) const {
    if (capacity() == 0)
      return {0, false};
    const size_type mask = capacity() - 1;
    const std::uint8_t tag = fingerprint(hash);
    size_type offset = hash & mask;
    size_type step = 0;
    size_type available = NPOS;
    for (;;) {
      Group group(m_storage.control() + offset);
      std::uint32_t candidates = group.match(tag);
      while (candidates) {
        const size_type i = (offset + std::countr_zero(candidates)) & mask;
        if (m_equal(m_storage.keys[i], key))
          return {i, true};
        candidates &= candidates - 1;
      }
      const std::uint32_t free = group.available();
      if (available == NPOS && free)
        available = (offset + std::countr_zero(free)) & mask;
      if (group.match(EMPTY))
        return {available, false};
      step += GroupWidth;
      offset = (offset + step) & mask;
    }
  }

  static size_type first_available(const Storage &storage,
                                   size_type hash) noexcept {
    const size_type mask = storage.capacity - 1;
    size_type offset = hash & mask;
    size_type step = 0;
    for (;;) {
      const std::uint32_t free = Group(storage.control() + offset).available();
      if (free)
        return (offset + std::countr_zero(free)) & mask;
      step += GroupWidth;
      offset = (offset + step) & mask;
    }
  }

  template <typename Key, typename... Args>
  std::pair<iterator, bool> try_emplace_impl(Key &&key, Args &&...args) {
    const size_type hash = mixed_hash(key);
    const ProbeResult result = find_or_slot(key, hash);
    if (result.found)
      return {iterator(this, result.index), false};
    const size_type i =
        insert_missing(result.index, hash, std::forward<Key>(key),
                       std::forward<Args>(args)...);
    return {iterator(this, i), true};
  }
  template <typename Key, typename M>
  std::pair<iterator, bool> insert_or_assign_impl(Key &&key, M &&value) {
    const size_type hash = mixed_hash(key);
    const ProbeResult result = find_or_slot(key, hash);
    if (result.found) {
      m_storage.values[result.index] = std::forward<M>(value);
      return {iterator(this, result.index), false};
    }
    const size_type i = insert_missing(
        result.index, hash, std::forward<Key>(key), std::forward<M>(value));
    return {iterator(this, i), true};
  }
  template <typename Key, typename... Args>
  size_type insert_missing(size_type i, size_type hash, Key &&key,
                           Args &&...args) {
    assert(m_size < max_size());
    const bool rebuild =
        capacity() == 0 || (m_storage.control()[i] == EMPTY &&
                            m_size + m_deleted == load_limit(capacity()));
    if (rebuild) [[unlikely]]
      return insert_growing(hash, std::forward<Key>(key),
                            std::forward<Args>(args)...);
    construct_entry(i, hash, std::forward<Key>(key),
                    std::forward<Args>(args)...);
    return i;
  }
  template <typename Key, typename... Args>
  [[gnu::noinline]] size_type insert_growing(size_type hash, Key &&key,
                                             Args &&...args) {
    // Keep allocation/staging off the ordinary insertion path. Arguments may
    // refer to entries that the impending rebuild will move.
    K stagedKey(std::forward<Key>(key));
    V stagedValue(std::forward<Args>(args)...);
    const size_type next =
        capacity() == 0
            ? GroupWidth
            : (m_size < load_limit(capacity()) ? capacity() : capacity() * 2);
    rehash_slots(next);
    const size_type i = first_available(m_storage, hash);
    construct_entry(i, hash, std::move(stagedKey), std::move(stagedValue));
    return i;
  }
  template <typename Key, typename... Args>
  void construct_entry(size_type i, size_type hash, Key &&key, Args &&...args) {
    assert(m_storage.control()[i] >= EMPTY);
    const bool deleted = m_storage.control()[i] == DELETED;
    std::construct_at(m_storage.keys + i, std::forward<Key>(key));
    std::construct_at(m_storage.values + i, std::forward<Args>(args)...);
    set_control(m_storage, i, fingerprint(hash));
    ++m_size;
    m_deleted -= static_cast<size_type>(deleted);
  }

  template <typename Q> size_type erase_key(const Q &key) {
    const size_type i = find_index(key);
    if (i == capacity())
      return 0;
    erase_index(i);
    return 1;
  }
  void erase_index(size_type i) {
    std::destroy_at(m_storage.keys + i);
    std::destroy_at(m_storage.values + i);
    --m_size;
    if (m_size == 0) {
      std::memset(m_storage.control(), EMPTY, capacity() + GroupWidth - 1);
      m_deleted = 0;
      return;
    }
    // Every Width-byte window containing i must already have contained EMPTY.
    // Equivalently, the nonempty run through i must be shorter than Width.
    const size_type before = (i - GroupWidth) & (capacity() - 1);
    const std::uint32_t left = Group(m_storage.control() + before).match(EMPTY);
    const std::uint32_t right = Group(m_storage.control() + i).match(EMPTY);
    const bool canEmpty =
        left && right &&
        (std::countl_zero(left) - (32 - GroupWidth) + std::countr_zero(right) <
         GroupWidth);
    set_control(m_storage, i, canEmpty ? EMPTY : DELETED);
    m_deleted += static_cast<size_type>(!canEmpty);
  }

  template <typename Fn>
  static void for_each_full(const Storage &storage, Fn &&fn) {
    for (size_type offset = 0; offset < storage.capacity;
         offset += GroupWidth) {
      std::uint32_t full = Group(storage.control() + offset).full();
      while (full) {
        fn(offset + std::countr_zero(full));
        full &= full - 1;
      }
    }
  }
  size_type next_full(size_type i) const noexcept {
    while (i < capacity()) {
      std::uint32_t full = Group(m_storage.control() + i).full();
      const size_type remaining = capacity() - i;
      if (remaining < GroupWidth)
        full &= (std::uint32_t{1} << remaining) - 1;
      if (full)
        return i + std::countr_zero(full);
      i += GroupWidth;
    }
    return capacity();
  }
  static void destroy_elements(const Storage &storage) noexcept {
    if constexpr (!std::is_trivially_destructible_v<K> ||
                  !std::is_trivially_destructible_v<V>) {
      for_each_full(storage, [&](size_type i) {
        std::destroy_at(storage.keys + i);
        std::destroy_at(storage.values + i);
      });
    }
  }
  Storage allocate_storage(size_type slots) {
    assert(slots >= GroupWidth && std::has_single_bit(slots));
    assert(slots <= max_slot_count());
    Storage result;
    result.capacity = slots;
    const size_type bytes = slots + GroupWidth - 1;
    const size_type blocks =
        bytes / sizeof(ControlBlock) + (bytes % sizeof(ControlBlock) != 0);
    auto [ctrl, ctrlCount] =
        ATraits::template allocate_at_least<ControlBlock>(m_allocator, blocks);
    auto [keys, keyCount] =
        ATraits::template allocate_at_least<K>(m_allocator, slots);
    auto [values, valueCount] =
        ATraits::template allocate_at_least<V>(m_allocator, slots);
    assert(ctrl && ctrlCount >= blocks);
    assert(keys && keyCount >= slots);
    assert(values && valueCount >= slots);
    result.blocks = ctrl;
    result.keys = keys;
    result.values = values;
    result.blockAllocation = ctrlCount;
    result.keyAllocation = keyCount;
    result.valueAllocation = valueCount;
    std::memset(result.control(), EMPTY, bytes);
    return result;
  }
  void deallocate_storage(Storage &storage) noexcept {
    if (!storage.blocks)
      return;
    ATraits::template deallocate<ControlBlock>(m_allocator, storage.blocks,
                                               storage.blockAllocation);
    ATraits::template deallocate<K>(m_allocator, storage.keys,
                                    storage.keyAllocation);
    ATraits::template deallocate<V>(m_allocator, storage.values,
                                    storage.valueAllocation);
    storage = {};
  }
  void rehash_slots(size_type slots) {
    Storage next = allocate_storage(slots);
    for_each_full(m_storage, [&](size_type i) {
      const size_type hash = mixed_hash(m_storage.keys[i]);
      const size_type target = first_available(next, hash);
      std::construct_at(next.keys + target, std::move(m_storage.keys[i]));
      std::construct_at(next.values + target, std::move(m_storage.values[i]));
      set_control(next, target, fingerprint(hash));
    });
    destroy_elements(m_storage);
    deallocate_storage(m_storage);
    m_storage = next;
    m_deleted = 0;
  }
  void copy_entries(const SwizzHashMap &other) {
    reserve(other.m_size);
    for_each_full(other.m_storage, [&](size_type i) {
      const size_type hash = mixed_hash(other.m_storage.keys[i]);
      const size_type slot = first_available(m_storage, hash);
      construct_entry(slot, hash, other.m_storage.keys[i],
                      other.m_storage.values[i]);
    });
  }
  void steal(SwizzHashMap &other) noexcept {
    m_storage = std::exchange(other.m_storage, {});
    m_size = std::exchange(other.m_size, 0);
    m_deleted = std::exchange(other.m_deleted, 0);
  }
  void reset() noexcept {
    destroy_elements(m_storage);
    deallocate_storage(m_storage);
    m_size = 0;
    m_deleted = 0;
  }

  Storage m_storage;
  size_type m_size = 0;
  size_type m_deleted = 0;
  [[no_unique_address]] Hash m_hash;
  [[no_unique_address]] Equal m_equal;
  [[no_unique_address]] A m_allocator;
};

} // namespace strobe

namespace fmt {

/**
 * \brief Formats a SIMD hash map as a brace-enclosed entry list.
 * \code{.cpp}
 * template<typename K, typename V, typename Hash, typename Equal,
 *          strobe::Allocator A, size_t GroupWidth>
 * struct formatter<strobe::SwizzHashMap<K, V, Hash, Equal, A, GroupWidth>>;
 * \endcode
 */
template <typename K, typename V, typename Hash, typename Equal,
          strobe::Allocator A, std::size_t GroupWidth>
struct formatter<strobe::SwizzHashMap<K, V, Hash, Equal, A, GroupWidth>> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }

  template <typename FormatContext>
  auto format(
      const strobe::SwizzHashMap<K, V, Hash, Equal, A, GroupWidth> &map,
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
