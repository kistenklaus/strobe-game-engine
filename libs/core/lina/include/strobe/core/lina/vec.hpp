#pragma once

#include "strobe/core/containers/bitset.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <concepts>
#include <ranges>
#include <tuple>
#include <type_traits>
#include <utility>

namespace strobe {

namespace detail {

// Core vector type definition
template <uint32_t N, typename T>
  requires std::is_arithmetic_v<T>
struct vec {
  // Data storage (trivially copyable and movable)
  std::array<T, N> data = {};

  // Default constructor (trivial)
  constexpr vec() noexcept = default;

  // Uniform value constructor (trivial)
  constexpr explicit vec(T value) noexcept { data.fill(value); }

  // Component-wise constructor for N = 2, 3, 4 (trivial)
  constexpr vec(T x, T y) noexcept
    requires(N >= 2)
      : data{x, y} {}
  constexpr vec(T x, T y, T z) noexcept
    requires(N >= 3)
      : data{x, y, z} {}
  constexpr vec(T x, T y, T z, T w) noexcept
    requires(N >= 4)
      : data{x, y, z, w} {}

  template <typename U>
    requires(std::is_arithmetic_v<U> && std::is_convertible_v<U, T>)
  constexpr vec(const vec<N, U> &other) noexcept {
    for (uint32_t i = 0; i < N; ++i) {
      data[i] = static_cast<T>(other[i]);
    }
  }

  // Array subscript access
  constexpr T &operator[](uint32_t index) noexcept {
    assert(index < N);
    return data[index];
  }

  constexpr const T &operator[](uint32_t index) const noexcept {
    assert(index < N);
    return data[index];
  }

  // Named component access for x, y, z, w (getter functions)
  constexpr T &x() noexcept
    requires(N >= 1)
  {
    return data[0];
  }
  constexpr const T &x() const noexcept
    requires(N >= 1)
  {
    return data[0];
  }

  constexpr T &y() noexcept
    requires(N >= 2)
  {
    return data[1];
  }
  constexpr const T &y() const noexcept
    requires(N >= 2)
  {
    return data[1];
  }

  constexpr T &z() noexcept
    requires(N >= 3)
  {
    return data[2];
  }

  constexpr const T &z() const noexcept
    requires(N >= 3)
  {
    return data[2];
  }

  constexpr T &w() noexcept
    requires(N >= 4)
  {
    return data[3];
  }
  constexpr const T &w() const noexcept
    requires(N >= 4)
  {
    return data[3];
  }

  // Alternative names for color access
  constexpr T &r() noexcept
    requires(N >= 1)
  {
    return data[0];
  }
  constexpr const T &r() const noexcept
    requires(N >= 1)
  {
    return data[0];
  }

  constexpr T &g() noexcept
    requires(N >= 2)
  {
    return data[1];
  }
  constexpr const T &g() const noexcept
    requires(N >= 2)
  {
    return data[1];
  }

  constexpr T &b() noexcept
    requires(N >= 3)
  {
    return data[2];
  }
  constexpr const T &b() const noexcept
    requires(N >= 3)
  {
    return data[2];
  }

  constexpr T &a() noexcept
    requires(N >= 4)
  {
    return data[3];
  }
  constexpr const T &a() const noexcept
    requires(N >= 4)
  {
    return data[3];
  }

  // Coordinate swizzles: two components.
  constexpr vec<2, T> xy() const noexcept
    requires(N >= 2)
  {
    return {data[0], data[1]};
  }

  constexpr vec<2, T> xz() const noexcept
    requires(N >= 3)
  {
    return {data[0], data[2]};
  }

  constexpr vec<2, T> xw() const noexcept
    requires(N >= 4)
  {
    return {data[0], data[3]};
  }

  constexpr vec<2, T> yz() const noexcept
    requires(N >= 3)
  {
    return {data[1], data[2]};
  }

  constexpr vec<2, T> yw() const noexcept
    requires(N >= 4)
  {
    return {data[1], data[3]};
  }

  constexpr vec<2, T> zw() const noexcept
    requires(N >= 4)
  {
    return {data[2], data[3]};
  }

