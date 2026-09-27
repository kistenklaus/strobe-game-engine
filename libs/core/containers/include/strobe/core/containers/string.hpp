#pragma once

#include "strobe/core/containers/vector.hpp"

#include <algorithm>
#include <cassert>
#include <compare>
#include <cstddef>
#include <functional>
#include <fmt/format.h>
#include <initializer_list>
#include <iterator>
#include <string_view>
#include <type_traits>
#include <utility>

namespace strobe {

/**
 * \ingroup core
 * \brief Dynamically sized null-terminated string.
 * \code{.cpp}
 * template<Allocator A = Mallocator, typename C = char>
 * class String;
 * \endcode
 */
template <Allocator A = Mallocator, typename C = char> class String {
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
   * \brief Constructs, copies, moves, assigns, or destroys a string.
   * \code{.cpp}
   * explicit String(const A& alloc = {});
   * String(view_type value, const A& alloc = {});
   * String(const C* value, const A& alloc = {});
   * String(size_type count, C value, const A& alloc = {});
   * String(const String& other);
   * String(String&& other);
   * String& operator=(const String& other);
   * String& operator=(String&& other);
   * ~String();
   * \endcode
   * \param alloc Allocator to use.
   * \param value Initial string value.
   * \param count Number of repeated characters.
   * \param other String to copy or move from.
   * Moved-from strings are empty.
   */
  explicit String(const A &alloc = {}) : m_chars(alloc) {
    m_chars.push_back(C{});
  }

  String(view_type value, const A &alloc = {}) : String(alloc) {
    append(value);
  }

  String(const C *value, const A &alloc = {})
      : String(view_type(value), alloc) {}

  String(size_type count, C value, const A &alloc = {}) : String(alloc) {
    append(count, value);
  }

  String(const String &) = default;

  String(String &&other) : m_chars(std::move(other.m_chars)) {
    other.m_chars.push_back(C{});
  }

  String &operator=(const String &) = default;

  String &operator=(String &&other) {
    if (this != &other) {
      m_chars = std::move(other.m_chars);
      other.m_chars.push_back(C{});
    }
    return *this;
  }

  /**
   * \brief Assigns string contents.
   * \code{.cpp}
   * String& operator=(view_type value);
   * String& operator=(const C* value);
   * \endcode
   * \param value Source string.
   * \return This string.
   */
  String &operator=(view_type value) { return assign(value); }

  String &operator=(const C *value) { return assign(view_type(value)); }

  /**
   * \brief Returns the number of characters.
   * \code{.cpp}
   * size_type size() const noexcept;
   * \endcode
   * \return Number of characters.
   */
  [[nodiscard]] size_type size() const noexcept { return m_chars.size() - 1; }
  /**
   * \brief Returns the number of characters.
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
   * \brief Reserves character capacity.
   * \code{.cpp}
   * void reserve(size_type count);
   * \endcode
   * \param count Desired capacity.
   */
  void reserve(size_type count) {
    assert(count < Vector<C, A>::max_size());
    m_chars.reserve(count + 1);
  }

  /**
   * \brief Returns character storage.
   * \code{.cpp}
   * pointer data() noexcept;
   * const_pointer data() const noexcept;
   * \endcode
   * \return Pointer to the first character.
   */
  [[nodiscard]] pointer data() noexcept { return m_chars.data(); }
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
    assert(old_size < Vector<C, A>::max_size() - 1);

    m_chars.resize(old_size + 2);
    data()[old_size] = value;
    data()[old_size + 1] = C{};
  }

