#pragma once

#include <cstddef>
#include <string_view>

namespace strobe {

template <std::size_t N> struct fixed_string {
  static_assert(N > 0);

  char value[N]{};

  consteval fixed_string() noexcept = default;

  consteval fixed_string(const char (&str)[N]) noexcept {
    for (std::size_t i = 0; i < N; ++i) {
      value[i] = str[i];
    }
  }

  [[nodiscard]]
  constexpr const char *data() const noexcept {
    return value;
  }

  [[nodiscard]]
  constexpr std::size_t size() const noexcept {
    return N - 1;
  }

  [[nodiscard]]
  constexpr bool empty() const noexcept {
    return size() == 0;
  }

  [[nodiscard]]
  constexpr operator std::string_view() const noexcept {
    return {value, size()};
  }
};

template <std::size_t N> fixed_string(const char (&)[N]) -> fixed_string<N>;

template <std::size_t LhsSize, std::size_t RhsSize>
[[nodiscard]]
consteval auto operator+(const fixed_string<LhsSize> &lhs,
                         const fixed_string<RhsSize> &rhs) noexcept {
  // Each operand contains one null terminator. The left-hand
  // terminator is replaced by the first character of rhs.
  char result[LhsSize + RhsSize - 1]{};

  for (std::size_t i = 0; i < LhsSize - 1; ++i) {
    result[i] = lhs.value[i];
  }

  for (std::size_t i = 0; i < RhsSize; ++i) {
    result[LhsSize - 1 + i] = rhs.value[i];
  }

  return fixed_string{result};
}

template <std::size_t L, std::size_t R>
[[nodiscard]]
constexpr bool operator==(const fixed_string<L> &lhs,
                          const fixed_string<R> &rhs) noexcept {
  if constexpr (L != R) {
    return false;
  } else {
    for (std::size_t i = 0; i < L; ++i) {
      if (lhs.value[i] != rhs.value[i]) {
        return false;
      }
    }

    return true;
  }
}

template <std::size_t N>
[[nodiscard]]
consteval auto fixed_string_from_view(std::string_view view) noexcept {
  fixed_string<N + 1> result{};

  for (std::size_t i = 0; i < N; ++i) {
    result.value[i] = view[i];
  }

  result.value[N] = '\0';

  return result;
}

} // namespace strobe