  // Coordinate swizzles: three and four components.

  constexpr vec<3, T> xyz() const noexcept
    requires(N >= 3)
  {
    return {data[0], data[1], data[2]};
  }

  constexpr vec<3, T> xyw() const noexcept
    requires(N >= 4)
  {
    return {data[0], data[1], data[3]};
  }

  constexpr vec<3, T> xzw() const noexcept
    requires(N >= 4)
  {
    return {data[0], data[2], data[3]};
  }

  constexpr vec<3, T> yzw() const noexcept
    requires(N >= 4)
  {
    return {data[1], data[2], data[3]};
  }

  constexpr vec<4, T> xyzw() const noexcept
    requires(N >= 4)
  {
    return {data[0], data[1], data[2], data[3]};
  }

  // Color swizzles: two components.

  constexpr vec<2, T> rg() const noexcept
    requires(N >= 2)
  {
    return {data[0], data[1]};
  }

  constexpr vec<2, T> rb() const noexcept
    requires(N >= 3)
  {
    return {data[0], data[2]};
  }

  constexpr vec<2, T> ra() const noexcept
    requires(N >= 4)
  {
    return {data[0], data[3]};
  }

  constexpr vec<2, T> gb() const noexcept
    requires(N >= 3)
  {
    return {data[1], data[2]};
  }

  constexpr vec<2, T> ga() const noexcept
    requires(N >= 4)
  {
    return {data[1], data[3]};
  }

  constexpr vec<2, T> ba() const noexcept
    requires(N >= 4)
  {
    return {data[2], data[3]};
  }

  // Color swizzles: three and four components.

  constexpr vec<3, T> rgb() const noexcept
    requires(N >= 3)
  {
    return {data[0], data[1], data[2]};
  }

  constexpr vec<3, T> rga() const noexcept
    requires(N >= 4)
  {
    return {data[0], data[1], data[3]};
  }

  constexpr vec<3, T> rba() const noexcept
    requires(N >= 4)
  {
    return {data[0], data[2], data[3]};
  }

  constexpr vec<3, T> gba() const noexcept
    requires(N >= 4)
  {
    return {data[1], data[2], data[3]};
  }

  constexpr vec<4, T> rgba() const noexcept
    requires(N >= 4)
  {
    return {data[0], data[1], data[2], data[3]};
  }

  constexpr bool operator==(const vec &o) const noexcept {
    return std::ranges::equal(this->data, o.data);
  }
  constexpr bool operator!=(const vec &o) const noexcept { return !(*this == o); }

  [[nodiscard]] constexpr T length2() const noexcept {
    T result{};
    for (const T value : data) {
      result += value * value;
    }
    return result;
  }

  [[nodiscard]] T length() const noexcept
    requires std::floating_point<T>
  {
    return std::sqrt(length2());
  }

  // Unary arithmetic.
  [[nodiscard]] constexpr vec operator+() const noexcept { return *this; }

  [[nodiscard]] constexpr vec operator-() const noexcept {
    vec result;
    for (uint32_t i = 0; i < N; ++i) {
      result[i] = -data[i];
    }
    return result;
  }

  // Component-wise vector arithmetic.
  [[nodiscard]] constexpr vec operator+(const vec &rhs) const noexcept {
    vec result;
    for (uint32_t i = 0; i < N; ++i) {
      result[i] = data[i] + rhs[i];
    }
    return result;
  }

  [[nodiscard]] constexpr vec operator-(const vec &rhs) const noexcept {
    vec result;
    for (uint32_t i = 0; i < N; ++i) {
      result[i] = data[i] - rhs[i];
    }
    return result;
  }

  [[nodiscard]] constexpr vec operator*(const vec &rhs) const noexcept {
    vec result;
    for (uint32_t i = 0; i < N; ++i) {
      result[i] = data[i] * rhs[i];
    }
    return result;
  }

  [[nodiscard]] constexpr vec operator/(const vec &rhs) const noexcept {
    vec result;
    for (uint32_t i = 0; i < N; ++i) {
      result[i] = data[i] / rhs[i];
    }
    return result;
  }

