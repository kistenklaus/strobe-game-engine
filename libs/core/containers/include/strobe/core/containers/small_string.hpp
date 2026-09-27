#pragma once

#include "strobe/core/containers/small_vector.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <functional>
#include <fmt/format.h>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <string_view>
#include <type_traits>
#include <utility>

namespace strobe {

/**
 * \ingroup core
 * \brief String with inline storage and allocator-backed growth.
 * \code{.cpp}
 * template<size_t InlineCapacity = 23, Allocator A = Mallocator,
 *          typename C = char>
 * class SmallString;
 * \endcode
 */
template <size_t InlineCapacity = 23, Allocator A = Mallocator,
          typename C = char>
class SmallString {
public:
  using value_type = C;
  using traits_type = std::char_traits<C>;
  using allocator_type = A;
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

  /**
   * \brief Constructs, copies, or moves a string.
   * \code{.cpp}
   * explicit SmallString(const A& alloc = {});
   * SmallString(view_type value, const A& alloc = {});
   * SmallString(const C* value, const A& alloc = {});
   * SmallString(size_type count, C value, const A& alloc = {});
   * SmallString(const SmallString& other);
   * SmallString(SmallString&& other);
   * SmallString& operator=(const SmallString& other);
   * SmallString& operator=(SmallString&& other);
   * ~SmallString();
   * \endcode
   * \param value Initial string contents.
   * \param count Number of repeated characters.
   * \param other String to copy or move from.
   */
  explicit SmallString(const A &alloc = {}) : m_chars(alloc) {
    m_chars.push_back(C{});
  }

  SmallString(view_type value, const A &alloc = {}) : SmallString(alloc) {
    append(value);
  }

  SmallString(const C *value, const A &alloc = {})
      : SmallString(view_type(value), alloc) {}

  SmallString(size_type count, C value, const A &alloc = {})
      : SmallString(alloc) {
    append(count, value);
  }

  SmallString(const SmallString &) = default;

  SmallString(SmallString &&other) noexcept(
      std::is_nothrow_move_constructible_v<decltype(m_chars)>)
      : m_chars(std::move(other.m_chars)) {
    other.m_chars.push_back(C{});
  }

  SmallString &operator=(const SmallString &) = default;

  SmallString &operator=(SmallString &&other) noexcept(
      std::is_nothrow_move_assignable_v<decltype(m_chars)>) {
    if (this != &other) {
      m_chars = std::move(other.m_chars);
      other.m_chars.push_back(C{});
    }
    return *this;
  }

  /**
   * \brief Assigns string contents.
   * \code{.cpp}
   * SmallString& operator=(view_type value);
   * SmallString& operator=(const C* value);
   * \endcode
   * \param value Source string.
   * \return This string.
   */
  SmallString &operator=(view_type value) { return assign(value); }

  SmallString &operator=(const C *value) { return assign(view_type(value)); }

  /**
   * \brief Returns the number of characters.
   * \code{.cpp}
   * size_type size() const noexcept;
   * \endcode
   * \return Number of characters.
   */
  [[nodiscard]] size_type size() const noexcept { return m_chars.size() - 1; }

  /**
   * \brief Returns the string length.
   * \code{.cpp}
   * size_type length() const noexcept;
   * \endcode
   * \return Number of characters.
   */
  [[nodiscard]] size_type length() const noexcept { return size(); }

  /**
   * \brief Checks whether the string is empty.
   * \code{.cpp}
   * bool empty() const noexcept;
   * \endcode
   * \return Whether size() is zero.
   */
  [[nodiscard]] bool empty() const noexcept { return size() == 0; }

  /**
   * \brief Returns the character capacity.
   * \code{.cpp}
   * size_type capacity() const noexcept;
   * \endcode
   * \return Number of characters that fit without allocation.
   */
  [[nodiscard]] size_type capacity() const noexcept {
    return m_chars.capacity() - 1;
  }

  /**
   * \brief Returns the inline character capacity.
   * \code{.cpp}
   * static constexpr size_type inline_capacity() noexcept;
   * \endcode
   * \return Number of characters stored without allocation.
   */
  [[nodiscard]] static constexpr size_type inline_capacity() noexcept {
    return InlineCapacity;
  }

