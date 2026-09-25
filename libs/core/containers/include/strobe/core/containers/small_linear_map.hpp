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

#include "strobe/core/containers/span.hpp"

namespace strobe {

/**
 * \ingroup core
 * \brief Linear map with small-buffer optimization.
 * \code{.cpp}
 * template<std::equality_comparable K, typename V,
 *          size_t MinSVOCapacity = 8, Allocator A = Mallocator>
 * class SmallLinearMap;
 * \endcode
 *
 * Entries are stored contiguously and erasure does not preserve order.
 *
 * \attention 1. Insertion and reserve() may invalidate all iterators and
 * references.
 * \attention 2. Erasure moves the last entry into the erased position.
 */
template <std::equality_comparable K, typename V,
          std::size_t MinSVOCapacity = 8, Allocator A = Mallocator>
class SmallLinearMap {
  using ATraits = AllocatorTraits<A>;

public:
  using size_type = std::size_t;

  /**
   * \brief Iterator over map entries.
   * \code{.cpp}
   * template<bool Const>
   * class Iterator;
   * \endcode
   *
   * Iterators expose entries as a pair of key and value references.
   */
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

    /**
     * \brief Constructs, copies, or converts an iterator.
     * \code{.cpp}
     * Iterator() = default;
     * Iterator(const Iterator&) = default;
     * Iterator& operator=(const Iterator&) = default;
     * Iterator(const Iterator<false>& other) requires Const;
     * \endcode
     * \param other Iterator to copy or convert.
     */
    Iterator() = default;
    Iterator(const Iterator &) = default;
    Iterator &operator=(const Iterator &) = default;

    Iterator(const Iterator<false> &other)
      requires Const
        : m_key(other.m_key), m_value(other.m_value) {}

    /**
     * \brief Accesses the current entry.
     * \code{.cpp}
     * reference operator*() const;
     * \endcode
     * \return The current key-value pair.
     * \attention 1. The iterator must refer to an entry.
     */
    reference operator*() const { return {*m_key, *m_value}; }

    /**
     * \brief Advances the iterator.
     * \code{.cpp}
     * Iterator& operator++();
     * Iterator operator++(int);
     * \endcode
     * \return The advanced iterator or its previous value.
     */
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

    /**
     * \brief Compares two iterators.
     * \code{.cpp}
     * template<bool OtherConst>
     * bool operator==(const Iterator<OtherConst>& other) const;
     * \endcode
     * \param other Iterator to compare with.
     * \return Whether both iterators refer to the same entry.
     */
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

  /**
   * \brief Constructs, copies, moves, assigns, or destroys a map.
   * \code{.cpp}
   * explicit SmallLinearMap(const A& allocator = {});
   * SmallLinearMap(const SmallLinearMap& other);
   * SmallLinearMap(SmallLinearMap&& other);
   * SmallLinearMap& operator=(const SmallLinearMap& other);
   * SmallLinearMap& operator=(SmallLinearMap&& other);
   * ~SmallLinearMap();
   * \endcode
   * \param allocator Allocator to use.
   * \param other Map to copy or move from.
   *
   * Moved-from maps are empty.
   */
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

  /**
   * \brief Inserts a key-value pair if the key is absent.
   * \code{.cpp}
   * std::pair<iterator, bool> insert(const K& key, const V& value);
   * std::pair<iterator, bool> insert(const K& key, V&& value);
   * std::pair<iterator, bool> insert(K&& key, const V& value);
   * std::pair<iterator, bool> insert(K&& key, V&& value);
   * \endcode
   * \param key Key to insert.
   * \param value Value to insert.
   * \return Iterator to the entry and whether insertion occurred.
   */
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

  /**
   * \brief Erases an entry by key.
   * \code{.cpp}
   * bool erase(const K& key);
   * \endcode
   * \param key Key to erase.
   * \return Whether an entry was erased.
   *
   * The last entry moves into the erased position.
   */
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

  /**
   * \brief Checks whether a key exists.
   * \code{.cpp}
   * bool containsKey(const K& key) const;
   * \endcode
   * \param key Key to find.
   * \return Whether the key exists.
   */
  [[nodiscard]]
  bool containsKey(const K &key) const {
    return find_index(key) != m_size;
  }

  /**
   * \brief Returns the number of entries.
   * \code{.cpp}
   * size_type size() const noexcept;
   * \endcode
   * \return Number of entries.
   */
  [[nodiscard]]
  size_type size() const noexcept {
    return m_size;
  }

  /**
   * \brief Checks whether the map is empty.
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
   * \brief Returns the entry capacity.
   * \code{.cpp}
   * size_type capacity() const noexcept;
   * \endcode
   * \return Number of entries that fit without allocation.
   */
  [[nodiscard]]
  size_type capacity() const noexcept {
    if (!m_heapKeys)
      return MinSVOCapacity;

    return std::min(m_keyCapacity, m_valueCapacity);
  }

  /**
   * \brief Returns a view of the keys.
   * \code{.cpp}
   * span<const K> keys() const noexcept;
   * \endcode
   * \return View of the stored keys.
   */
  [[nodiscard]]
  span<const K> keys() const noexcept {
    return {key_data(), m_size};
  }

  /**
   * \brief Returns a view of the mapped values.
   * \code{.cpp}
   * span<V> values() noexcept;
   * span<const V> values() const noexcept;
   * \endcode
   * \return View of the stored values.
   */
  [[nodiscard]]
  span<V> values() noexcept {
    return {value_data(), m_size};
  }

  [[nodiscard]]
  span<const V> values() const noexcept {
    return {value_data(), m_size};
  }

  /**
   * \brief Returns an iterator to the first entry.
   * \code{.cpp}
   * iterator begin() noexcept;
   * const_iterator begin() const noexcept;
   * \endcode
   * \return Iterator to the first entry.
   */
  iterator begin() noexcept { return {key_data(), value_data()}; }

  /**
   * \brief Returns an iterator past the last entry.
   * \code{.cpp}
   * iterator end() noexcept;
   * const_iterator end() const noexcept;
   * \endcode
  * \return Iterator past the last entry.
  */
  iterator end() noexcept {
    return {key_data() + m_size, value_data() + m_size};
  }
  const_iterator begin() const noexcept { return {key_data(), value_data()}; }
  const_iterator end() const noexcept {
    return {key_data() + m_size, value_data() + m_size};
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

  /**
   * \brief Removes all entries.
   * \code{.cpp}
   * void clear() noexcept;
   * \endcode
   *
   * Capacity is retained.
   */
  void clear() noexcept {
    std::destroy_n(key_data(), m_size);
    std::destroy_n(value_data(), m_size);
    m_size = 0;
  }

  /**
   * \brief Reserves capacity for entries.
   * \code{.cpp}
   * void reserve(size_type count);
   * \endcode
   * \param count Desired number of entries.
   *
   * \attention 1. \p count must not exceed max_size().
   */
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

namespace fmt {

/**
 * \brief Formats a small linear map as a brace-enclosed list of entries.
 * \code{.cpp}
 * template<std::equality_comparable K, typename V, size_t MinSVOCapacity,
 *          strobe::Allocator A>
 * struct formatter<strobe::SmallLinearMap<K, V, MinSVOCapacity, A>>;
 * \endcode
 */
template <std::equality_comparable K, typename V, size_t MinSVOCapacity,
          strobe::Allocator A>
struct formatter<strobe::SmallLinearMap<K, V, MinSVOCapacity, A>> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }

  template <typename FormatContext>
  auto format(const strobe::SmallLinearMap<K, V, MinSVOCapacity, A> &map,
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
