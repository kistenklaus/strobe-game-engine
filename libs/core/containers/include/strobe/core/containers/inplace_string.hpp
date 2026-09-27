#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <compare>
#include <cstddef>
#include <functional>
#include <fmt/format.h>
#include <initializer_list>
#include <iterator>
#include <string_view>
#include <type_traits>

namespace strobe {

/**
 * \ingroup core
 * \brief Fixed-capacity string with inline storage.
 * \code{.cpp}
 * template<size_t Capacity, typename C = char>
 * class InplaceString;
 * \endcode
 *
 * Stores at most \p Capacity characters and always keeps a trailing null
 * character.
 *
 * \attention 1. The string cannot contain more than Capacity characters.
 * \attention 2. Element access at index size() refers to the null terminator.
 * \attention 3. Mutating operations may invalidate pointers, references, views,
 * and iterators.
 */
template <size_t Capacity, typename C = char> class InplaceString {
public:
  using value_type = C;
  using traits_type = std::char_traits<C>;
  using size_type = size_t;
  using difference_type = std::ptrdiff_t;
  using reference = C &;
  using const_reference = const C &;
  using pointer = C *;
  using const_pointer = const C *;
  using iterator = C *;
  using const_iterator = const C *;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using view_type = std::basic_string_view<C, traits_type>;

  /**
   * \brief Sentinel returned by search functions when no match exists.
   * \code{.cpp}
   * static constexpr size_type npos = size_type(-1);
   * \endcode
   */
  static constexpr size_type npos = size_type(-1);
  static_assert(Capacity < npos);

  /**
   * \brief Constructs, copies, or moves a string.
   * \code{.cpp}
   * constexpr InplaceString() noexcept;
   * constexpr InplaceString(const InplaceString& other);
   * constexpr InplaceString(InplaceString&& other);
   * constexpr InplaceString& operator=(const InplaceString& other);
   * constexpr InplaceString& operator=(InplaceString&& other);
   * \endcode
   *
   * \param other String to copy or move from.
   *
   * The default constructor creates an empty string.
   */
  constexpr InplaceString() noexcept = default;

  /**
   * \brief Constructs a string from a view.
   * \code{.cpp}
   * constexpr InplaceString(view_type value);
   * \endcode
   * \param value Source string.
   * \attention 1. The resulting string must fit within Capacity.
   */
  constexpr InplaceString(view_type value) { assign(value); }

  /**
   * \brief Constructs a string from a null-terminated pointer.
   * \code{.cpp}
   * constexpr InplaceString(const C* value);
   * \endcode
   * \param value Source string.
   * \attention 1. \p value must be null-terminated.
   */
  constexpr InplaceString(const C *value) : InplaceString(view_type(value)) {}

  /**
   * \brief Constructs a string from a value or repeated character.
   * \code{.cpp}
   * constexpr InplaceString(size_type count, C value);
   * \endcode
   *
   * \param count Number of repeated characters.
   * \param value Character to repeat.
   *
   * \attention 1. The resulting string must fit within Capacity.
   */
  constexpr InplaceString(size_type count, C value) { append(count, value); }

  /**
   * \brief Returns the string size.
   * \code{.cpp}
   * constexpr size_type size() const noexcept;
   * \endcode
   *
   * \return Number of characters.
   */
  [[nodiscard]] constexpr size_type size() const noexcept { return m_size; }

  /**
   * \brief Returns the string length.
   * \code{.cpp}
   * constexpr size_type length() const noexcept;
   * \endcode
   * \return String length.
   */
  [[nodiscard]] constexpr size_type length() const noexcept { return m_size; }

  /**
   * \brief Checks whether the string is empty.
   * \code{.cpp}
   * constexpr bool empty() const noexcept;
   * \endcode
   * \return Whether size() is zero.
   */
  [[nodiscard]] constexpr bool empty() const noexcept { return m_size == 0; }

  /**
   * \brief Returns the maximum string size.
   * \code{.cpp}
   * static constexpr size_type capacity() noexcept;
   * \endcode
   * \return Capacity in characters.
   */
  [[nodiscard]] static constexpr size_type capacity() noexcept {
    return Capacity;
  }

  /**
   * \brief Returns mutable character storage.
   * \code{.cpp}
   * constexpr pointer data() noexcept;
   * \endcode
   *
   * \return A pointer to the null-terminated character storage.
   */
  [[nodiscard]] constexpr pointer data() noexcept { return m_data.data(); }

  /**
   * \brief Returns read-only character storage.
   * \code{.cpp}
   * constexpr const_pointer data() const noexcept;
   * \endcode
   * \return Character storage.
   */
  [[nodiscard]] constexpr const_pointer data() const noexcept {
    return m_data.data();
  }

  /**
   * \brief Returns the null-terminated character pointer.
   * \code{.cpp}
   * constexpr const_pointer c_str() const noexcept;
   * \endcode
   * \return Null-terminated storage.
   */
  [[nodiscard]] constexpr const_pointer c_str() const noexcept {
    return data();
  }

  /**
   * \brief Returns an iterator to the first character.
   * \code{.cpp}
   * constexpr iterator begin() noexcept;
   * constexpr const_iterator begin() const noexcept;
   * \endcode
   * \return Iterator to the first character.
   */
  [[nodiscard]] constexpr iterator begin() noexcept { return data(); }

  [[nodiscard]] constexpr const_iterator begin() const noexcept {
    return data();
  }

  /**
   * \brief Returns a const iterator to the first character.
   * \code{.cpp}
   * constexpr const_iterator cbegin() const noexcept;
   * \endcode
   * \return Const iterator to the first character.
   */
  [[nodiscard]] constexpr const_iterator cbegin() const noexcept {
    return data();
  }

  /**
   * \brief Returns an iterator past the last character.
   * \code{.cpp}
   * constexpr iterator end() noexcept;
   * constexpr const_iterator end() const noexcept;
   * \endcode
   * \return Iterator past the last character.
   */
  [[nodiscard]] constexpr iterator end() noexcept { return data() + m_size; }

  [[nodiscard]] constexpr const_iterator end() const noexcept {
    return data() + m_size;
  }

  /**
   * \brief Returns a const iterator past the last character.
   * \code{.cpp}
   * constexpr const_iterator cend() const noexcept;
   * \endcode
   * \return Const iterator past the last character.
   */
  [[nodiscard]] constexpr const_iterator cend() const noexcept {
    return data() + m_size;
  }

  /**
   * \brief Returns a reverse iterator to the last character.
   * \code{.cpp}
   * constexpr reverse_iterator rbegin() noexcept;
   * constexpr const_reverse_iterator rbegin() const noexcept;
   * \endcode
   * \return Reverse iterator to the last character.
   */
  [[nodiscard]] constexpr reverse_iterator rbegin() noexcept {
    return reverse_iterator(end());
  }

  [[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept {
    return const_reverse_iterator(end());
  }

  /**
   * \brief Returns a reverse iterator before the first character.
   * \code{.cpp}
   * constexpr reverse_iterator rend() noexcept;
   * constexpr const_reverse_iterator rend() const noexcept;
   * \endcode
   * \return Reverse iterator before the first character.
   */
  [[nodiscard]] constexpr reverse_iterator rend() noexcept {
    return reverse_iterator(begin());
  }

  [[nodiscard]] constexpr const_reverse_iterator rend() const noexcept {
    return const_reverse_iterator(begin());
  }

  /**
   * \brief Accesses a character.
   * \code{.cpp}
   * constexpr reference operator[](size_type index) noexcept;
   * constexpr const_reference operator[](size_type index) const noexcept;
   * \endcode
   *
   * \param index Character index.
   *
   * \return Reference to the character.
   *
   * \attention 1. \p index must be less than or equal to size().
   */
  [[nodiscard]] constexpr reference operator[](size_type index) noexcept {
    assert(index <= m_size);
    return m_data[index];
  }

  [[nodiscard]] constexpr const_reference
  operator[](size_type index) const noexcept {
    assert(index <= m_size);
    return m_data[index];
  }

  /**
   * \brief Returns the first character.
   * \code{.cpp}
   * constexpr reference front() noexcept;
   * constexpr const_reference front() const noexcept;
   * \endcode
   *
   * \return Reference to the requested character.
   *
   * \attention 1. The string must not be empty.
   */
  [[nodiscard]] constexpr reference front() noexcept {
    assert(!empty());
    return m_data[0];
  }

  [[nodiscard]] constexpr const_reference front() const noexcept {
    assert(!empty());
    return m_data[0];
  }

  /**
   * \brief Returns the last character.
   * \code{.cpp}
   * constexpr reference back() noexcept;
   * constexpr const_reference back() const noexcept;
   * \endcode
   * \return Reference to the last character.
   * \attention 1. The string must not be empty.
   */
  [[nodiscard]] constexpr reference back() noexcept {
    assert(!empty());
    return m_data[m_size - 1];
  }

  [[nodiscard]] constexpr const_reference back() const noexcept {
    assert(!empty());
    return m_data[m_size - 1];
  }

  /**
   * \brief Returns a string view.
   * \code{.cpp}
   * constexpr view_type view() const noexcept;
   * \endcode
   * \return A view over the string characters.
   */
  [[nodiscard]] constexpr view_type view() const noexcept {
    return {data(), m_size};
  }

  /**
   * \brief Converts to a string view.
   * \code{.cpp}
   * constexpr operator view_type() const noexcept;
   * \endcode
   * \return A view over the string characters.
   */
  [[nodiscard]] constexpr operator view_type() const noexcept { return view(); }

  /**
   * \brief Clears the string.
   * \code{.cpp}
   * constexpr void clear() noexcept;
   * \endcode
   */
  constexpr void clear() noexcept {
    m_size = 0;
    m_data[0] = C{};
  }

  /**
   * \brief Appends a character.
   * \code{.cpp}
   * constexpr void push_back(C value);
   * \endcode
   *
   * \param value Character to append.
   *
   * \attention 1. The string must contain fewer than capacity() characters.
   */
  constexpr void push_back(C value) {
    assert(m_size < Capacity && "InplaceString is full");
    m_data[m_size++] = value;
    m_data[m_size] = C{};
  }

  /**
   * \brief Removes the final character.
   * \code{.cpp}
   * constexpr void pop_back() noexcept;
   * \endcode
   * \attention 1. The string must not be empty.
   */
  constexpr void pop_back() noexcept {
    assert(!empty());
    m_data[--m_size] = C{};
  }

  /**
   * \brief Changes the string size.
   * \code{.cpp}
   * constexpr void resize(size_type count, C value = C{});
   * \endcode
   *
   * \param count New string size.
   * \param value Character used to initialize appended elements.
   *
   * \attention 1. \p count must not exceed capacity().
   */
  constexpr void resize(size_type count, C value = C{}) {
    assert(count <= Capacity && "InplaceString capacity exceeded");

    if (count > m_size) {
      traits_type::assign(data() + m_size, count - m_size, value);
    }

    m_size = count;
    m_data[m_size] = C{};
  }

  /**
   * \brief Replaces string contents.
   * \code{.cpp}
   * constexpr InplaceString& assign(view_type value);
   * \endcode
   *
   * \param value Source string.
   *
   * \return This string.
   *
   * \attention 1. The resulting string must fit within Capacity.
   */
  constexpr InplaceString &assign(view_type value) {
    assert(value.size() <= Capacity && "InplaceString capacity exceeded");

    if (!value.empty()) {
      traits_type::move(data(), value.data(), value.size());
    }

    m_size = value.size();
    m_data[m_size] = C{};
    return *this;
  }

  /**
   * \brief Replaces the contents with a substring.
   * \code{.cpp}
   * constexpr InplaceString& assign(view_type value, size_type pos, size_type count = size_type(-1));
   * \endcode
   * \param value Source view.
   * \param pos Starting position.
   * \param count Maximum number of characters.
   * \return This string.
   */
  constexpr InplaceString &assign(view_type value, size_type pos,
                                  size_type count = size_type(-1)) {
    return assign(value.substr(pos, count));
  }

  /**
   * \brief Replaces the contents from a character pointer.
   * \code{.cpp}
   * constexpr InplaceString& assign(const C* value, size_type count);
   * \endcode
   * \param value Source characters.
   * \param count Number of characters.
   * \return This string.
   */
  constexpr InplaceString &assign(const C *value, size_type count) {
    return assign(view_type(value, count));
  }

  /**
   * \brief Replaces the contents from an initializer list.
   * \code{.cpp}
   * constexpr InplaceString& assign(std::initializer_list<C> value);
   * \endcode
   * \param value Characters to copy.
   * \return This string.
   */
  constexpr InplaceString &assign(std::initializer_list<C> value) {
    return assign(value.begin(), value.end());
  }

  /**
   * \brief Replaces the contents with repeated characters.
   * \code{.cpp}
   * constexpr InplaceString& assign(size_type count, C value);
   * \endcode
   * \param count Number of characters.
   * \param value Character to repeat.
   * \return This string.
   * \attention 1. \p count must not exceed Capacity.
   */
  constexpr InplaceString &assign(size_type count, C value) {
    assert(count <= Capacity && "InplaceString capacity exceeded");
    m_size = count;
    traits_type::assign(data(), count, value);
    m_data[m_size] = C{};
    return *this;
  }

  /**
   * \brief Replaces the contents from an iterator range.
   * \code{.cpp}
   * template<class InputIt> constexpr InplaceString& assign(InputIt first, InputIt last);
   * \endcode
   * \param first Range beginning.
   * \param last Range end.
   * \return This string.
   * \attention 1. The range length must not exceed Capacity.
   */
  template <class InputIt>
  constexpr InplaceString &assign(InputIt first, InputIt last) {
    m_size = 0;
    for (; first != last; ++first) push_back(static_cast<C>(*first));
    return *this;
  }

  /**
   * \brief Appends a string view.
   * \code{.cpp}
   * constexpr InplaceString& append(view_type value);
   * \endcode
   * \param value Characters to append.
   * \return This string.
   * \attention 1. The resulting string must fit within Capacity.
   */
  constexpr InplaceString &append(view_type value) {
    assert(value.size() <= Capacity - m_size &&
           "InplaceString capacity exceeded");

    if (!value.empty()) {
      traits_type::move(data() + m_size, value.data(), value.size());
      m_size += value.size();
      m_data[m_size] = C{};
    }

    return *this;
  }

  /**
   * \brief Appends a null-terminated string.
   * \code{.cpp}
   * constexpr InplaceString& append(const C* value);
   * \endcode
   * \param value String to append.
   * \return This string.
   */
  constexpr InplaceString &append(const C *value) {
    return append(view_type(value));
  }

  /**
   * \brief Appends repeated characters.
   * \code{.cpp}
   * constexpr InplaceString& append(size_type count, C value);
   * \endcode
   * \param count Number of characters.
   * \param value Character to append.
   * \return This string.
   */
  constexpr InplaceString &append(size_type count, C value) {
    assert(count <= Capacity - m_size && "InplaceString capacity exceeded");

    if (count != 0) {
      traits_type::assign(data() + m_size, count, value);
      m_size += count;
      m_data[m_size] = C{};
    }

    return *this;
  }

  /**
   * \brief Appends characters from an iterator range.
   * \code{.cpp}
   * template<std::input_iterator InputIt>
   * constexpr InplaceString& append(InputIt first, InputIt last);
   * \endcode
   *
   * \param first Range beginning.
   * \param last Range end.
   * \return This string.
   * \attention 1. The resulting string must fit within Capacity.
   */
  template <std::input_iterator InputIt>
  constexpr InplaceString &append(InputIt first, InputIt last) {
    for (; first != last; ++first) {
      push_back(static_cast<C>(*first));
    }
    return *this;
  }

  /**
   * \brief Appends a string view.
   * \code{.cpp}
   * constexpr InplaceString& operator+=(view_type value);
   * \endcode
   * \param value Characters to append.
   * \return This string.
   */
  constexpr InplaceString &operator+=(view_type value) { return append(value); }

  /**
   * \brief Appends a null-terminated string.
   * \code{.cpp}
   * constexpr InplaceString& operator+=(const C* value);
   * \endcode
   * \param value String to append.
   * \return This string.
   */
  constexpr InplaceString &operator+=(const C *value) { return append(value); }

  /**
   * \brief Appends one character.
   * \code{.cpp}
   * constexpr InplaceString& operator+=(C value);
   * \endcode
   * \param value Character to append.
   * \return This string.
   */
  constexpr InplaceString &operator+=(C value) {
    push_back(value);
    return *this;
  }

  /**
   * \brief Inserts characters at a position.
   * \code{.cpp}
   * constexpr InplaceString& insert(size_type pos, view_type value);
   * constexpr InplaceString& insert(size_type pos, const C* value);
   * \endcode
   * \param pos Insertion position.
   * \param value Characters to insert.
   * \return This string.
   * \attention 1. \p pos must not exceed size().
   * \attention 2. The resulting string must fit within Capacity.
   */
  constexpr InplaceString &insert(size_type pos, view_type value) {
    assert(pos <= m_size);
    assert(value.size() <= Capacity - m_size &&
           "InplaceString capacity exceeded");

    if (value.empty()) {
      return *this;
    }

    if (aliases(value)) {
      std::array<C, Capacity + 1> snapshot{};
      traits_type::copy(snapshot.data(), value.data(), value.size());
      return insert_external(pos, {snapshot.data(), value.size()});
    }

    return insert_external(pos, value);
  }

  constexpr InplaceString &insert(size_type pos, const C *value) {
    return insert(pos, view_type(value));
  }

  /**
   * \brief Erases characters from a position.
   * \code{.cpp}
   * constexpr InplaceString& erase(size_type pos = 0,
   *                                size_type count = size_type(-1));
   * \endcode
   * \param pos Erasure position.
   * \param count Maximum number of characters to erase.
   * \return This string.
   * \attention 1. \p pos must not exceed size().
   */
  constexpr InplaceString &erase(size_type pos = 0, size_type count = size_type(-1)) {
    assert(pos <= m_size);
    const size_type removed = std::min(count, m_size - pos);

    if (removed != 0) {
      traits_type::move(data() + pos, data() + pos + removed,
                        m_size - pos - removed + 1);
      m_size -= removed;
    }

    return *this;
  }

  /**
   * \brief Replaces characters at a position.
   * \code{.cpp}
   * constexpr InplaceString& replace(size_type pos, size_type count,
   *                                   view_type value);
   * \endcode
   * \param pos Replacement position.
   * \param count Maximum number of characters to erase.
   * \param value Replacement characters.
   * \return This string.
   * \attention 1. \p pos must not exceed size().
   * \attention 2. The resulting string must fit within Capacity.
   */
  constexpr InplaceString &replace(size_type pos, size_type count,
                                   view_type value) {
    assert(pos <= m_size);
    const size_type removed = std::min(count, m_size - pos);
    assert(value.size() <= Capacity - (m_size - removed) &&
           "InplaceString capacity exceeded");

    if (aliases(value)) {
      std::array<C, Capacity + 1> snapshot{};
      traits_type::copy(snapshot.data(), value.data(), value.size());
      return replace_external(pos, removed, {snapshot.data(), value.size()});
    }

    return replace_external(pos, removed, value);
  }

  /**
   * \brief Returns a substring.
   * \code{.cpp}
   * constexpr InplaceString substr(size_type pos = 0,
   *                                size_type count = size_type(-1)) const;
   * \endcode
   *
   * \param pos Starting position.
   * \param count Maximum substring length.
   *
   * \return The requested string.
   *
   * \attention 1. \p pos must be less than or equal to size().
   */
  [[nodiscard]] constexpr InplaceString substr(size_type pos = 0,
                                               size_type count = size_type(-1)) const {
    assert(pos <= m_size);
    return InplaceString(view().substr(pos, count));
  }

  /**
   * \brief Compares the string with another view.
   * \code{.cpp}
   * constexpr int compare(view_type other) const noexcept;
   * \endcode
   * \param other String to compare with.
   * \return Negative, zero, or positive according to lexical order.
   */
  [[nodiscard]] constexpr int compare(view_type other) const noexcept {
    return view().compare(other);
  }

  /**
   * \brief Finds the first occurrence of a value.
   * \code{.cpp}
   * constexpr size_type find(view_type value,
   *                          size_type pos = 0) const noexcept;
   * constexpr size_type find(C value,
   *                          size_type pos = 0) const noexcept;
   * \endcode
   * \param value Value to find.
   * \param pos Search position.
   * \return Position of the match, or npos.
   */
  [[nodiscard]] constexpr size_type find(view_type value,
                                         size_type pos = 0) const noexcept {
    return view().find(value, pos);
  }

  [[nodiscard]] constexpr size_type find(C value,
                                         size_type pos = 0) const noexcept {
    return view().find(value, pos);
  }

  /**
   * \brief Finds the last occurrence of a view.
   * \code{.cpp}
   * constexpr size_type rfind(view_type value,
   *                           size_type pos = npos) const noexcept;
   * \endcode
   * \param value View to find.
   * \param pos Search position.
   * \return Position of the match, or npos.
   */
  [[nodiscard]] constexpr size_type rfind(view_type value,
                                          size_type pos = npos) const noexcept {
    return view().rfind(value, pos);
  }

  /**
   * \brief Checks whether the string starts with a view.
   * \code{.cpp}
   * constexpr bool starts_with(view_type value) const noexcept;
   * \endcode
   * \param value Prefix to check.
   * \return Whether the prefix matches.
   */
  [[nodiscard]] constexpr bool starts_with(view_type value) const noexcept {
    return view().starts_with(value);
  }

  /**
   * \brief Checks whether the string ends with a view.
   * \code{.cpp}
   * constexpr bool ends_with(view_type value) const noexcept;
   * \endcode
   * \param value Suffix to check.
   * \return Whether the suffix matches.
   */
  [[nodiscard]] constexpr bool ends_with(view_type value) const noexcept {
    return view().ends_with(value);
  }

  /**
   * \brief Checks whether the string contains a view.
   * \code{.cpp}
   * constexpr bool contains(view_type value) const noexcept;
   * \endcode
   * \param value View to find.
   * \return Whether the view occurs.
   */
  [[nodiscard]] constexpr bool contains(view_type value) const noexcept {
    return find(value) != npos;
  }

  /**
   * \brief Compares two strings for equality.
   * \code{.cpp}
   * constexpr bool operator==(const InplaceString& lhs,
   *                           const InplaceString& rhs) noexcept;
   * \endcode
   * \param lhs Left operand.
   * \param rhs Right operand.
   * \return Whether the strings are equal.
   */
  [[nodiscard]] friend constexpr bool
  operator==(const InplaceString &lhs, const InplaceString &rhs) noexcept {
    return lhs.view() == rhs.view();
  }

  /**
   * \brief Compares two strings lexically.
   * \code{.cpp}
   * constexpr auto operator<=>(const InplaceString& lhs,
   *                            const InplaceString& rhs) noexcept;
   * \endcode
   * \param lhs Left operand.
   * \param rhs Right operand.
   * \return The lexical ordering.
   */
  [[nodiscard]] friend constexpr auto
  operator<=>(const InplaceString &lhs, const InplaceString &rhs) noexcept {
    return lhs.view() <=> rhs.view();
  }

private:
  [[nodiscard]] constexpr bool aliases(view_type value) const noexcept {
    if (value.empty()) {
      return false;
    }

    if (std::is_constant_evaluated()) {
      for (size_type i = 0; i <= m_size; ++i) {
        if (value.data() == data() + i) {
          return true;
        }
      }

      return false;
    }

    std::less<const C *> less;
    return !less(value.data(), data()) &&
           less(value.data(), data() + m_size + 1);
  }

  constexpr InplaceString &insert_external(size_type pos, view_type value) {
    traits_type::move(data() + pos + value.size(), data() + pos,
                      m_size - pos + 1);
    traits_type::copy(data() + pos, value.data(), value.size());
    m_size += value.size();
    return *this;
  }

  constexpr InplaceString &replace_external(size_type pos, size_type removed,
                                            view_type value) {
    const size_type new_size = m_size - removed + value.size();

    if (value.size() != removed) {
      traits_type::move(data() + pos + value.size(), data() + pos + removed,
                        m_size - pos - removed + 1);
    }

    if (!value.empty()) {
      traits_type::copy(data() + pos, value.data(), value.size());
    }

    m_size = new_size;
    return *this;
  }

  std::array<C, Capacity + 1> m_data{};
  size_type m_size = 0;
};

} // namespace strobe

namespace std {

/**
 * \brief Hashes an inline string by its characters.
 * \code{.cpp}
 * template<size_t Capacity, typename C>
 * struct hash<strobe::InplaceString<Capacity, C>>;
 * \endcode
 */
template <size_t Capacity, typename C>
struct hash<strobe::InplaceString<Capacity, C>> {
  size_t operator()(const strobe::InplaceString<Capacity, C> &value) const
      noexcept(noexcept(std::hash<C>{}(*value.begin()))) {
    size_t result = static_cast<size_t>(1469598103934665603ull);
    for (C character : value) {
      result ^= std::hash<C>{}(character) +
                static_cast<size_t>(0x9e3779b97f4a7c15ull) +
                (result << 6) + (result >> 2);
    }
    return result;
  }
};

} // namespace std

namespace fmt {

/**
 * \brief Formats an inline string as its character sequence.
 * \code{.cpp}
 * template<size_t Capacity, typename C>
 * struct formatter<strobe::InplaceString<Capacity, C>>;
 * \endcode
 */
template <size_t Capacity, typename C>
struct formatter<strobe::InplaceString<Capacity, C>> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }

  template <typename FormatContext>
  auto format(const strobe::InplaceString<Capacity, C> &value,
              FormatContext &ctx) const {
    return fmt::format_to(ctx.out(), "{}", value.view());
  }
};

} // namespace fmt