  /**
   * \brief Reserves character capacity.
   * \code{.cpp}
   * void reserve(size_type count);
   * \endcode
   * \param count Desired character capacity.
   */
  void reserve(size_type count) {
    assert(count < std::numeric_limits<size_type>::max());
    m_chars.reserve(count + 1);
  }

  /**
   * \brief Returns mutable character storage.
   * \code{.cpp}
   * pointer data() noexcept;
   * \endcode
   * \return Pointer to the null-terminated character storage.
   */
  [[nodiscard]] pointer data() noexcept { return m_chars.data(); }

  /**
   * \brief Returns read-only character storage.
   * \code{.cpp}
   * const_pointer data() const noexcept;
   * \endcode
   * \return Pointer to the null-terminated character storage.
   */
  [[nodiscard]] const_pointer data() const noexcept { return m_chars.data(); }

  /**
   * \brief Returns null-terminated character storage.
   * \code{.cpp}
   * const_pointer c_str() const noexcept;
   * \endcode
   * \return Pointer to the first character.
   */
  [[nodiscard]] const_pointer c_str() const noexcept { return data(); }

  /**
   * \brief Returns an iterator to the first character.
   * \code{.cpp}
   * iterator begin() noexcept;
   * const_iterator begin() const noexcept;
   * \endcode
   * \return Iterator to the first character.
   */
  [[nodiscard]] iterator begin() noexcept { return data(); }
  [[nodiscard]] const_iterator begin() const noexcept { return data(); }

  /**
   * \brief Returns a const iterator to the first character.
   * \code{.cpp}
   * const_iterator cbegin() const noexcept;
   * \endcode
   * \return Const iterator to the first character.
   */
  [[nodiscard]] const_iterator cbegin() const noexcept { return data(); }

  /**
   * \brief Returns an iterator past the last character.
   * \code{.cpp}
   * iterator end() noexcept;
   * const_iterator end() const noexcept;
   * \endcode
   * \return Iterator past the last character.
   */
  [[nodiscard]] iterator end() noexcept { return data() + size(); }
  [[nodiscard]] const_iterator end() const noexcept { return data() + size(); }

  /**
   * \brief Returns a const iterator past the last character.
   * \code{.cpp}
   * const_iterator cend() const noexcept;
   * \endcode
   * \return Const iterator past the last character.
   */
  [[nodiscard]] const_iterator cend() const noexcept { return data() + size(); }

  /**
   * \brief Returns a reverse iterator to the last character.
   * \code{.cpp}
   * reverse_iterator rbegin() noexcept;
   * const_reverse_iterator rbegin() const noexcept;
   * \endcode
   * \return Reverse iterator to the last character.
   */
  [[nodiscard]] reverse_iterator rbegin() noexcept {
    return reverse_iterator(end());
  }

  [[nodiscard]] const_reverse_iterator rbegin() const noexcept {
    return const_reverse_iterator(end());
  }

  /**
   * \brief Returns a reverse iterator before the first character.
   * \code{.cpp}
   * reverse_iterator rend() noexcept;
   * const_reverse_iterator rend() const noexcept;
   * \endcode
   * \return Reverse iterator before the first character.
   */
  [[nodiscard]] reverse_iterator rend() noexcept {
    return reverse_iterator(begin());
  }

  [[nodiscard]] const_reverse_iterator rend() const noexcept {
    return const_reverse_iterator(begin());
  }

  /**
   * \brief Accesses a character.
   * \code{.cpp}
   * reference operator[](size_type index) noexcept;
   * const_reference operator[](size_type index) const noexcept;
   * \endcode
   * \param index Character index.
   * \return Reference to the character.
   * \attention 1. \p index must not exceed size(); size() accesses the terminator.
   */
  [[nodiscard]] reference operator[](size_type index) noexcept {
    assert(index <= size());
    return data()[index];
  }

  [[nodiscard]] const_reference operator[](size_type index) const noexcept {
    assert(index <= size());
    return data()[index];
  }