  /**
   * \brief Removes the last character.
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
   * \brief Changes the string size.
   * \code{.cpp}
   * void resize(size_type count, C value = C{});
   * \endcode
   * \param count New size.
   * \param value Character used to initialize appended positions.
   */
  void resize(size_type count, C value = C{}) {
    assert(count < Vector<C, A>::max_size());

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
   * String& assign(view_type value);
   * \endcode
   * \param value New contents.
   * \return This string.
   */
  String &assign(view_type value) {
    const size_type count = value.size();
    assert(count < Vector<C, A>::max_size());

    if (aliases(value)) {
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
   * String& assign(view_type value, size_type pos, size_type count = npos);
   * \endcode
   * \param value Source view.
   * \param pos Starting position.
   * \param count Maximum number of characters.
   * \return This string.
   */
  String &assign(view_type value, size_type pos, size_type count = npos) {
    return assign(value.substr(pos, count));
  }

  /**
   * \brief Replaces the contents from a character pointer.
   * \code{.cpp}
   * String& assign(const C* value, size_type count);
   * \endcode
   * \param value Source characters.
   * \param count Number of characters.
   * \return This string.
   */
  String &assign(const C *value, size_type count) {
    return assign(view_type(value, count));
  }

  /**
   * \brief Replaces the contents from an initializer list.
   * \code{.cpp}
   * String& assign(std::initializer_list<C> value);
   * \endcode
   * \param value Characters to copy.
   * \return This string.
   */
  String &assign(std::initializer_list<C> value) {
    return assign(value.begin(), value.end());
  }

  /**
   * \brief Replaces the contents with repeated characters.
   * \code{.cpp}
   * String& assign(size_type count, C value);
   * \endcode
   * \param count Number of characters.
   * \param value Character to repeat.
   * \return This string.
   */
  String &assign(size_type count, C value) { resize(count, value); return *this; }

  /**
   * \brief Replaces the contents from an iterator range.
   * \code{.cpp}
   * template<class InputIt> String& assign(InputIt first, InputIt last);
   * \endcode
   * \param first Range beginning.
   * \param last Range end.
   * \return This string.
   */
  template <class InputIt> String &assign(InputIt first, InputIt last) {
    clear();
    for (; first != last; ++first) push_back(static_cast<C>(*first));
    return *this;
  }

  /**
   * \brief Appends characters.
   * \code{.cpp}
   * String& append(view_type value);
   * String& append(const C* value);
   * String& append(size_type count, C value);
   * template<std::input_iterator InputIt>
   * String& append(InputIt first, InputIt last);
   * \endcode
   * \param value Characters to append.
   * \param count Number of repeated characters.
   * \param first Range beginning.
   * \param last Range end.
   * \return This string.
   */
  String &append(view_type value) {
    const size_type old_size = size();
    const size_type count = value.size();
    assert(count < Vector<C, A>::max_size() - old_size);

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

  String &append(const C *value) { return append(view_type(value)); }

  String &append(size_type count, C value) {
    const size_type old_size = size();
    assert(count < Vector<C, A>::max_size() - old_size);

    if (count != 0) {
      m_chars.resize(old_size + count + 1);
      traits_type::assign(data() + old_size, count, value);
      data()[old_size + count] = C{};
    }
    return *this;
  }

  template <std::input_iterator InputIt>
  String &append(InputIt first, InputIt last) {
    for (; first != last; ++first) {
      push_back(static_cast<C>(*first));
    }
    return *this;
  }

  /**
   * \brief Appends characters.
   * \code{.cpp}
   * String& operator+=(view_type value);
   * String& operator+=(const C* value);
   * String& operator+=(C value);
   * \endcode
   * \param value Characters to append.
   * \return This string.
   */
  String &operator+=(view_type value) { return append(value); }
  String &operator+=(const C *value) { return append(value); }
  String &operator+=(C value) {
    push_back(value);
    return *this;
  }

  /**
   * \brief Inserts characters.
   * \code{.cpp}
   * String& insert(size_type pos, view_type value);
   * String& insert(size_type pos, const C* value);
   * \endcode
   * \param pos Insertion position.
   * \param value Characters to insert.
   * \return This string.
   */
  String &insert(size_type pos, view_type value) {
    assert(pos <= size());

    if (aliases(value)) {
      String snapshot(value);
      return insert_external(pos, snapshot.view());
    }

    return insert_external(pos, value);
  }

  String &insert(size_type pos, const C *value) {
    return insert(pos, view_type(value));
  }

  /**
   * \brief Erases characters.
   * \code{.cpp}
   * String& erase(size_type pos = 0, size_type count = npos);
   * \endcode
   * \param pos First position.
   * \param count Maximum number of characters.
   * \return This string.
   */
  String &erase(size_type pos = 0, size_type count = npos) {
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
   * \brief Replaces characters.
   * \code{.cpp}
   * String& replace(size_type pos, size_type count, view_type value);
   * \endcode
   * \param pos First position.
   * \param count Number of characters to remove.
   * \param value Replacement characters.
   * \return This string.
   */
  String &replace(size_type pos, size_type count, view_type value) {
    assert(pos <= size());
    const size_type removed = std::min(count, size() - pos);

    if (aliases(value)) {
      String snapshot(value);
      return replace_external(pos, removed, snapshot.view());
    }

    return replace_external(pos, removed, value);
  }

  /**
   * \brief Returns a substring.
   * \code{.cpp}
   * String substr(size_type pos = 0, size_type count = npos) const;
   * \endcode
   * \param pos First position.
   * \param count Maximum number of characters.
   * \return The substring.
   */
  [[nodiscard]] String substr(size_type pos = 0, size_type count = npos) const {
    assert(pos <= size());
    return String(view().substr(pos, count));
  }

  /**
   * \brief Compares the string with another view.
   * \code{.cpp}
   * int compare(view_type other) const noexcept;
   * \endcode
   * \param other View to compare.
   * \return Lexical comparison result.
   */
  [[nodiscard]] int compare(view_type other) const noexcept {
    return view().compare(other);
  }

  /**
   * \brief Finds a value.
   * \code{.cpp}
   * size_type find(view_type value, size_type pos = 0) const noexcept;
   * size_type find(C value, size_type pos = 0) const noexcept;
   * \endcode
   * \param value Value to find.
   * \param pos Starting position.
   * \return Position, or npos.
   */
  [[nodiscard]] size_type find(view_type value,
                               size_type pos = 0) const noexcept {
    return view().find(value, pos);
  }

  [[nodiscard]] size_type find(C value, size_type pos = 0) const noexcept {
    return view().find(value, pos);
  }

  /**
   * \brief Finds the last occurrence of a value.
   * \code{.cpp}
   * size_type rfind(view_type value, size_type pos = npos) const noexcept;
   * \endcode
   * \param value Value to find.
   * \param pos Starting position.
   * \return Position, or npos.
   */
  [[nodiscard]] size_type rfind(view_type value,
                                size_type pos = npos) const noexcept {
    return view().rfind(value, pos);
  }

  /**
   * \brief Checks a prefix.
   * \code{.cpp}
   * bool starts_with(view_type value) const noexcept;
   * \endcode
   * \param value Prefix to test.
   * \return Whether it matches.
   */
  [[nodiscard]] bool starts_with(view_type value) const noexcept {
    return view().starts_with(value);
  }

  /**
   * \brief Checks a suffix.
   * \code{.cpp}
   * bool ends_with(view_type value) const noexcept;
   * \endcode
   * \param value Suffix to test.
   * \return Whether it matches.
   */
  [[nodiscard]] bool ends_with(view_type value) const noexcept {
    return view().ends_with(value);
  }

  /**
   * \brief Checks whether a value occurs.
   * \code{.cpp}
   * bool contains(view_type value) const noexcept;
   * \endcode
   * \param value Value to find.
   * \return Whether it occurs.
   */
  [[nodiscard]] bool contains(view_type value) const noexcept {
    return find(value) != npos;
  }

  /**
   * \brief Compares two strings for equality.
   * \code{.cpp}
   * friend bool operator==(const String& lhs, const String& rhs) noexcept;
   * \endcode
   * \param lhs Left operand.
   * \param rhs Right operand.
   * \return Whether the strings are equal.
   */
  [[nodiscard]] friend bool operator==(const String &lhs,
                                       const String &rhs) noexcept {
    return lhs.view() == rhs.view();
  }

  /**
   * \brief Compares a string with a view for equality.
   * \code{.cpp}
   * friend bool operator==(const String& lhs, view_type rhs) noexcept;
   * \endcode
   * \param lhs Left operand.
   * \param rhs Right operand.
   * \return Whether the values are equal.
   */
  [[nodiscard]] friend bool operator==(const String &lhs,
                                       view_type rhs) noexcept {
    return lhs.view() == rhs;
  }

  /**
   * \brief Compares two strings lexically.
   * \code{.cpp}
   * friend auto operator<=>(const String& lhs, const String& rhs) noexcept;
   * \endcode
   * \param lhs Left operand.
   * \param rhs Right operand.
   * \return Lexical ordering.
   */
  [[nodiscard]] friend auto operator<=>(const String &lhs,
                                        const String &rhs) noexcept {
    return lhs.view() <=> rhs.view();
  }

private:
  [[nodiscard]] bool aliases(view_type value) const noexcept {
    if (value.empty()) {
      return false;
    }

    std::less<const C *> less;
    return !less(value.data(), data()) &&
           less(value.data(), data() + m_chars.size());
  }

  String &insert_external(size_type pos, view_type value) {
    const size_type old_size = size();
    const size_type count = value.size();
    assert(count < Vector<C, A>::max_size() - old_size);

    if (count == 0) {
      return *this;
    }

    m_chars.resize(old_size + count + 1);
    traits_type::move(data() + pos + count, data() + pos,
                      old_size - pos + 1); // Includes terminator.
    traits_type::copy(data() + pos, value.data(), count);
    return *this;
  }

  String &replace_external(size_type pos, size_type removed, view_type value) {
    const size_type old_size = size();
    const size_type added = value.size();
    assert(added < Vector<C, A>::max_size() - (old_size - removed));

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

  Vector<C, A> m_chars;
};

} // namespace strobe

namespace std {

/**
 * \brief Hashes a dynamically sized string by its characters.
 * \code{.cpp}
 * template<strobe::Allocator A, typename C>
 * struct hash<strobe::String<A, C>>;
 * \endcode
 */
template <strobe::Allocator A, typename C>
struct hash<strobe::String<A, C>> {
  size_t operator()(const strobe::String<A, C> &value) const
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
 * \brief Formats a dynamically sized string as its character sequence.
 * \code{.cpp}
 * template<strobe::Allocator A, typename C>
 * struct formatter<strobe::String<A, C>>;
 * \endcode
 */
template <strobe::Allocator A, typename C>
struct formatter<strobe::String<A, C>> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }

  template <typename FormatContext>
  auto format(const strobe::String<A, C> &value, FormatContext &ctx) const {
    return fmt::format_to(ctx.out(), "{}", value.view());
  }
};

} // namespace fmt
