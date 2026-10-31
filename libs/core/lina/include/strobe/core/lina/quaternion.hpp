#pragma once

#include "strobe/core/containers/bitset.hpp"
#include "strobe/core/lina/vec.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <concepts>
#include <cstdint>
#include <limits>

namespace strobe {

namespace detail {

template <std::floating_point T> struct quaternion {
  std::array<T, 4> data;

  constexpr quaternion() noexcept : data{T{0}, T{0}, T{0}, T{1}} {}

  constexpr quaternion(T x, T y, T z, T w) noexcept : data{x, y, z, w} {}

  constexpr explicit quaternion(std::array<T, 4> values) noexcept
      : data{values} {}

  [[nodiscard]] static constexpr quaternion identity() noexcept {
    return {};
  }

  [[nodiscard]] static quaternion rotation(const vec<3, T> &axis,
                                           T angle) noexcept {
    const T axis_length =
        std::sqrt(axis.x() * axis.x() + axis.y() * axis.y() +
                  axis.z() * axis.z());
    if (axis_length == T{0}) {
      return {};
    }

    const T half_angle = angle / T{2};
    const T scale = std::sin(half_angle) / axis_length;
    return {axis.x() * scale, axis.y() * scale, axis.z() * scale,
            std::cos(half_angle)};
  }

  [[nodiscard]] static quaternion rotation_between(const vec<3, T> &from,
                                                   const vec<3, T> &to) noexcept {
    const T from_length =
        std::sqrt(from.x() * from.x() + from.y() * from.y() +
                  from.z() * from.z());
    const T to_length =
        std::sqrt(to.x() * to.x() + to.y() * to.y() + to.z() * to.z());

    const vec<3, T> source{from.x() / from_length, from.y() / from_length,
                           from.z() / from_length};
    const vec<3, T> target{to.x() / to_length, to.y() / to_length,
                           to.z() / to_length};

    T cosine = source.x() * target.x() + source.y() * target.y() +
               source.z() * target.z();
    const T threshold = std::sqrt(std::numeric_limits<T>::epsilon());

    if (cosine >= T{1} - threshold) {
      return identity();
    }

    if (cosine <= T{-1} + threshold) {
      vec<3, T> axis;
      if (std::abs(source.x()) > std::abs(source.z())) {
        axis = {-source.y(), source.x(), T{0}};
      } else {
        axis = {T{0}, -source.z(), source.y()};
      }
      return rotation(axis, std::acos(T{-1}));
    }

    const vec<3, T> cross{source.y() * target.z() - source.z() * target.y(),
                          source.z() * target.x() - source.x() * target.z(),
                          source.x() * target.y() - source.y() * target.x()};
    cosine = std::max(T{-1}, std::min(T{1}, cosine));
    const T scale = std::sqrt((T{1} + cosine) * T{2});
    return {cross.x() / scale, cross.y() / scale, cross.z() / scale,
            scale / T{2}};
  }

  [[nodiscard]] static quaternion from_euler(const vec<3, T> &angles) noexcept {
    const quaternion pitch = rotation({T{1}, T{0}, T{0}}, angles.x());
    const quaternion yaw = rotation({T{0}, T{1}, T{0}}, angles.y());
    const quaternion roll = rotation({T{0}, T{0}, T{1}}, angles.z());

    quaternion result = yaw;
    result *= pitch;
    result *= roll;
    return result;
  }

  [[nodiscard]] constexpr vec<3, T>
  operator*(const vec<3, T> &rhs) const noexcept {
    const T xx = x() * x();
    const T yy = y() * y();
    const T zz = z() * z();
    const T xy = x() * y();
    const T xz = x() * z();
    const T yz = y() * z();
    const T wx = w() * x();
    const T wy = w() * y();
    const T wz = w() * z();

    return {(T{1} - T{2} * (yy + zz)) * rhs.x() +
                T{2} * (xy - wz) * rhs.y() + T{2} * (xz + wy) * rhs.z(),
            T{2} * (xy + wz) * rhs.x() +
                (T{1} - T{2} * (xx + zz)) * rhs.y() +
                T{2} * (yz - wx) * rhs.z(),
            T{2} * (xz - wy) * rhs.x() + T{2} * (yz + wx) * rhs.y() +
                (T{1} - T{2} * (xx + yy)) * rhs.z()};
  }

  [[nodiscard]] T angle() const noexcept {
    const T magnitude =
        std::sqrt(x() * x() + y() * y() + z() * z() + w() * w());
    if (magnitude == T{0}) {
      return T{0};
    }

    T normalized_w = w() / magnitude;
    if (normalized_w < T{-1}) {
      normalized_w = T{-1};
    } else if (normalized_w > T{1}) {
      normalized_w = T{1};
    }

    return T{2} * std::acos(std::abs(normalized_w));
  }

  [[nodiscard]] vec<3, T> axis() const noexcept {
    const T magnitude =
        std::sqrt(x() * x() + y() * y() + z() * z() + w() * w());
    if (magnitude == T{0}) {
      return {T{1}, T{0}, T{0}};
    }

    T nx = x() / magnitude;
    T ny = y() / magnitude;
    T nz = z() / magnitude;
    T nw = w() / magnitude;
    if (nw < T{0}) {
      nx = -nx;
      ny = -ny;
      nz = -nz;
    }

    const T axis_length = std::sqrt(nx * nx + ny * ny + nz * nz);
    if (axis_length == T{0}) {
      return {T{1}, T{0}, T{0}};
    }

    return {nx / axis_length, ny / axis_length, nz / axis_length};
  }

  constexpr T &x() noexcept { return data[0]; }
  constexpr T &y() noexcept { return data[1]; }
  constexpr T &z() noexcept { return data[2]; }
  constexpr T &w() noexcept { return data[3]; }

  constexpr const T &x() const noexcept { return data[0]; }
  constexpr const T &y() const noexcept { return data[1]; }
  constexpr const T &z() const noexcept { return data[2]; }
  constexpr const T &w() const noexcept { return data[3]; }

  constexpr T &operator[](uint32_t i) noexcept {
    assert(i < data.size());
    return data[i];
  }
  constexpr const T &operator[](uint32_t i) const noexcept {
    assert(i < data.size());
    return data[i];
  }

  [[nodiscard]] constexpr quaternion operator+() const noexcept { return *this; }

  [[nodiscard]] constexpr quaternion operator-() const noexcept {
    return {-x(), -y(), -z(), -w()};
  }

  [[nodiscard]] constexpr bool operator==(const quaternion &rhs) const noexcept {
    return data == rhs.data;
  }

  [[nodiscard]] constexpr bool operator!=(const quaternion &rhs) const noexcept {
    return !(*this == rhs);
  }

  [[nodiscard]] constexpr T length2() const noexcept {
    return x() * x() + y() * y() + z() * z() + w() * w();
  }

  [[nodiscard]] T length() const noexcept { return std::sqrt(length2()); }

  constexpr quaternion &operator+=(const quaternion &rhs) noexcept {
    for (uint32_t i = 0; i < 4; ++i) {
      data[i] += rhs.data[i];
    }
    return *this;
  }

  constexpr quaternion &operator-=(const quaternion &rhs) noexcept {
    for (uint32_t i = 0; i < 4; ++i) {
      data[i] -= rhs.data[i];
    }
    return *this;
  }

  constexpr quaternion &operator*=(const quaternion &rhs) noexcept {
    const T ax = x(), ay = y(), az = z(), aw = w();
    const T bx = rhs.x(), by = rhs.y(), bz = rhs.z(), bw = rhs.w();

    data = {aw * bx + ax * bw + ay * bz - az * by,
            aw * by - ax * bz + ay * bw + az * bx,
            aw * bz + ax * by - ay * bx + az * bw,
            aw * bw - ax * bx - ay * by - az * bz};
    return *this;
  }

  constexpr quaternion &operator*=(T scalar) noexcept {
    for (T &v : data) {
      v *= scalar;
    }
    return *this;
  }

  constexpr quaternion &operator/=(T scalar) noexcept {
    for (T &v : data) {
      v /= scalar;
    }
    return *this;
  }

  [[nodiscard]] constexpr quaternion operator/(T scalar) const noexcept {
    quaternion result = *this;
    result /= scalar;
    return result;
  }
};

template <std::floating_point T>
[[nodiscard]] constexpr Bitset<4>
operator<(const quaternion<T> &lhs, const quaternion<T> &rhs) noexcept {
  Bitset<4> result;
  for (uint32_t i = 0; i < 4; ++i) {
    if (lhs[i] < rhs[i]) {
      result.set(i);
    }
  }
  return result;
}

template <std::floating_point T>
[[nodiscard]] constexpr Bitset<4>
operator<=(const quaternion<T> &lhs, const quaternion<T> &rhs) noexcept {
  Bitset<4> result;
  for (uint32_t i = 0; i < 4; ++i) {
    if (lhs[i] <= rhs[i]) {
      result.set(i);
    }
  }
  return result;
}

template <std::floating_point T>
[[nodiscard]] constexpr Bitset<4>
operator>(const quaternion<T> &lhs, const quaternion<T> &rhs) noexcept {
  Bitset<4> result;
  for (uint32_t i = 0; i < 4; ++i) {
    if (lhs[i] > rhs[i]) {
      result.set(i);
    }
  }
  return result;
}

template <std::floating_point T>
[[nodiscard]] constexpr Bitset<4>
operator>=(const quaternion<T> &lhs, const quaternion<T> &rhs) noexcept {
  Bitset<4> result;
  for (uint32_t i = 0; i < 4; ++i) {
    if (lhs[i] >= rhs[i]) {
      result.set(i);
    }
  }
  return result;
}

} // namespace detail

template <typename T>
[[nodiscard]] constexpr detail::quaternion<T>
conjugate(const detail::quaternion<T> &q) noexcept {
  return {-q.x(), -q.y(), -q.z(), q.w()};
}

template <typename T>
[[nodiscard]] constexpr T dot(const detail::quaternion<T> &lhs,
                              const detail::quaternion<T> &rhs) noexcept {
  return lhs.x() * rhs.x() + lhs.y() * rhs.y() + lhs.z() * rhs.z() +
         lhs.w() * rhs.w();
}

template <typename T>
[[nodiscard]] constexpr T length2(const detail::quaternion<T> &q) noexcept {
  return dot(q, q);
}

template <typename T>
[[nodiscard]] T length(const detail::quaternion<T> &q) noexcept {
  return std::sqrt(length2(q));
}

template <typename T>
[[nodiscard]] detail::quaternion<T>
normalize(const detail::quaternion<T> &q) noexcept {
  return q / length(q);
}

template <typename T>
[[nodiscard]] detail::quaternion<T>
inverse(const detail::quaternion<T> &q) noexcept {
  return conjugate(q) / length2(q);
}

namespace detail {

template <typename T>
[[nodiscard]] constexpr quaternion<T>
operator+(const quaternion<T> &lhs, const quaternion<T> &rhs) noexcept {
  return {lhs.x() + rhs.x(), lhs.y() + rhs.y(), lhs.z() + rhs.z(),
          lhs.w() + rhs.w()};
}

template <typename T>
[[nodiscard]] constexpr quaternion<T>
operator-(const quaternion<T> &lhs, const quaternion<T> &rhs) noexcept {
  return {lhs.x() - rhs.x(), lhs.y() - rhs.y(), lhs.z() - rhs.z(),
          lhs.w() - rhs.w()};
}

template <typename T>
[[nodiscard]] constexpr quaternion<T>
operator*(const quaternion<T> &a, const quaternion<T> &b) noexcept {
  return {a.w() * b.x() + a.x() * b.w() + a.y() * b.z() - a.z() * b.y(),
          a.w() * b.y() - a.x() * b.z() + a.y() * b.w() + a.z() * b.x(),
          a.w() * b.z() + a.x() * b.y() - a.y() * b.x() + a.z() * b.w(),
          a.w() * b.w() - a.x() * b.x() - a.y() * b.y() - a.z() * b.z()};
}

template <typename T>
[[nodiscard]] constexpr quaternion<T>
operator*(const quaternion<T> &lhs, T rhs) noexcept {
  return {lhs.x() * rhs, lhs.y() * rhs, lhs.z() * rhs, lhs.w() * rhs};
}

template <typename T>
[[nodiscard]] constexpr quaternion<T>
operator*(T lhs, const quaternion<T> &rhs) noexcept {
  return rhs * lhs;
}

} // namespace detail

template <typename T>
[[nodiscard]] constexpr detail::quaternion<T>
lerp(const detail::quaternion<T> &a, const detail::quaternion<T> &b,
     T t) noexcept {
  return {a.x() + (b.x() - a.x()) * t,
          a.y() + (b.y() - a.y()) * t,
          a.z() + (b.z() - a.z()) * t,
          a.w() + (b.w() - a.w()) * t};
}

template <typename T>
[[nodiscard]] detail::quaternion<T>
nlerp(const detail::quaternion<T> &a, const detail::quaternion<T> &b,
      T t) noexcept {
  const auto target = dot(a, b) < T{0} ? -b : b;
  return normalize(lerp(a, target, t));
}

template <typename T>
[[nodiscard]] detail::quaternion<T>
slerp(const detail::quaternion<T> &a, const detail::quaternion<T> &b,
      T t) noexcept {
  const auto start = normalize(a);
  auto target = normalize(b);
  T cosine = dot(start, target);

  if (cosine < T{0}) {
    target = -target;
    cosine = -cosine;
  }

  if (cosine > T{0.9995}) {
    return nlerp(start, target, t);
  }

  if (cosine < T{-1}) {
    cosine = T{-1};
  } else if (cosine > T{1}) {
    cosine = T{1};
  }

  const T angle = std::acos(cosine);
  const T sine = std::sin(angle);
  const T start_weight = std::sin((T{1} - t) * angle) / sine;
  const T target_weight = std::sin(t * angle) / sine;
  return normalize(start * start_weight + target * target_weight);
}

template <typename T>
[[nodiscard]] detail::quaternion<T>
exp(const detail::quaternion<T> &q) noexcept {
  const T vector_length =
      std::sqrt(q.x() * q.x() + q.y() * q.y() + q.z() * q.z());
  const T scalar_exp = std::exp(q.w());

  if (vector_length == T{0}) {
    return {T{0}, T{0}, T{0}, scalar_exp};
  }

  const T scale = scalar_exp * std::sin(vector_length) / vector_length;
  return {q.x() * scale, q.y() * scale, q.z() * scale,
          scalar_exp * std::cos(vector_length)};
}

template <typename T>
[[nodiscard]] detail::quaternion<T>
log(const detail::quaternion<T> &q) noexcept {
  const T magnitude = std::sqrt(length2(q));
  const T vector_length =
      std::sqrt(q.x() * q.x() + q.y() * q.y() + q.z() * q.z());

  if (vector_length == T{0}) {
    return {T{0}, T{0}, T{0}, std::log(magnitude)};
  }

  const T cosine = std::clamp(q.w() / magnitude, T{-1}, T{1});
  const T scale = std::acos(cosine) / vector_length;
  return {q.x() * scale, q.y() * scale, q.z() * scale,
          std::log(magnitude)};
}

template <typename T>
[[nodiscard]] detail::quaternion<T>
pow(const detail::quaternion<T> &q, T exponent) noexcept {
  return exp(log(q) * exponent);
}

template <typename T>
[[nodiscard]] detail::quaternion<T>
sqrt(const detail::quaternion<T> &q) noexcept {
  return pow(q, T{0.5});
}

template <std::floating_point T>
[[nodiscard]] detail::quaternion<T>
abs(const detail::quaternion<T> &q) noexcept {
  return {std::abs(q.x()), std::abs(q.y()), std::abs(q.z()), std::abs(q.w())};
}

using quaternion = detail::quaternion<float>;

} // namespace strobe