  /**
   * \brief Returns the first character.
   * \code{.cpp}
   * reference front() noexcept;
   * const_reference front() const noexcept;
   * \endcode
   * \return Reference to the first character.
   * \attention 1. The string must not be empty.
   */
  [[nodiscard]] reference front() noexcept {
    assert(!empty());
    return data()[0];
  }

  [[nodiscard]] const_reference front() const noexcept {
    assert(!empty());
    return data()[0];
  }

  /**
   * \brief Returns the last character.
   * \code{.cpp}
   * reference back() noexcept;
   * const_reference back() const noexcept;
   * \endcode
   * \return Reference to the last character.
   * \attention 1. The string must not be empty.
   */
  [[nodiscard]] reference back() noexcept {
    assert(!empty());
    return data()[size() - 1];
  }

  [[nodiscard]] const_reference back() const noexcept {
    assert(!empty());
    return data()[size() - 1];
  }

  /**
   * \brief Returns a string view.
   * \code{.cpp}
   * view_type view() const noexcept;
   * \endcode
   * \return View of the characters.
   */
  [[nodiscard]] view_type view() const noexcept { return {data(), size()}; }

  /**
   * \brief Converts the string to a string view.
   * \code{.cpp}
   * operator view_type() const noexcept;
   * \endcode
   * \return View of the characters.
   */
  [[nodiscard]] operator view_type() const noexcept { return view(); }

  /**
   * \brief Removes all characters.
   * \code{.cpp}
   * void clear() noexcept;
   * \endcode
   */
  void clear() noexcept {
    m_chars.resize(1);
    m_chars[0] = C{};
  }

  /**
   * \brief Appends one character.
   * \code{.cpp}
   * void push_back(C value);
   * \endcode
   * \param value Character to append.
   */
  void push_back(C value) {
    const size_type old_size = size();
    m_chars.resize(old_size + 2);
    data()[old_size] = value;
    data()[old_size + 1] = C{};
  }

  /**
   * \brief Removes the final character.
   * \code{.cpp}
   * void pop_back() noexcept;
   * \endcode
   * \attention 1. The string must not be empty.
   */
  void pop_back() noexcept {
    assert(!empty());
    m_chars.pop_back();
    m_chars.back() = C{};
  }

  /**
   * \brief Changes the string length.
   * \code{.cpp}
   * void resize(size_type count, C value = C{});
   * \endcode
   * \param count New length.
   * \param value Character used for appended positions.
   */
  void resize(size_type count, C value = C{}) {
    assert(count < std::numeric_limits<size_type>::max());

    const size_type old_size = size();
    m_chars.resize(count + 1);

    if (count > old_size) {
      traits_type::assign(data() + old_size, count - old_size, value);
    }
    data()[count] = C{};
  }

  /**
   * \brief Replaces the string contents.
   * \code{.cpp}
   * SmallString& assign(view_type value);
   * \endcode
   * \param value Source string.
   * \return This string.
   */
  SmallString &assign(view_type value) {
    const size_type count = value.size();
    assert(count < std::numeric_limits<size_type>::max());

    if (aliases(value)) {
      // Source may begin inside the current string. Copy before shrinking.
      traits_type::move(data(), value.data(), count);
      m_chars.resize(count + 1);
    } else {
      m_chars.resize(count + 1);
      if (count != 0) {
        traits_type::copy(data(), value.data(), count);
      }
    }

    data()[count] = C{};
    return *this;
  }

  /**
   * \brief Replaces the contents with a substring.
   * \code{.cpp}
   * SmallString& assign(view_type value, size_type pos, size_type count = size_type(-1));
   * \endcode
   * \param value Source view.
   * \param pos Starting position.
   * \param count Maximum number of characters.
   * \return This string.
   */
  SmallString &assign(view_type value, size_type pos, size_type count = size_type(-1)) {
    return assign(value.substr(pos, count));
  }

  /**
   * \brief Replaces the contents from a character pointer.
   * \code{.cpp}
   * SmallString& assign(const C* value, size_type count);
   * \endcode
   * \param value Source characters.
   * \param count Number of characters.
   * \return This string.
   */
  SmallString &assign(const C *value, size_type count) {
    return assign(view_type(value, count));
  }

