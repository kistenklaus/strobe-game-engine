#pragma once

#include <array>
#include <cassert>
#include <compare>
#include <cstddef>
#include <fmt/format.h>
#include <functional>
#include <iterator>
#include <string_view>

namespace strobe {

/**
 * \ingroup core
 * \brief Fixed-size, null-terminated string literal wrapper.
 * \code{.cpp}
 * template<size_t N, typename C = char>
 * class Str;
 * \endcode
 */
template <size_t N, typename C = char> class Str {
public:
  using value_type = C;
  using traits_type = std::char_traits<C>;
  using size_type = size_t;
  using difference_type = std::ptrdiff_t;
  using const_reference = const C &;
  using const_pointer = const C *;
  using const_iterator = const C *;
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
   * \brief Constructs a fixed-size string.
   * \code{.cpp}
   * constexpr Str() noexcept;
   * constexpr Str(const C (&literal)[N + 1]) noexcept;
   * explicit constexpr Str(view_type value) noexcept;
   * \endcode
   * \param literal Null-terminated character array.
   * \param value View containing exactly N characters.
   * \attention 1. \p value must contain exactly N characters.
   */
  constexpr Str() noexcept = default;

  constexpr Str(const C (&literal)[N + 1]) noexcept {
    traits_type::copy(m_data.data(), literal, N);
    m_data[N] = C{};
  }

  explicit constexpr Str(view_type value) noexcept {
    assert(value.size() == N);
    if constexpr (N != 0) {
      traits_type::copy(m_data.data(), value.data(), N);
    }
    m_data[N] = C{};
  }

  /**
   * \brief Returns the number of characters.
   * \code{.cpp}
   * static constexpr size_type size() noexcept;
   * \endcode
   * \return N.
   */
  [[nodiscard]] static constexpr size_type size() noexcept { return N; }
  /**
   * \brief Returns the number of characters.
   * \code{.cpp}
   * static constexpr size_type length() noexcept;
   * \endcode
   * \return N.
   */
  [[nodiscard]] static constexpr size_type length() noexcept { return N; }
  /**
   * \brief Checks whether the string is empty.
   * \code{.cpp}
   * static constexpr bool empty() noexcept;
   * \endcode
   * \return Whether N is zero.
   */
  [[nodiscard]] static constexpr bool empty() noexcept { return N == 0; }

  /**
   * \brief Returns the character storage.
   * \code{.cpp}
   * constexpr const_pointer data() const noexcept;
   * \endcode
   * \return Pointer to the first character.
   */
  [[nodiscard]] constexpr const_pointer data() const noexcept {
    return m_data.data();
  }

  /**
   * \brief Returns the null-terminated character storage.
   * \code{.cpp}
   * constexpr const_pointer c_str() const noexcept;
   * \endcode
   * \return Pointer to the first character.
   */
  [[nodiscard]] constexpr const_pointer c_str() const noexcept {
    return data();
  }

  /**
   * \brief Returns an iterator to the first character.
   * \code{.cpp}
   * constexpr const_iterator begin() const noexcept;
   * \endcode
   * \return Iterator to the first character.
   */
  [[nodiscard]] constexpr const_iterator begin() const noexcept {
    return data();
  }

  /**
   * \brief Returns an iterator past the last character.
   * \code{.cpp}
   * constexpr const_iterator end() const noexcept;
   * \endcode
   * \return Iterator past the last character.
   */
  [[nodiscard]] constexpr const_iterator end() const noexcept {
    return data() + N;
  }

  /**
   * \brief Returns a const iterator to the first character.
   * \code{.cpp}
   * constexpr const_iterator cbegin() const noexcept;
   * \endcode
   * \return Const iterator to the first character.
   */
  [[nodiscard]] constexpr const_iterator cbegin() const noexcept {
    return begin();
  }

  /**
   * \brief Returns a const iterator past the last character.
   * \code{.cpp}
   * constexpr const_iterator cend() const noexcept;
   * \endcode
   * \return Const iterator past the last character.
   */
  [[nodiscard]] constexpr const_iterator cend() const noexcept {
    return end();
  }

  /**
   * \brief Returns a reverse iterator to the last character.
   * \code{.cpp}
   * constexpr const_reverse_iterator rbegin() const noexcept;
   * \endcode
   * \return Reverse iterator to the last character.
   */
  [[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept {
    return const_reverse_iterator(end());
  }

  /**
   * \brief Returns a reverse iterator before the first character.
   * \code{.cpp}
   * constexpr const_reverse_iterator rend() const noexcept;
   * \endcode
   * \return Reverse iterator before the first character.
   */
  [[nodiscard]] constexpr const_reverse_iterator rend() const noexcept {
    return const_reverse_iterator(begin());
  }

  /**
   * \brief Accesses a character.
   * \code{.cpp}
   * constexpr const_reference operator[](size_type index) const noexcept;
   * \endcode
   * \param index Character index.
   * \return Reference to the character.
   * \attention 1. \p index must not exceed size().
   */
  [[nodiscard]] constexpr const_reference
  operator[](size_type index) const noexcept {
    assert(index <= N);
    return m_data[index];
  }

  /**
   * \brief Returns the first character.
   * \code{.cpp}
   * constexpr const_reference front() const noexcept;
   * \endcode
   * \return Reference to the first character.
   * \attention 1. N must not be zero.
   */
  [[nodiscard]] constexpr const_reference front() const noexcept {
    static_assert(N != 0);
    return m_data[0];
  }

  /**
   * \brief Returns the last character.
   * \code{.cpp}
   * constexpr const_reference back() const noexcept;
   * \endcode
   * \return Reference to the last character.
   * \attention 1. N must not be zero.
   */
  [[nodiscard]] constexpr const_reference back() const noexcept {
    static_assert(N != 0);
    return m_data[N - 1];
  }

  /**
   * \brief Returns a string view.
   * \code{.cpp}
   * constexpr view_type view() const noexcept;
   * \endcode
   * \return View of the characters.
   */
  [[nodiscard]] constexpr view_type view() const noexcept {
    return {data(), N};
  }

  /**
   * \brief Converts the string to a view.
   * \code{.cpp}
   * constexpr operator view_type() const noexcept;
   * \endcode
   * \return View of the characters.
   */
  [[nodiscard]] constexpr operator view_type() const noexcept {
    return view();
  }

  /**
   * \brief Compares the string with another view.
   * \code{.cpp}
   * constexpr int compare(view_type other) const noexcept;
   * \endcode
   * \param other View to compare.
   * \return Negative, zero, or positive according to lexical order.
   */
  [[nodiscard]] constexpr int compare(view_type other) const noexcept {
    return view().compare(other);
  }

  /**
   * \brief Finds a value.
   * \code{.cpp}
   * constexpr size_type find(view_type value, size_type pos = 0) const noexcept;
   * constexpr size_type find(C value, size_type pos = 0) const noexcept;
   * \endcode
   * \param value Value to find.
   * \param pos Position at which to begin searching.
   * \return Position of the value, or npos.
   */
  [[nodiscard]] constexpr size_type
  find(view_type value, size_type pos = 0) const noexcept {
    return view().find(value, pos);
  }

  [[nodiscard]] constexpr size_type
  find(C value, size_type pos = 0) const noexcept {
    return view().find(value, pos);
  }

  /**
   * \brief Finds the last occurrence of a value.
   * \code{.cpp}
   * constexpr size_type rfind(view_type value,
   *                             size_type pos = npos) const noexcept;
   * \endcode
   * \param value Value to find.
   * \param pos Position at which to begin searching backward.
   * \return Position of the value, or npos.
   */
  [[nodiscard]] constexpr size_type
  rfind(view_type value, size_type pos = npos) const noexcept {
    return view().rfind(value, pos);
  }

  /**
   * \brief Checks whether the string starts with a value.
   * \code{.cpp}
   * constexpr bool starts_with(view_type value) const noexcept;
   * \endcode
   * \param value Prefix to test.
   * \return Whether the prefix matches.
   */
  [[nodiscard]] constexpr bool starts_with(view_type value) const noexcept {
    return view().starts_with(value);
  }

  /**
   * \brief Checks whether the string ends with a value.
   * \code{.cpp}
   * constexpr bool ends_with(view_type value) const noexcept;
   * \endcode
   * \param value Suffix to test.
   * \return Whether the suffix matches.
   */
  [[nodiscard]] constexpr bool ends_with(view_type value) const noexcept {
    return view().ends_with(value);
  }

  /**
   * \brief Checks whether the string contains a value.
   * \code{.cpp}
   * constexpr bool contains(view_type value) const noexcept;
   * \endcode
   * \param value Value to find.
   * \return Whether the value occurs.
   */
  [[nodiscard]] constexpr bool contains(view_type value) const noexcept {
    return view().find(value) != npos;
  }

  /**
   * \brief Returns a substring view.
   * \code{.cpp}
   * constexpr view_type substr(size_type pos = 0,
   *                            size_type count = npos) const noexcept;
   * \endcode
   * \param pos First character position.
   * \param count Maximum number of characters.
   * \return Requested substring.
   * \attention 1. \p pos must not exceed size().
   */
  [[nodiscard]] constexpr view_type
  substr(size_type pos = 0, size_type count = npos) const noexcept {
    assert(pos <= N);
    return view().substr(pos, count);
  }

private:
  std::array<C, N + 1> m_data{};
};

template <typename C, size_t M>
Str(const C (&)[M]) -> Str<M - 1, C>;

/**
 * \brief Compares two fixed-size strings for equality.
 * \code{.cpp}
 * template<size_t N, size_t M, typename C>
 * constexpr bool operator==(const Str<N, C>& lhs,
 *                           const Str<M, C>& rhs) noexcept;
 * \endcode
 * \param lhs Left operand.
 * \param rhs Right operand.
 * \return Whether the strings are equal.
 */
template <size_t N, size_t M, typename C>
[[nodiscard]] constexpr bool operator==(const Str<N, C> &lhs,
                                        const Str<M, C> &rhs) noexcept {
  return lhs.view() == rhs.view();
}

/**
 * \brief Compares two fixed-size strings lexically.
 * \code{.cpp}
 * template<size_t N, size_t M, typename C>
 * constexpr auto operator<=>(const Str<N, C>& lhs,
 *                            const Str<M, C>& rhs) noexcept;
 * \endcode
 * \param lhs Left operand.
 * \param rhs Right operand.
 * \return The lexical ordering.
 */
template <size_t N, size_t M, typename C>
[[nodiscard]] constexpr auto operator<=>(const Str<N, C> &lhs,
                                         const Str<M, C> &rhs) noexcept {
  return lhs.view() <=> rhs.view();
}

} // namespace strobe

namespace std {

/**
 * \brief Hashes a fixed-size string by its characters.
 * \code{.cpp}
 * template<size_t N, typename C>
 * struct hash<strobe::Str<N, C>>;
 * \endcode
 */
template <size_t N, typename C>
struct hash<strobe::Str<N, C>> {
  size_t operator()(const strobe::Str<N, C> &value) const
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
 * \brief Formats a fixed-size string as its character sequence.
 * \code{.cpp}
 * template<size_t N, typename C>
 * struct formatter<strobe::Str<N, C>>;
 * \endcode
 */
template <size_t N, typename C>
struct formatter<strobe::Str<N, C>> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }

  template <typename FormatContext>
  auto format(const strobe::Str<N, C> &value, FormatContext &ctx) const {
    return fmt::format_to(ctx.out(), "{}", value.view());
  }
};

} // namespace fmt