  // Compound vector arithmetic.
  constexpr vec &operator+=(const vec &rhs) noexcept {
    for (uint32_t i = 0; i < N; ++i) {
      data[i] += rhs[i];
    }
    return *this;
  }

  constexpr vec &operator-=(const vec &rhs) noexcept {
    for (uint32_t i = 0; i < N; ++i) {
      data[i] -= rhs[i];
    }
    return *this;
  }

  constexpr vec &operator*=(const vec &rhs) noexcept {
    for (uint32_t i = 0; i < N; ++i) {
      data[i] *= rhs[i];
    }
    return *this;
  }

  constexpr vec &operator/=(const vec &rhs) noexcept {
    for (uint32_t i = 0; i < N; ++i) {
      data[i] /= rhs[i];
    }
    return *this;
  }

  // Compound scalar arithmetic.
  constexpr vec &operator+=(T rhs) noexcept {
    for (T &x : data) {
      x += rhs;
    }
    return *this;
  }

  constexpr vec &operator-=(T rhs) noexcept {
    for (T &x : data) {
      x -= rhs;
    }
    return *this;
  }

  constexpr vec &operator*=(T rhs) noexcept {
    for (T &x : data) {
      x *= rhs;
    }
    return *this;
  }

  constexpr vec &operator/=(T rhs) noexcept {
    for (T &x : data) {
      x /= rhs;
    }
    return *this;
  }
};

template <uint32_t N, typename T>
[[nodiscard]] constexpr Bitset<N>
operator<(const vec<N, T> &lhs, const vec<N, T> &rhs) noexcept {
  Bitset<N> result;
  for (uint32_t i = 0; i < N; ++i) {
    if (lhs[i] < rhs[i]) {
      result.set(i);
    }
  }
  return result;
}

template <uint32_t N, typename T>
[[nodiscard]] constexpr Bitset<N>
operator<=(const vec<N, T> &lhs, const vec<N, T> &rhs) noexcept {
  Bitset<N> result;
  for (uint32_t i = 0; i < N; ++i) {
    if (lhs[i] <= rhs[i]) {
      result.set(i);
    }
  }
  return result;
}

template <uint32_t N, typename T>
[[nodiscard]] constexpr Bitset<N>
operator>(const vec<N, T> &lhs, const vec<N, T> &rhs) noexcept {
  Bitset<N> result;
  for (uint32_t i = 0; i < N; ++i) {
    if (lhs[i] > rhs[i]) {
      result.set(i);
    }
  }
  return result;
}

template <uint32_t N, typename T>
[[nodiscard]] constexpr Bitset<N>
operator>=(const vec<N, T> &lhs, const vec<N, T> &rhs) noexcept {
  Bitset<N> result;
  for (uint32_t i = 0; i < N; ++i) {
    if (lhs[i] >= rhs[i]) {
      result.set(i);
    }
  }
  return result;
}

template <uint32_t N, typename T>
[[nodiscard]] constexpr vec<N, T> operator+(const vec<N, T> &lhs,
                                            std::type_identity_t<T> rhs) noexcept {
  vec<N, T> result;
  for (uint32_t i = 0; i < N; ++i) {
    result[i] = lhs[i] + rhs;
  }
  return result;
}

template <uint32_t N, typename T>
[[nodiscard]] constexpr vec<N, T> operator-(const vec<N, T> &lhs,
                                            std::type_identity_t<T> rhs) noexcept {
  vec<N, T> result;
  for (uint32_t i = 0; i < N; ++i) {
    result[i] = lhs[i] - rhs;
  }
  return result;
}

template <uint32_t N, typename T>
[[nodiscard]] constexpr vec<N, T> operator*(const vec<N, T> &lhs,
                                            std::type_identity_t<T> rhs) noexcept {
  vec<N, T> result;
  for (uint32_t i = 0; i < N; ++i) {
    result[i] = lhs[i] * rhs;
  }
  return result;
}

template <uint32_t N, typename T>
[[nodiscard]] constexpr vec<N, T> operator/(const vec<N, T> &lhs,
                                            std::type_identity_t<T> rhs) noexcept {
  vec<N, T> result;
  for (uint32_t i = 0; i < N; ++i) {
    result[i] = lhs[i] / rhs;
  }
  return result;
}

template <uint32_t N, typename T>
[[nodiscard]] constexpr vec<N, T> operator+(std::type_identity_t<T> lhs,
                                            const vec<N, T> &rhs) noexcept {
  return rhs + lhs;
}

template <uint32_t N, typename T>
[[nodiscard]] constexpr vec<N, T> operator-(std::type_identity_t<T> lhs,
                                            const vec<N, T> &rhs) noexcept {
  vec<N, T> result;
  for (uint32_t i = 0; i < N; ++i) {
    result[i] = lhs - rhs[i];
  }
  return result;
}

template <uint32_t N, typename T>
[[nodiscard]] constexpr vec<N, T> operator*(std::type_identity_t<T> lhs,
                                            const vec<N, T> &rhs) noexcept {
  return rhs * lhs;
}

template <uint32_t N, typename T>
[[nodiscard]] constexpr vec<N, T> operator/(std::type_identity_t<T> lhs,
                                            const vec<N, T> &rhs) noexcept {
  vec<N, T> result;
  for (uint32_t i = 0; i < N; ++i) {
    result[i] = lhs / rhs[i];
  }
  return result;
}

} // namespace detail

template <uint32_t N, typename T>
[[nodiscard]] constexpr T length2(const detail::vec<N, T> &value) noexcept {
  return value.length2();
}

template <uint32_t N, std::floating_point T>
[[nodiscard]] T length(const detail::vec<N, T> &value) noexcept {
  return value.length();
}

template <uint32_t N, std::floating_point T>
[[nodiscard]] detail::vec<N, T>
normalize(const detail::vec<N, T> &value) noexcept {
  return value / value.length();
}

template <uint32_t N, typename T>
[[nodiscard]] constexpr detail::vec<N, T>
min(const detail::vec<N, T> &lhs,
    const detail::vec<N, T> &rhs) noexcept {
  detail::vec<N, T> result;
  for (uint32_t i = 0; i < N; ++i) {
    result[i] = std::min(lhs[i], rhs[i]);
  }
  return result;
}

template <uint32_t N, typename T>
[[nodiscard]] constexpr detail::vec<N, T>
min(const detail::vec<N, T> &lhs, T rhs) noexcept {
  detail::vec<N, T> result;
  for (uint32_t i = 0; i < N; ++i) {
    result[i] = std::min(lhs[i], rhs);
  }
  return result;
}

template <uint32_t N, typename T>
[[nodiscard]] constexpr detail::vec<N, T>
max(const detail::vec<N, T> &lhs,
    const detail::vec<N, T> &rhs) noexcept {
  detail::vec<N, T> result;
  for (uint32_t i = 0; i < N; ++i) {
    result[i] = std::max(lhs[i], rhs[i]);
  }
  return result;
}

template <uint32_t N, typename T>
[[nodiscard]] constexpr detail::vec<N, T>
max(const detail::vec<N, T> &lhs, T rhs) noexcept {
  detail::vec<N, T> result;
  for (uint32_t i = 0; i < N; ++i) {
    result[i] = std::max(lhs[i], rhs);
  }
  return result;
}

template <uint32_t N, typename T>
[[nodiscard]] detail::vec<N, T>
abs(const detail::vec<N, T> &value) noexcept {
  detail::vec<N, T> result;
  for (uint32_t i = 0; i < N; ++i) {
    if constexpr (std::is_unsigned_v<T>)
      result[i] = value[i];
    else
      result[i] = std::abs(value[i]);
  }
  return result;
}

template <uint32_t N, typename T>
[[nodiscard]] constexpr detail::vec<N, T>
clamp(const detail::vec<N, T> &value, T lower, T upper) noexcept {
  return min(max(value, lower), upper);
}

template <uint32_t N, typename T>
[[nodiscard]] constexpr detail::vec<N, T>
clamp(const detail::vec<N, T> &value,
      const detail::vec<N, T> &lower,
      const detail::vec<N, T> &upper) noexcept {
  return min(max(value, lower), upper);
}

template <typename T>
[[nodiscard]] constexpr detail::vec<3, T>
cross(const detail::vec<3, T> &lhs,
      const detail::vec<3, T> &rhs) noexcept {
  return {lhs.y() * rhs.z() - lhs.z() * rhs.y(),
          lhs.z() * rhs.x() - lhs.x() * rhs.z(),
          lhs.x() * rhs.y() - lhs.y() * rhs.x()};
}

template <uint32_t N, typename T>
[[nodiscard]] constexpr T dot(const detail::vec<N, T> &lhs,
                              const detail::vec<N, T> &rhs) noexcept;

template <uint32_t N, typename T>
[[nodiscard]] constexpr detail::vec<N, T>
reflect(const detail::vec<N, T> &incident,
        const detail::vec<N, T> &normal) noexcept {
  return incident - normal * (T{2} * dot(incident, normal));
}

template <uint32_t N, std::floating_point T>
[[nodiscard]] detail::vec<N, T>
refract(const detail::vec<N, T> &incident,
        const detail::vec<N, T> &normal, T eta) noexcept {
  const T cosine = dot(incident, normal);
  const T discriminant = T{1} - eta * eta * (T{1} - cosine * cosine);
  if (discriminant < T{0}) {
    return {};
  }

  return eta * incident -
         normal * (eta * cosine + std::sqrt(discriminant));
}

template <uint32_t N, typename T>
[[nodiscard]] constexpr detail::vec<N, T>
lerp(const detail::vec<N, T> &lhs, const detail::vec<N, T> &rhs,
     T t) noexcept {
  return lhs + (rhs - lhs) * t;
}

template <uint32_t N, typename T>
[[nodiscard]] constexpr T dot(const detail::vec<N, T> &lhs,
                              const detail::vec<N, T> &rhs) noexcept {
  T out{};
  for (uint32_t i = 0; i < N; ++i) {
    out += lhs[i] * rhs[i];
  }
  return out;
}

using vec2 = detail::vec<2, float>;
using vec3 = detail::vec<3, float>;
using vec4 = detail::vec<4, float>;

using dvec2 = detail::vec<2, double>;
using dvec3 = detail::vec<3, double>;
using dvec4 = detail::vec<4, double>;

using uvec2 = detail::vec<2, uint32_t>;
using uvec3 = detail::vec<3, uint32_t>;
using uvec4 = detail::vec<4, uint32_t>;

using ivec2 = detail::vec<2, int32_t>;
using ivec3 = detail::vec<3, int32_t>;
using ivec4 = detail::vec<4, int32_t>;

} // namespace strobe