  /**
   * \brief Replaces the contents from an initializer list.
   * \code{.cpp}
   * SmallString& assign(std::initializer_list<C> value);
   * \endcode
   * \param value Characters to copy.
   * \return This string.
   */
  SmallString &assign(std::initializer_list<C> value) {
    return assign(value.begin(), value.end());
  }

  /**
   * \brief Replaces the contents with repeated characters.
   * \code{.cpp}
   * SmallString& assign(size_type count, C value);
   * \endcode
   * \param count Number of characters.
   * \param value Character to repeat.
   * \return This string.
   */
  SmallString &assign(size_type count, C value) { resize(count, value); return *this; }

  /**
   * \brief Replaces the contents from an iterator range.
   * \code{.cpp}
   * template<class InputIt> SmallString& assign(InputIt first, InputIt last);
   * \endcode
   * \param first Range beginning.
   * \param last Range end.
   * \return This string.
   */
  template <class InputIt> SmallString &assign(InputIt first, InputIt last) {
    clear();
    for (; first != last; ++first) push_back(static_cast<C>(*first));
    return *this;
  }

  /**
   * \brief Appends characters.
   * \code{.cpp}
   * SmallString& append(view_type value);
   * SmallString& append(const C* value);
   * SmallString& append(size_type count, C value);
   * template<std::input_iterator InputIt>
   * SmallString& append(InputIt first, InputIt last);
   * \endcode
   * \param value Characters or source string to append.
   * \param count Number of repeated characters.
   * \param first Range beginning.
   * \param last Range end.
   * \return This string.
   */
  SmallString &append(view_type value) {
    const size_type old_size = size();
    const size_type count = value.size();
    assert(count < std::numeric_limits<size_type>::max() - old_size);

    if (count == 0) {
      return *this;
    }

    const bool overlapping = aliases(value);
    const size_type source_offset =
        overlapping ? static_cast<size_type>(value.data() - data()) : 0;

    m_chars.resize(old_size + count + 1);

    const C *source = overlapping ? data() + source_offset : value.data();
    traits_type::move(data() + old_size, source, count);
    data()[old_size + count] = C{};
    return *this;
  }

  SmallString &append(const C *value) { return append(view_type(value)); }

  SmallString &append(size_type count, C value) {
    const size_type old_size = size();
    assert(count < std::numeric_limits<size_type>::max() - old_size);

    if (count == 0) {
      return *this;
    }

    m_chars.resize(old_size + count + 1);
    traits_type::assign(data() + old_size, count, value);
    data()[old_size + count] = C{};
    return *this;
  }

  template <std::input_iterator InputIt>
  SmallString &append(InputIt first, InputIt last) {
    for (; first != last; ++first) {
      push_back(static_cast<C>(*first));
    }
    return *this;
  }

  /**
   * \brief Appends characters.
   * \code{.cpp}
   * SmallString& operator+=(view_type value);
   * SmallString& operator+=(const C* value);
   * SmallString& operator+=(C value);
   * \endcode
   * \param value Characters or character to append.
   * \return This string.
   */
  SmallString &operator+=(view_type value) { return append(value); }
  SmallString &operator+=(const C *value) { return append(value); }

  SmallString &operator+=(C value) {
    push_back(value);
    return *this;
  }

  /**
   * \brief Inserts characters at a position.
   * \code{.cpp}
   * SmallString& insert(size_type pos, view_type value);
   * SmallString& insert(size_type pos, const C* value);
   * \endcode
   * \param pos Insertion position.
   * \param value Characters to insert.
   * \return This string.
   * \attention 1. \p pos must not exceed size().
   */
  SmallString &insert(size_type pos, view_type value) {
    assert(pos <= size());

    if (aliases(value)) {
      SmallString snapshot(value);
      return insert(pos, snapshot.view());
    }

    const size_type old_size = size();
    const size_type count = value.size();
    assert(count < std::numeric_limits<size_type>::max() - old_size);

    if (count == 0) {
      return *this;
    }

    m_chars.resize(old_size + count + 1);
    traits_type::move(data() + pos + count, data() + pos, old_size - pos + 1);
    traits_type::copy(data() + pos, value.data(), count);
    return *this;
  }

