#pragma once

#include <cassert>
#include <concepts>
#include <cstddef>
#include <iterator>
#include <fmt/format.h>
#include <memory>
#include <type_traits>
#include <utility>

#include "strobe/core/containers/span.hpp"

namespace strobe {

/**
 * \ingroup core
 * \brief Fixed-capacity map with linear search.
 * \code{.cpp}
 * template<std::equality_comparable K, typename V, size_t Capacity>
 * class InplaceLinearMap;
 * \endcode
 *
 * Stores keys and values in separate inline arrays.
 *
 * \attention 1. The map cannot contain more than Capacity entries.
 * \attention 2. Keys must be equality-comparable.
 * \attention 3. Erasing an entry may reorder the remaining entries.
 * \attention 4. Insertion and erasure invalidate iterators and spans.
 */
template <std::equality_comparable K, typename V, std::size_t Capacity>
class InplaceLinearMap {
public:
  using size_type = std::size_t;

  /**
   * \brief Iterator over map entries.
   * \code{.cpp}
   * template<bool Const>
   * class Iterator;
   * \endcode
   *
   * Entries are exposed as pairs of references to the key and value.
   */
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
        : m_key(other.m_key), m_value(other.m_value) {}

    /**
     * \brief Accesses the current entry.
     * \code{.cpp}
     * reference operator*() const;
     * \endcode
     *
     * \return The current key-value pair.
     *
     * \attention 1. The iterator must refer to an entry.
     */
    reference operator*() const {
      return {*m_key, *m_value};
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
     *
     * \param other Iterator to compare with.
     *
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
   * \brief Constructs, copies, or moves a map.
   * \code{.cpp}
   * InplaceLinearMap();
   * InplaceLinearMap(const InplaceLinearMap& other);
   * InplaceLinearMap(InplaceLinearMap&& other);
   * InplaceLinearMap& operator=(const InplaceLinearMap& other);
   * InplaceLinearMap& operator=(InplaceLinearMap&& other);
   * ~InplaceLinearMap();
   * \endcode
   *
   * \param other Map to copy or move from.
   *
   * Moved-from maps are empty.
   */
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

  /**
   * \brief Inserts a key-value pair if the key is absent.
   * \code{.cpp}
   * std::pair<iterator, bool> insert(const K& key, const V& value);
   * std::pair<iterator, bool> insert(const K& key, V&& value);
   * std::pair<iterator, bool> insert(K&& key, const V& value);
   * std::pair<iterator, bool> insert(K&& key, V&& value);
   * \endcode
   *
   * \param key Key to insert.
   * \param value Value to insert.
   *
   * \return Iterator to the entry and whether insertion occurred.
   *
   * \attention 1. The map must have fewer than Capacity entries when inserting
   * a new key.
   */
  std::pair<iterator, bool> insert(const K &key, const V &value) {
    const size_type index = find_index(key);

    if (index != m_size)
      return {iterator(key_data() + index, value_data() + index), false};

    append(key, value);
    return {iterator(key_data() + m_size - 1,
                     value_data() + m_size - 1), true};
  }

  std::pair<iterator, bool> insert(const K &key, V &&value) {
    const size_type index = find_index(key);

    if (index != m_size)
      return {iterator(key_data() + index, value_data() + index), false};

    append(key, std::move(value));
    return {iterator(key_data() + m_size - 1,
                     value_data() + m_size - 1), true};
  }

  std::pair<iterator, bool> insert(K &&key, const V &value) {
    const size_type index = find_index(key);

    if (index != m_size)
      return {iterator(key_data() + index, value_data() + index), false};

    append(std::move(key), value);
    return {iterator(key_data() + m_size - 1,
                     value_data() + m_size - 1), true};
  }

  std::pair<iterator, bool> insert(K &&key, V &&value) {
    const size_type index = find_index(key);

    if (index != m_size)
      return {iterator(key_data() + index, value_data() + index), false};

    append(std::move(key), std::move(value));
    return {iterator(key_data() + m_size - 1,
                     value_data() + m_size - 1), true};
  }

  /**
   * \brief Erases an entry by key.
   * \code{.cpp}
   * bool erase(const K& key);
   * \endcode
   *
   * \param key Key to erase.
   *
   * \return Whether an entry was erased.
   *
   * \attention 1. Erasing may reorder the remaining entries.
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
   *
   * \param key Key to find.
   *
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
   *
   * \return Number of entries.
   */
  [[nodiscard]]
  size_type size() const noexcept {
    return m_size;
  }

  /**
   * \brief Returns the map capacity.
   * \code{.cpp}
   * static constexpr size_type capacity() noexcept;
   * \endcode
   */
  [[nodiscard]]
  static constexpr size_type capacity() noexcept {
    return Capacity;
  }

  /**
   * \brief Checks whether the map is empty.
   * \code{.cpp}
   * bool empty() const noexcept;
   * \endcode
   */
  [[nodiscard]]
  bool empty() const noexcept {
    return m_size == 0;
  }

  /**
   * \brief Returns views of the keys or values.
   * \code{.cpp}
   * span<const K> keys() const noexcept;
   * \endcode
   *
   * \return A view over the requested elements.
   *
   * \attention 1. Returned spans are invalidated by insertion and erasure.
   */
  [[nodiscard]]
  span<const K> keys() const noexcept {
    return {key_data(), m_size};
  }

  /**
   * \brief Returns a mutable view of the values.
   * \code{.cpp}
   * span<V> values() noexcept;
   * \endcode
   */
  [[nodiscard]]
  span<V> values() noexcept {
    return {value_data(), m_size};
  }

  /**
   * \brief Returns a read-only view of the values.
   * \code{.cpp}
   * span<const V> values() const noexcept;
   * \endcode
   */
  [[nodiscard]]
  span<const V> values() const noexcept {
    return {value_data(), m_size};
  }

  /**
   * \brief Returns iterators to the entries.
   * \code{.cpp}
   * iterator begin() noexcept;
   * \endcode
   *
   * \return Iterator delimiting the map.
   */
  iterator begin() noexcept {
    return {key_data(), value_data()};
  }

  /**
   * \brief Returns the iterator past the last entry.
   * \code{.cpp}
   * iterator end() noexcept;
   * \endcode
   */
  iterator end() noexcept {
    return {key_data() + m_size, value_data() + m_size};
  }

  /**
   * \brief Returns a read-only iterator to the first entry.
   * \code{.cpp}
   * const_iterator begin() const noexcept;
   * \endcode
   */
  const_iterator begin() const noexcept {
    return {key_data(), value_data()};
  }

  /**
   * \brief Returns a read-only iterator past the last entry.
   * \code{.cpp}
   * const_iterator end() const noexcept;
   * \endcode
   */
  const_iterator end() const noexcept {
    return {key_data() + m_size, value_data() + m_size};
  }

  /**
   * \brief Returns a read-only iterator to the first entry.
   * \code{.cpp}
   * const_iterator cbegin() const noexcept;
   * \endcode
   */
  const_iterator cbegin() const noexcept {
    return begin();
  }

  /**
   * \brief Returns a read-only iterator past the last entry.
   * \code{.cpp}
   * const_iterator cend() const noexcept;
   * \endcode
   */
  const_iterator cend() const noexcept {
    return end();
  }

  /**
   * \brief Removes all entries.
   * \code{.cpp}
   * void clear() noexcept;
   * \endcode
   */
  void clear() noexcept {
    std::destroy_n(key_data(), m_size);
    std::destroy_n(value_data(), m_size);
    m_size = 0;
  }

private:
  K *key_data() noexcept {
    return reinterpret_cast<K *>(m_keyStorage);
  }

  const K *key_data() const noexcept {
    return reinterpret_cast<const K *>(m_keyStorage);
  }

  V *value_data() noexcept {
    return reinterpret_cast<V *>(m_valueStorage);
  }

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

  alignas(K) std::byte m_keyStorage[sizeof(K) * (Capacity ? Capacity : 1)];
  alignas(V) std::byte m_valueStorage[sizeof(V) * (Capacity ? Capacity : 1)];
  size_type m_size = 0;
};

} // namespace strobe

namespace fmt {

template <std::equality_comparable K, typename V, std::size_t Capacity>
struct formatter<strobe::InplaceLinearMap<K, V, Capacity>> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }

  template <typename FormatContext>
  auto format(const strobe::InplaceLinearMap<K, V, Capacity> &map,
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