// Specialize std::tuple_size and std::tuple_element for std::get support
namespace std {
template <uint32_t N, typename T>
struct tuple_size<strobe::detail::vec<N, T>>
    : std::integral_constant<std::size_t, N> {};

template <std::size_t Index, uint32_t N, typename T>
struct tuple_element<Index, strobe::detail::vec<N, T>> {
  using type = T;
};

// Overload std::get for vec
template <std::size_t Index, uint32_t N, typename T>
constexpr T &get(strobe::detail::vec<N, T> &v) noexcept {
  static_assert(Index < N, "Index out of range for vec");
  return v.data[Index];
}

template <std::size_t Index, uint32_t N, typename T>
constexpr const T &get(const strobe::detail::vec<N, T> &v) noexcept {
  static_assert(Index < N, "Index out of range for vec");
  return v.data[Index];
}

template <std::size_t Index, uint32_t N, typename T>
constexpr T &&get(strobe::detail::vec<N, T> &&v) noexcept {
  static_assert(Index < N, "Index out of range for vec");
  return std::move(v.data[Index]);
}

template <std::size_t Index, uint32_t N, typename T>
constexpr const T &&get(const strobe::detail::vec<N, T> &&v) noexcept {
  static_assert(Index < N, "Index out of range for vec");
  return std::move(v.data[Index]);
}

} // namespace std