  SmallString &insert(size_type pos, const C *value) {
    return insert(pos, view_type(value));
  }

  /**
   * \brief Erases characters from a position.
   * \code{.cpp}
   * SmallString& erase(size_type pos = 0, size_type count = size_type(-1));
   * \endcode
   * \param pos Erasure position.
   * \param count Maximum number of characters to erase.
   * \return This string.
   * \attention 1. \p pos must not exceed size().
   */
  SmallString &erase(size_type pos = 0, size_type count = size_type(-1)) {
    assert(pos <= size());

    const size_type old_size = size();
    const size_type removed = std::min(count, old_size - pos);

    if (removed != 0) {
      traits_type::move(data() + pos, data() + pos + removed,
                        old_size - pos - removed + 1);
      m_chars.resize(old_size - removed + 1);
    }
    return *this;
  }

  /**
   * \brief Replaces characters at a position.
   * \code{.cpp}
   * SmallString& replace(size_type pos, size_type count, view_type value);
   * \endcode
   * \param pos Replacement position.
   * \param count Maximum number of characters to erase.
   * \param value Replacement characters.
   * \return This string.
   * \attention 1. \p pos must not exceed size().
   */
  SmallString &replace(size_type pos, size_type count, view_type value) {
    assert(pos <= size());

    if (aliases(value)) {
      SmallString snapshot(value);
      return replace(pos, count, snapshot.view());
    }

    const size_type old_size = size();
    const size_type removed = std::min(count, old_size - pos);
    const size_type added = value.size();
    assert(added <
           std::numeric_limits<size_type>::max() - (old_size - removed));

    const size_type new_size = old_size - removed + added;

    if (new_size > old_size) {
      m_chars.resize(new_size + 1);
    }

    if (added != removed) {
      traits_type::move(data() + pos + added, data() + pos + removed,
                        old_size - pos - removed + 1);
    }
    if (added != 0) {
      traits_type::copy(data() + pos, value.data(), added);
    }

    if (new_size < old_size) {
      m_chars.resize(new_size + 1);
    }
    return *this;
  }

  /**
   * \brief Returns a substring.
   * \code{.cpp}
   * SmallString substr(size_type pos = 0, size_type count = size_type(-1)) const;
   * \endcode
   * \param pos Starting position.
   * \param count Maximum substring length.
   * \return The requested string.
   * \attention 1. \p pos must not exceed size().
   */
  [[nodiscard]] SmallString substr(size_type pos = 0,
                                   size_type count = size_type(-1)) const {
    assert(pos <= size());
    return SmallString(view().substr(pos, count));
  }

  /**
   * \brief Compares the string with another view.
   * \code{.cpp}
   * int compare(view_type other) const noexcept;
   * \endcode
   * \param other String to compare with.
   * \return Negative, zero, or positive according to lexical order.
   */
  [[nodiscard]] int compare(view_type other) const noexcept {
    return view().compare(other);
  }

  /**
   * \brief Finds the first occurrence of a value.
   * \code{.cpp}
   * size_type find(view_type value, size_type pos = 0) const noexcept;
   * size_type find(C value, size_type pos = 0) const noexcept;
   * \endcode
   * \param value Value to find.
   * \param pos Search position.
   * \return Position of the match, or npos.
   */
  [[nodiscard]] size_type find(view_type value,
                               size_type pos = 0) const noexcept {
    return view().find(value, pos);
  }

  [[nodiscard]] size_type find(C value, size_type pos = 0) const noexcept {
    return view().find(value, pos);
  }

  /**
   * \brief Finds the last occurrence of a view.
   * \code{.cpp}
   * size_type rfind(view_type value, size_type pos = npos) const noexcept;
   * \endcode
   * \param value View to find.
   * \param pos Search position.
   * \return Position of the match, or npos.
   */
  [[nodiscard]] size_type rfind(view_type value,
                                size_type pos = npos) const noexcept {
    return view().rfind(value, pos);
  }

  /**
   * \brief Checks whether the string starts with a view.
   * \code{.cpp}
   * bool starts_with(view_type value) const noexcept;
   * \endcode
   * \param value Prefix to check.
   * \return Whether the prefix matches.
   */
  [[nodiscard]] bool starts_with(view_type value) const noexcept {
    return view().starts_with(value);
  }

  /**
   * \brief Checks whether the string ends with a view.
   * \code{.cpp}
   * bool ends_with(view_type value) const noexcept;
   * \endcode
   * \param value Suffix to check.
   * \return Whether the suffix matches.
   */
  [[nodiscard]] bool ends_with(view_type value) const noexcept {
    return view().ends_with(value);
  }

  /**
   * \brief Checks whether the string contains a view.
   * \code{.cpp}
   * bool contains(view_type value) const noexcept;
   * \endcode
   * \param value View to find.
   * \return Whether the view occurs.
   */
  [[nodiscard]] bool contains(view_type value) const noexcept {
    return find(value) != npos;
  }

  /**
   * \brief Compares two strings for equality.
   * \code{.cpp}
   * bool operator==(const SmallString& lhs, const SmallString& rhs) noexcept;
   * \endcode
   * \param lhs Left operand.
   * \param rhs Right operand.
   * \return Whether the strings are equal.
   */
  [[nodiscard]] friend bool operator==(const SmallString &lhs,
                                       const SmallString &rhs) noexcept {
    return lhs.view() == rhs.view();
  }

  /**
   * \brief Compares a string with a view for equality.
   * \code{.cpp}
   * bool operator==(const SmallString& lhs, view_type rhs) noexcept;
   * \endcode
   * \param lhs Left operand.
   * \param rhs Right operand.
   * \return Whether the values are equal.
   */
  [[nodiscard]] friend bool operator==(const SmallString &lhs,
                                       view_type rhs) noexcept {
    return lhs.view() == rhs;
  }

  /**
   * \brief Compares two strings lexically.
   * \code{.cpp}
   * auto operator<=>(const SmallString& lhs, const SmallString& rhs) noexcept;
   * \endcode
   * \param lhs Left operand.
   * \param rhs Right operand.
   * \return The lexical ordering.
   */
  [[nodiscard]] friend auto operator<=>(const SmallString &lhs,
                                        const SmallString &rhs) noexcept {
    return lhs.view() <=> rhs.view();
  }

private:
  [[nodiscard]] bool aliases(view_type value) const noexcept {
    if (value.empty()) {
      return false;
    }

    // std::less provides an ordering for pointers into different objects.
    std::less<const C *> less;
    return !less(value.data(), data()) &&
           less(value.data(), data() + m_chars.size());
  }

  SmallVector<C, InlineCapacity + 1, A> m_chars;
};

} // namespace strobe

namespace std {

/**
 * \brief Hashes a small string by its characters.
 * \code{.cpp}
 * template<size_t InlineCapacity, strobe::Allocator A, typename C>
 * struct hash<strobe::SmallString<InlineCapacity, A, C>>;
 * \endcode
 */
template <size_t InlineCapacity, strobe::Allocator A, typename C>
struct hash<strobe::SmallString<InlineCapacity, A, C>> {
  size_t operator()(const strobe::SmallString<InlineCapacity, A, C> &value)
      const noexcept(noexcept(std::hash<C>{}(*value.begin()))) {
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
 * \brief Formats a small string as its character sequence.
 * \code{.cpp}
 * template<size_t InlineCapacity, strobe::Allocator A, typename C>
 * struct formatter<strobe::SmallString<InlineCapacity, A, C>>;
 * \endcode
 */
template <size_t InlineCapacity, strobe::Allocator A, typename C>
struct formatter<strobe::SmallString<InlineCapacity, A, C>> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }

  template <typename FormatContext>
  auto format(const strobe::SmallString<InlineCapacity, A, C> &value,
              FormatContext &ctx) const {
    return fmt::format_to(ctx.out(), "{}", value.view());
  }
};

} // namespace fmt
