#pragma once

#include "strobe/core/containers/bitset.hpp"
#include "strobe/core/lina/quaternion.hpp"
#include "strobe/core/lina/vec.hpp"
#include <array>
#include <cassert>
#include <cmath>
#include <concepts>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>
namespace strobe {

namespace detail {

/// N : rows
/// M : columns
/// T : underlying_type
template <uint32_t N, uint32_t M, typename T>
  requires std::is_arithmetic_v<T>
struct mat {
  using column_type = detail::vec<N, T>;

  std::array<column_type, M> data = {};

  constexpr mat() noexcept = default;

  constexpr explicit mat(std::array<column_type, M> columns) noexcept
      : data(columns) {}

  constexpr mat(column_type c1) noexcept
    requires(M >= 1)
      : data{c1} {}

  constexpr mat(column_type c1, column_type c2) noexcept
    requires(M >= 2)
      : data{c1, c2} {}

  constexpr mat(column_type c1, column_type c2, column_type c3) noexcept
    requires(M >= 3)
      : data{c1, c2, c3} {}

  constexpr mat(column_type c1, column_type c2, column_type c3,
                column_type c4) noexcept
    requires(M >= 4)
      : data{c1, c2, c3, c4} {}

  template <typename U>
    requires(std::is_arithmetic_v<U> && std::is_convertible_v<U, T>)
  constexpr mat(const mat<N, M, U> &other) noexcept {
    for (uint32_t column = 0; column < M; ++column) {
      for (uint32_t row = 0; row < N; ++row) {
        data[column][row] = static_cast<T>(other[column][row]);
      }
    }
  }

  [[nodiscard]] static constexpr mat identity() noexcept
    requires(N == M)
  {
    mat r{};
    for (uint32_t i = 0; i < N; ++i) {
      r.at(i, i) = T{1};
    }
    return r;
  }

  [[nodiscard]] static mat perspective(T fov_y, T aspect, T near, T far,
                                       bool reverseZ = false,
                                       bool flipY = false) noexcept
    requires(N == 4 && M == 4 && std::floating_point<T>)
  {
    const T f = T{1} / std::tan(fov_y / T{2});
    const T depth_scale =
        reverseZ ? (far + near) / (far - near) : (far + near) / (near - far);
    const T depth_offset = reverseZ ? (T{2} * near * far) / (far - near)
                                    : (T{2} * near * far) / (near - far);

    mat r{};
    r.at(0, 0) = f / aspect;
    r.at(1, 1) = flipY ? -f : f;
    r.at(2, 2) = depth_scale;
    r.at(2, 3) = depth_offset;
    r.at(3, 2) = T{-1};
    return r;
  }

  [[nodiscard]] static mat projection(T left, T right, T bottom, T top,
                                      T near, T far) noexcept
    requires(N == 4 && M == 4 && std::floating_point<T>)
  {
    mat r{};
    r.at(0, 0) = T{2} * near / (right - left);
    r.at(1, 1) = T{2} * near / (top - bottom);
    r.at(0, 2) = (right + left) / (right - left);
    r.at(1, 2) = (top + bottom) / (top - bottom);
    r.at(2, 2) = (far + near) / (near - far);
    r.at(2, 3) = T{2} * far * near / (near - far);
    r.at(3, 2) = T{-1};
    return r;
  }

  [[nodiscard]] static mat ortho(T left, T right, T bottom, T top, T near,
                                 T far, bool reverseZ = false,
                                 bool flipY = false) noexcept
    requires(N == 4 && M == 4 && std::floating_point<T>)
  {
    const T y_scale = (flipY ? -T{2} : T{2}) / (top - bottom);
    const T y_offset = (flipY ? T{1} : T{-1}) * (top + bottom) / (top - bottom);
    const T z_scale = reverseZ ? T{2} / (far - near) : T{2} / (near - far);
    const T z_offset =
        reverseZ ? (far + near) / (far - near) : (far + near) / (near - far);

    mat r{};
    r.at(0, 0) = T{2} / (right - left);
    r.at(1, 1) = y_scale;
    r.at(2, 2) = z_scale;
    r.at(0, 3) = -(right + left) / (right - left);
    r.at(1, 3) = y_offset;
    r.at(2, 3) = z_offset;
    r.at(3, 3) = T{1};
    return r;
  }

  [[nodiscard]] static constexpr mat
  translation(const detail::vec<3, T> &value) noexcept
    requires(N == 4 && M == 4 && std::floating_point<T>)
  {
    mat r = identity();
    r.at(0, 3) = value.x();
    r.at(1, 3) = value.y();
    r.at(2, 3) = value.z();
    return r;
  }

  [[nodiscard]] static mat scaling(T x, T y, T z) noexcept
    requires(N == 4 && M == 4 && std::floating_point<T>)
  {
    mat r{};
    r.at(0, 0) = x;
    r.at(1, 1) = y;
    r.at(2, 2) = z;
    r.at(3, 3) = T{1};
    return r;
  }

  [[nodiscard]] static mat scaling(const detail::vec<N, T> &value) noexcept
    requires(N == M && std::floating_point<T>)
  {
    mat r = identity();
    for (uint32_t i = 0; i < N; ++i) {
      r.at(i, i) = value[i];
    }
    return r;
  }

  [[nodiscard]] static mat rotation(const detail::vec<3, T> &axis,
                                    T angle) noexcept
    requires(((N == 3 && M == 3) || (N == 4 && M == 4)) &&
             std::floating_point<T>)
  {
    const T axis_length = std::sqrt(axis.x() * axis.x() +
                                    axis.y() * axis.y() +
                                    axis.z() * axis.z());
    if (axis_length == T{0}) {
      return identity();
    }

    const T x = axis.x() / axis_length;
    const T y = axis.y() / axis_length;
    const T z = axis.z() / axis_length;
    const T sine = std::sin(angle);
    const T cosine = std::cos(angle);
    const T one_minus_cosine = T{1} - cosine;

    mat result = identity();
    result.at(0, 0) = cosine + x * x * one_minus_cosine;
    result.at(0, 1) = x * y * one_minus_cosine - z * sine;
    result.at(0, 2) = x * z * one_minus_cosine + y * sine;
    result.at(1, 0) = y * x * one_minus_cosine + z * sine;
    result.at(1, 1) = cosine + y * y * one_minus_cosine;
    result.at(1, 2) = y * z * one_minus_cosine - x * sine;
    result.at(2, 0) = z * x * one_minus_cosine - y * sine;
    result.at(2, 1) = z * y * one_minus_cosine + x * sine;
    result.at(2, 2) = cosine + z * z * one_minus_cosine;
    return result;
  }

  [[nodiscard]] static mat from_euler(const detail::vec<3, T> &angles) noexcept
    requires(((N == 3 && M == 3) || (N == 4 && M == 4)) &&
             std::floating_point<T>)
  {
    const T half_pitch = angles.x() / T{2};
    const T half_yaw = angles.y() / T{2};
    const T half_roll = angles.z() / T{2};
    const T sp = std::sin(half_pitch);
    const T cp = std::cos(half_pitch);
    const T sy = std::sin(half_yaw);
    const T cy = std::cos(half_yaw);
    const T sr = std::sin(half_roll);
    const T cr = std::cos(half_roll);

    const T qx = cy * sp * cr + sy * cp * sr;
    const T qy = -cy * sp * sr + sy * cp * cr;
    const T qz = cy * cp * sr - sy * sp * cr;
    const T qw = cy * cp * cr + sy * sp * sr;

    mat r = identity();
    r.at(0, 0) = T{1} - T{2} * (qy * qy + qz * qz);
    r.at(0, 1) = T{2} * (qx * qy - qw * qz);
    r.at(0, 2) = T{2} * (qx * qz + qw * qy);
    r.at(1, 0) = T{2} * (qx * qy + qw * qz);
    r.at(1, 1) = T{1} - T{2} * (qx * qx + qz * qz);
    r.at(1, 2) = T{2} * (qy * qz - qw * qx);
    r.at(2, 0) = T{2} * (qx * qz - qw * qy);
    r.at(2, 1) = T{2} * (qy * qz + qw * qx);
    r.at(2, 2) = T{1} - T{2} * (qx * qx + qy * qy);
    return r;
  }

  [[nodiscard]] static mat transform(const detail::vec<3, T> &position,
                                     const detail::vec<3, T> &scale,
                                     const detail::vec<3, T> &angles) noexcept
    requires(N == 4 && M == 4 && std::floating_point<T>)
  {
    const T half_pitch = angles.x() / T{2};
    const T half_yaw = angles.y() / T{2};
    const T half_roll = angles.z() / T{2};
    const T sp = std::sin(half_pitch);
    const T cp = std::cos(half_pitch);
    const T sy = std::sin(half_yaw);
    const T cy = std::cos(half_yaw);
    const T sr = std::sin(half_roll);
    const T cr = std::cos(half_roll);

    const T qx = cy * sp * cr + sy * cp * sr;
    const T qy = -cy * sp * sr + sy * cp * cr;
    const T qz = cy * cp * sr - sy * sp * cr;
    const T qw = cy * cp * cr + sy * sp * sr;

    const T xx = qx * qx;
    const T yy = qy * qy;
    const T zz = qz * qz;
    const T xy = qx * qy;
    const T xz = qx * qz;
    const T yz = qy * qz;
    const T wx = qw * qx;
    const T wy = qw * qy;
    const T wz = qw * qz;

    mat r{};
    r.at(0, 0) = (T{1} - T{2} * (yy + zz)) * scale.x();
    r.at(1, 0) = T{2} * (xy + wz) * scale.x();
    r.at(2, 0) = T{2} * (xz - wy) * scale.x();
    r.at(0, 1) = T{2} * (xy - wz) * scale.y();
    r.at(1, 1) = (T{1} - T{2} * (xx + zz)) * scale.y();
    r.at(2, 1) = T{2} * (yz + wx) * scale.y();
    r.at(0, 2) = T{2} * (xz + wy) * scale.z();
    r.at(1, 2) = T{2} * (yz - wx) * scale.z();
    r.at(2, 2) = (T{1} - T{2} * (xx + yy)) * scale.z();
    r.at(0, 3) = position.x();
    r.at(1, 3) = position.y();
    r.at(2, 3) = position.z();
    r.at(3, 3) = T{1};
    return r;
  }

  template <std::floating_point U>
  [[nodiscard]] static mat
  transform(const detail::vec<3, T> &position, const detail::vec<3, T> &scale,
            const detail::quaternion<U> &rotation) noexcept
    requires(N == 4 && M == 4 && std::floating_point<T> &&
             std::same_as<T, U>)
  {
    const T xx = rotation.x() * rotation.x();
    const T yy = rotation.y() * rotation.y();
    const T zz = rotation.z() * rotation.z();
    const T xy = rotation.x() * rotation.y();
    const T xz = rotation.x() * rotation.z();
    const T yz = rotation.y() * rotation.z();
    const T wx = rotation.w() * rotation.x();
    const T wy = rotation.w() * rotation.y();
    const T wz = rotation.w() * rotation.z();

    mat result{};
    result.at(0, 0) = (T{1} - T{2} * (yy + zz)) * scale.x();
    result.at(1, 0) = T{2} * (xy + wz) * scale.x();
    result.at(2, 0) = T{2} * (xz - wy) * scale.x();
    result.at(0, 1) = T{2} * (xy - wz) * scale.y();
    result.at(1, 1) = (T{1} - T{2} * (xx + zz)) * scale.y();
    result.at(2, 1) = T{2} * (yz + wx) * scale.y();
    result.at(0, 2) = T{2} * (xz + wy) * scale.z();
    result.at(1, 2) = T{2} * (yz - wx) * scale.z();
    result.at(2, 2) = (T{1} - T{2} * (xx + yy)) * scale.z();
    result.at(0, 3) = position.x();
    result.at(1, 3) = position.y();
    result.at(2, 3) = position.z();
    result.at(3, 3) = T{1};
    return result;
  }

  template <std::floating_point U>
  [[nodiscard]] static mat
  from_quaternion(const detail::quaternion<U> &q) noexcept
    requires(((N == 3 && M == 3) || (N == 4 && M == 4)) &&
             std::floating_point<T> && std::same_as<T, U>)
  {
    const T xx = q.x() * q.x();
    const T yy = q.y() * q.y();
    const T zz = q.z() * q.z();
    const T xy = q.x() * q.y();
    const T xz = q.x() * q.z();
    const T yz = q.y() * q.z();
    const T wx = q.w() * q.x();
    const T wy = q.w() * q.y();
    const T wz = q.w() * q.z();

    mat result = identity();
    result.at(0, 0) = T{1} - T{2} * (yy + zz);
    result.at(0, 1) = T{2} * (xy - wz);
    result.at(0, 2) = T{2} * (xz + wy);
    result.at(1, 0) = T{2} * (xy + wz);
    result.at(1, 1) = T{1} - T{2} * (xx + zz);
    result.at(1, 2) = T{2} * (yz - wx);
    result.at(2, 0) = T{2} * (xz - wy);
    result.at(2, 1) = T{2} * (yz + wx);
    result.at(2, 2) = T{1} - T{2} * (xx + yy);
    return result;
  }

  [[nodiscard]] static mat look_to(const detail::vec<3, T> &position,
                                   const detail::vec<3, T> &direction,
                                   const detail::vec<3, T> &up) noexcept
    requires(N == 4 && M == 4 && std::floating_point<T>)
  {
    const auto forward = normalize(direction);
    const auto right = normalize(cross(forward, up));
    const auto camera_up = cross(right, forward);

    mat result = identity();
    result.at(0, 0) = right.x();
    result.at(0, 1) = right.y();
    result.at(0, 2) = right.z();
    result.at(0, 3) = -dot(right, position);

    result.at(1, 0) = camera_up.x();
    result.at(1, 1) = camera_up.y();
    result.at(1, 2) = camera_up.z();
    result.at(1, 3) = -dot(camera_up, position);

    result.at(2, 0) = -forward.x();
    result.at(2, 1) = -forward.y();
    result.at(2, 2) = -forward.z();
    result.at(2, 3) = dot(forward, position);
    return result;
  }

  [[nodiscard]] static mat look_at(const detail::vec<3, T> &position,
                                   const detail::vec<3, T> &target,
                                   const detail::vec<3, T> &up) noexcept
    requires(N == 4 && M == 4 && std::floating_point<T>)
  {
    return look_to(position, target - position, up);
  }

  constexpr const column_type &operator[](uint32_t column) const noexcept {
    assert(column < M);
    return data[column];
  }
  constexpr column_type &operator[](uint32_t column) noexcept {
    assert(column < M);
    return data[column];
  }

  const T &at(uint32_t row, uint32_t column) const noexcept {
    assert(row < N);
    assert(column < M);
    return data[column][row];
  }

  T &at(uint32_t row, uint32_t column) noexcept {
    assert(row < N);
    assert(column < M);
    return data[column][row];
  }

  [[nodiscard]] detail::vec<M, T> row(uint32_t index) const noexcept {
    assert(index < N);
    detail::vec<M, T> result;
    for (uint32_t column = 0; column < M; ++column) {
      result[column] = data[column][index];
    }
    return result;
  }

  [[nodiscard]] detail::vec<N, T> column(uint32_t index) const noexcept {
    assert(index < M);
    return data[index];
  }

  [[nodiscard]] constexpr mat operator+() const noexcept { return *this; }

  [[nodiscard]] constexpr mat operator-() const noexcept {
    mat result{};
    for (uint32_t column = 0; column < M; ++column) {
      for (uint32_t row = 0; row < N; ++row) {
        result.at(row, column) = -at(row, column);
      }
    }
    return result;
  }

  friend constexpr bool operator==(const mat &lhs, const mat &rhs) noexcept {
    for (uint32_t column = 0; column < M; ++column) {
      if (lhs.data[column] != rhs.data[column]) {
        return false;
      }
    }
    return true;
  }

  friend constexpr bool operator!=(const mat &lhs, const mat &rhs) noexcept {
    return !(lhs == rhs);
  }

  constexpr mat &operator+=(const mat &rhs) noexcept {
    for (uint32_t column = 0; column < M; ++column) {
      data[column] += rhs.data[column];
    }
    return *this;
  }

  constexpr mat &operator+=(const T &rhs) noexcept {
    for (uint32_t column = 0; column < M; ++column) {
      data[column] += rhs;
    }
    return *this;
  }

  constexpr mat &operator-=(const mat &rhs) noexcept {
    for (uint32_t column = 0; column < M; ++column) {
      data[column] -= rhs.data[column];
    }
    return *this;
  }

  constexpr mat &operator-=(const T &rhs) noexcept {
    for (uint32_t column = 0; column < M; ++column) {
      data[column] -= rhs;
    }
    return *this;
  }

  constexpr mat &operator*=(const T &rhs) noexcept {
    for (uint32_t column = 0; column < M; ++column) {
      data[column] *= rhs;
    }
    return *this;
  }

  constexpr mat &operator/=(const T &rhs) noexcept {
    for (uint32_t column = 0; column < M; ++column) {
      data[column] /= rhs;
    }
    return *this;
  }

  constexpr mat &operator*=(const mat &rhs) noexcept
    requires(N == M)
  {
    mat result{};
    for (uint32_t column = 0; column < M; ++column) {
      for (uint32_t row = 0; row < N; ++row) {
        for (uint32_t i = 0; i < N; ++i) {
          result.at(row, column) += at(row, i) * rhs.at(i, column);
        }
      }
    }
    data = result.data;
    return *this;
  }
};

template <uint32_t N, uint32_t M, typename T>
  requires std::is_arithmetic_v<T>
[[nodiscard]] constexpr Bitset<N * M>
operator<(const mat<N, M, T> &lhs, const mat<N, M, T> &rhs) noexcept {
  Bitset<N * M> result;
  for (uint32_t column = 0; column < M; ++column) {
    for (uint32_t row = 0; row < N; ++row) {
      if (lhs.at(row, column) < rhs.at(row, column)) {
        result.set(column * N + row);
      }
    }
  }
  return result;
}

template <uint32_t N, uint32_t M, typename T>
  requires std::is_arithmetic_v<T>
[[nodiscard]] constexpr Bitset<N * M>
operator<=(const mat<N, M, T> &lhs, const mat<N, M, T> &rhs) noexcept {
  Bitset<N * M> result;
  for (uint32_t column = 0; column < M; ++column) {
    for (uint32_t row = 0; row < N; ++row) {
      if (lhs.at(row, column) <= rhs.at(row, column)) {
        result.set(column * N + row);
      }
    }
  }
  return result;
}

template <uint32_t N, uint32_t M, typename T>
  requires std::is_arithmetic_v<T>
[[nodiscard]] constexpr Bitset<N * M>
operator>(const mat<N, M, T> &lhs, const mat<N, M, T> &rhs) noexcept {
  Bitset<N * M> result;
  for (uint32_t column = 0; column < M; ++column) {
    for (uint32_t row = 0; row < N; ++row) {
      if (lhs.at(row, column) > rhs.at(row, column)) {
        result.set(column * N + row);
      }
    }
  }
  return result;
}

template <uint32_t N, uint32_t M, typename T>
  requires std::is_arithmetic_v<T>
[[nodiscard]] constexpr Bitset<N * M>
operator>=(const mat<N, M, T> &lhs, const mat<N, M, T> &rhs) noexcept {
  Bitset<N * M> result;
  for (uint32_t column = 0; column < M; ++column) {
    for (uint32_t row = 0; row < N; ++row) {
      if (lhs.at(row, column) >= rhs.at(row, column)) {
        result.set(column * N + row);
      }
    }
  }
  return result;
}

template <uint32_t N, std::floating_point T>
[[nodiscard]] mat<N, N, T> nan_matrix() noexcept {
  mat<N, N, T> r;
  const T nan = std::numeric_limits<T>::quiet_NaN();
  for (uint32_t i = 0; i < N; ++i) {
    for (uint32_t j = 0; j < N; ++j) {
      r.at(i, j) = nan;
    }
  }
  return r;
}

} // namespace detail

template <uint32_t N, uint32_t M, typename T>
  requires std::is_arithmetic_v<T>
[[nodiscard]] constexpr detail::mat<N, M, T>
outer_product(const detail::vec<N, T> &lhs,
              const detail::vec<M, T> &rhs) noexcept {
  detail::mat<N, M, T> result{};
  for (uint32_t column = 0; column < M; ++column) {
    for (uint32_t row = 0; row < N; ++row) {
      result.at(row, column) = lhs[row] * rhs[column];
    }
  }
  return result;
}

template <uint32_t N, uint32_t M, typename T>
  requires std::is_arithmetic_v<T>
[[nodiscard]] constexpr Bitset<N * M>
equal(const detail::mat<N, M, T> &lhs,
      const detail::mat<N, M, T> &rhs) noexcept {
  Bitset<N * M> result;
  for (uint32_t column = 0; column < M; ++column) {
    for (uint32_t row = 0; row < N; ++row) {
      if (lhs.at(row, column) == rhs.at(row, column)) {
        result.set(column * N + row);
      }
    }
  }
  return result;
}

template <uint32_t N, uint32_t M, std::floating_point T>
[[nodiscard]] Bitset<N * M>
equal(const detail::mat<N, M, T> &lhs,
      const detail::mat<N, M, T> &rhs, T epsilon) noexcept {
  Bitset<N * M> result;
  for (uint32_t column = 0; column < M; ++column) {
    for (uint32_t row = 0; row < N; ++row) {
      if (std::abs(lhs.at(row, column) - rhs.at(row, column)) <= epsilon) {
        result.set(column * N + row);
      }
    }
  }
  return result;
}

template <uint32_t N, uint32_t M, typename T>
  requires std::is_arithmetic_v<T>
[[nodiscard]] constexpr Bitset<N * M>
unequal(const detail::mat<N, M, T> &lhs,
        const detail::mat<N, M, T> &rhs) noexcept {
  Bitset<N * M> result;
  for (uint32_t column = 0; column < M; ++column) {
    for (uint32_t row = 0; row < N; ++row) {
      if (lhs.at(row, column) != rhs.at(row, column)) {
        result.set(column * N + row);
      }
    }
  }
  return result;
}

template <uint32_t N, uint32_t M, std::floating_point T>
[[nodiscard]] Bitset<N * M>
unequal(const detail::mat<N, M, T> &lhs,
        const detail::mat<N, M, T> &rhs, T epsilon) noexcept {
  Bitset<N * M> result;
  for (uint32_t column = 0; column < M; ++column) {
    for (uint32_t row = 0; row < N; ++row) {
      if (std::abs(lhs.at(row, column) - rhs.at(row, column)) > epsilon) {
        result.set(column * N + row);
      }
    }
  }
  return result;
}

template <uint32_t N, uint32_t M, typename T>
  requires std::is_arithmetic_v<T>
[[nodiscard]] detail::mat<N, M, T>
abs(const detail::mat<N, M, T> &value) noexcept {
  detail::mat<N, M, T> result{};
  for (uint32_t column = 0; column < M; ++column) {
    for (uint32_t row = 0; row < N; ++row) {
      if constexpr (std::is_unsigned_v<T>)
        result.at(row, column) = value.at(row, column);
      else
        result.at(row, column) = std::abs(value.at(row, column));
    }
  }
  return result;
}

template <uint32_t N, uint32_t M, typename T>
  requires std::is_arithmetic_v<T>
[[nodiscard]] constexpr detail::mat<N, M, T>
elementwise_mul(const detail::mat<N, M, T> &lhs,
                const detail::mat<N, M, T> &rhs) noexcept {
  detail::mat<N, M, T> result{};
  for (uint32_t column = 0; column < M; ++column) {
    for (uint32_t row = 0; row < N; ++row) {
      result.at(row, column) = lhs.at(row, column) * rhs.at(row, column);
    }
  }
  return result;
}

template <uint32_t N, uint32_t M, typename T>
  requires std::is_arithmetic_v<T>
[[nodiscard]] detail::mat<N, M, T>
lerp(const detail::mat<N, M, T> &lhs,
     const detail::mat<N, M, T> &rhs, T t) noexcept {
  detail::mat<N, M, T> result{};
  for (uint32_t column = 0; column < M; ++column) {
    for (uint32_t row = 0; row < N; ++row) {
      result.at(row, column) =
          lhs.at(row, column) + (rhs.at(row, column) - lhs.at(row, column)) * t;
    }
  }
  return result;
}

namespace detail {

template <uint32_t N, uint32_t M, typename T>
[[nodiscard]] detail::mat<N, M, T>
operator+(const detail::mat<N, M, T> &lhs,
          const detail::mat<N, M, T> &rhs) noexcept {
  detail::mat<N, M, T> out{};
  for (uint32_t column = 0; column < M; ++column) {
    out.data[column] = lhs.data[column] + rhs.data[column];
  }
  return out;
}

template <uint32_t N, uint32_t M, typename T>
[[nodiscard]] detail::mat<N, M, T>
operator-(const detail::mat<N, M, T> &lhs,
          const detail::mat<N, M, T> &rhs) noexcept {
  detail::mat<N, M, T> out{};
  for (uint32_t column = 0; column < M; ++column) {
    out.data[column] = lhs.data[column] - rhs.data[column];
  }
  return out;
}

template <uint32_t N, uint32_t M, typename T>
[[nodiscard]] detail::mat<N, M, T>
operator+(const detail::mat<N, M, T> &lhs, T rhs) noexcept {
  detail::mat<N, M, T> result = lhs;
  result += rhs;
  return result;
}

template <uint32_t N, uint32_t M, typename T>
[[nodiscard]] detail::mat<N, M, T>
operator-(const detail::mat<N, M, T> &lhs, T rhs) noexcept {
  detail::mat<N, M, T> result = lhs;
  result -= rhs;
  return result;
}

template <uint32_t N, uint32_t M, typename T>
[[nodiscard]] detail::mat<N, M, T>
operator+(T lhs, const detail::mat<N, M, T> &rhs) noexcept {
  return rhs + lhs;
}

template <uint32_t N, uint32_t M, typename T>
[[nodiscard]] detail::mat<N, M, T>
operator-(T lhs, const detail::mat<N, M, T> &rhs) noexcept {
  detail::mat<N, M, T> result{};
  for (uint32_t column = 0; column < M; ++column) {
    for (uint32_t row = 0; row < N; ++row) {
      result.at(row, column) = lhs - rhs.at(row, column);
    }
  }
  return result;
}

template <uint32_t N, uint32_t K, uint32_t M, typename T>
[[nodiscard]] detail::mat<N, M, T>
operator*(const detail::mat<N, K, T> &lhs,
          const detail::mat<K, M, T> &rhs) noexcept {
  detail::mat<N, M, T> out{};
  for (uint32_t column = 0; column < M; ++column) {
    for (uint32_t row = 0; row < N; ++row) {
      for (uint32_t k = 0; k < K; ++k) {
        out.at(row, column) +=
            lhs.at(row, k) * rhs.at(k, column);
      }
    }
  }
  return out;
}

template <uint32_t N, uint32_t M, typename T>
[[nodiscard]] detail::vec<N, T>
operator*(const detail::mat<N, M, T> &lhs,
          const detail::vec<M, T> &rhs) noexcept {
  detail::vec<N, T> out{};
  for (uint32_t column = 0; column < M; ++column) {
    out += lhs.data[column] * rhs[column];
  }
  return out;
}

template <uint32_t N, uint32_t M, typename T>
[[nodiscard]] detail::mat<N, M, T> operator*(const detail::mat<N, M, T> &lhs,
                                             const T &rhs) noexcept {
  detail::mat<N, M, T> out{};
  for (uint32_t column = 0; column < M; ++column) {
    out.data[column] = lhs.data[column] * rhs;
  }
  return out;
}

template <uint32_t N, uint32_t M, typename T>
[[nodiscard]] detail::mat<N, M, T> operator*(T lhs,
                                             const detail::mat<N, M, T> &rhs) noexcept {
  return rhs * lhs;
}

template <uint32_t N, uint32_t M, typename T>
[[nodiscard]] detail::mat<N, M, T> operator/(const detail::mat<N, M, T> &lhs,
                                             T rhs) noexcept {
  detail::mat<N, M, T> result = lhs;
  result /= rhs;
  return result;
}

template <uint32_t N, uint32_t M, typename T>
[[nodiscard]] detail::mat<N, M, T> operator/(T lhs,
                                             const detail::mat<N, M, T> &rhs) noexcept {
  detail::mat<N, M, T> result{};
  for (uint32_t column = 0; column < M; ++column) {
    for (uint32_t row = 0; row < N; ++row) {
      result.at(row, column) = lhs / rhs.at(row, column);
    }
  }
  return result;
}

} // namespace detail

template <uint32_t N, uint32_t M, typename T>
[[nodiscard]] detail::mat<M, N, T>
transpose(const detail::mat<N, M, T> &mat) noexcept {
  detail::mat<M, N, T> out;
  for (uint32_t column = 0; column < M; ++column) {
    for (uint32_t row = 0; row < N; ++row) {
      out.at(column, row) = mat.at(row, column);
    }
  }
  return out;
}

template <uint32_t N, std::floating_point T>
[[nodiscard]] T det(detail::mat<N, N, T> a) noexcept {
  T det{1};
  for (uint32_t i = 0; i < N; ++i) {
    uint32_t pivot = i;
    T max_abs = std::abs(a.at(i, i));
    for (uint32_t j = i + 1; j < N; ++j) {
      T v = std::abs(a.at(j, i));
      if (v > max_abs) {
        max_abs = v;
        pivot = j;
      }
    }
    if (max_abs == T{0}) {
      return T{0};
    }
    if (pivot != i) {
      for (uint32_t j = i; j < N; ++j) {
        std::swap(a.at(i, j), a.at(pivot, j));
      }
      det = -det;
    }

    const T diagonal = a.at(i, i);
    det *= diagonal;
    for (uint32_t j = i + 1; j < N; ++j) {
      a.at(j, i) /= diagonal;
    }
    for (uint32_t k = i + 1; k < N; ++k) {
      const T v = a.at(i, k);
      for (uint32_t j = i + 1; j < N; ++j) {
        a.at(j, k) -= a.at(j, i) * v;
      }
    }
  }
  return det;
}

template <std::floating_point T>
[[nodiscard]] T det(const detail::mat<1, 1, T> &a) noexcept {
  return a.at(0, 0);
}

template <std::floating_point T>
[[nodiscard]] T det(const detail::mat<2, 2, T> &a) noexcept {
  return a.at(0, 0) * a.at(1, 1) - a.at(0, 1) * a.at(1, 0);
}

template <std::floating_point T>
[[nodiscard]] T det(const detail::mat<3, 3, T> &a) noexcept {
  const T a00 = a.at(0, 0), a01 = a.at(0, 1), a02 = a.at(0, 2);
  const T a10 = a.at(1, 0), a11 = a.at(1, 1), a12 = a.at(1, 2);
  const T a20 = a.at(2, 0), a21 = a.at(2, 1), a22 = a.at(2, 2);

  return a00 * (a11 * a22 - a12 * a21) - a01 * (a10 * a22 - a12 * a20) +
         a02 * (a10 * a21 - a11 * a20);
}

template <std::floating_point T>
[[nodiscard]] T det(const detail::mat<4, 4, T> &a) noexcept {
  const T s0 = a.at(0, 0) * a.at(1, 1) - a.at(1, 0) * a.at(0, 1);
  const T s1 = a.at(0, 0) * a.at(1, 2) - a.at(1, 0) * a.at(0, 2);
  const T s2 = a.at(0, 0) * a.at(1, 3) - a.at(1, 0) * a.at(0, 3);
  const T s3 = a.at(0, 1) * a.at(1, 2) - a.at(1, 1) * a.at(0, 2);
  const T s4 = a.at(0, 1) * a.at(1, 3) - a.at(1, 1) * a.at(0, 3);
  const T s5 = a.at(0, 2) * a.at(1, 3) - a.at(1, 2) * a.at(0, 3);

  const T c0 = a.at(2, 0) * a.at(3, 1) - a.at(3, 0) * a.at(2, 1);
  const T c1 = a.at(2, 0) * a.at(3, 2) - a.at(3, 0) * a.at(2, 2);
  const T c2 = a.at(2, 0) * a.at(3, 3) - a.at(3, 0) * a.at(2, 3);
  const T c3 = a.at(2, 1) * a.at(3, 2) - a.at(3, 1) * a.at(2, 2);
  const T c4 = a.at(2, 1) * a.at(3, 3) - a.at(3, 1) * a.at(2, 3);
  const T c5 = a.at(2, 2) * a.at(3, 3) - a.at(3, 2) * a.at(2, 3);

  return s0 * c5 - s1 * c4 + s2 * c3 + s3 * c2 - s4 * c1 + s5 * c0;
}

template <uint32_t N, std::floating_point T>
[[nodiscard]] detail::mat<N, N, T> inverse(detail::mat<N, N, T> a) noexcept {
  static_assert(N > 0);
  detail::mat<N, N, T> inv;
  for (uint32_t k = 0; k < N; ++k) {
    for (uint32_t j = 0; j < N; ++j) {
      inv.at(j, k) = T(j == k);
    }
  }
  for (uint32_t i = 0; i < N; ++i) {
    uint32_t pivot = i;
    T max_abs = std::abs(a.at(i, i));
    for (uint32_t j = i + 1; j < N; ++j) {
      T v = std::abs(a.at(j, i));
      if (v > max_abs) {
        max_abs = v;
        pivot = j;
      }
    }
    if (max_abs == T{0}) {
      const T nan = std::numeric_limits<T>::quiet_NaN();
      for (uint32_t k = 0; k < N; ++k) {
        for (uint32_t j = 0; j < N; ++j) {
          inv.at(j, k) = nan;
        }
      }
      return inv;
    }
    if (pivot != i) {
      for (uint32_t k = i; k < N; ++k) {
        std::swap(a.at(i, k), a.at(pivot, k));
      }
      for (uint32_t k = 0; k < N; ++k) {
        std::swap(inv.at(i, k), inv.at(pivot, k));
      }
    }
    const T inv_diag = T{1} / a.at(i, i);
    for (uint32_t k = i + 1; k < N; ++k) {
      const T v = a.at(i, k) * inv_diag;
      a.at(i, k) = v;
      for (uint32_t j = 0; j < N; ++j) {
        if (j != i) {
          a.at(j, k) -= a.at(j, i) * v;
        }
      }
    }
    for (uint32_t k = 0; k < N; ++k) {
      const T v = inv.at(i, k) * inv_diag;
      inv.at(i, k) = v;

      for (uint32_t j = 0; j < N; ++j) {
        if (j != i) {
          inv.at(j, k) -= a.at(j, i) * v;
        }
      }
    }
  }
  return inv;
}

template <std::floating_point T>
[[nodiscard]] detail::mat<1, 1, T>
inverse(const detail::mat<1, 1, T> &a) noexcept {
  const T d = a.at(0, 0);
  if (d == T{0}) {
    return detail::nan_matrix<1, T>();
  }
  detail::mat<1, 1, T> r;
  r.at(0, 0) = T{1} / d;
  return r;
}

template <std::floating_point T>
[[nodiscard]] detail::mat<2, 2, T>
inverse(const detail::mat<2, 2, T> &a) noexcept {
  const T a00 = a.at(0, 0), a01 = a.at(0, 1);
  const T a10 = a.at(1, 0), a11 = a.at(1, 1);
  const T d = a00 * a11 - a01 * a10;
  if (d == T{0}) {
    return detail::nan_matrix<2, T>();
  }
  const T s = T{1} / d;
  detail::mat<2, 2, T> r;
  r.at(0, 0) = a11 * s;
  r.at(0, 1) = -a01 * s;
  r.at(1, 0) = -a10 * s;
  r.at(1, 1) = a00 * s;
  return r;
}

template <std::floating_point T>
[[nodiscard]] detail::mat<3, 3, T>
inverse(const detail::mat<3, 3, T> &a) noexcept {
  const T a00 = a.at(0, 0), a01 = a.at(0, 1), a02 = a.at(0, 2);
  const T a10 = a.at(1, 0), a11 = a.at(1, 1), a12 = a.at(1, 2);
  const T a20 = a.at(2, 0), a21 = a.at(2, 1), a22 = a.at(2, 2);
  const T c00 = a11 * a22 - a12 * a21;
  const T c01 = a12 * a20 - a10 * a22;
  const T c02 = a10 * a21 - a11 * a20;
  const T d = a00 * c00 + a01 * c01 + a02 * c02;
  if (d == T{0}) {
    return detail::nan_matrix<3, T>();
  }
  const T s = T{1} / d;
  detail::mat<3, 3, T> r;

  r.at(0, 0) = c00 * s;
  r.at(0, 1) = (a02 * a21 - a01 * a22) * s;
  r.at(0, 2) = (a01 * a12 - a02 * a11) * s;

  r.at(1, 0) = c01 * s;
  r.at(1, 1) = (a00 * a22 - a02 * a20) * s;
  r.at(1, 2) = (a02 * a10 - a00 * a12) * s;

  r.at(2, 0) = c02 * s;
  r.at(2, 1) = (a01 * a20 - a00 * a21) * s;
  r.at(2, 2) = (a00 * a11 - a01 * a10) * s;

  return r;
}

template <std::floating_point T>
[[nodiscard]] detail::mat<4, 4, T>
inverse(const detail::mat<4, 4, T> &a) noexcept {
  const T a00 = a.at(0, 0), a01 = a.at(0, 1);
  const T a02 = a.at(0, 2), a03 = a.at(0, 3);
  const T a10 = a.at(1, 0), a11 = a.at(1, 1);
  const T a12 = a.at(1, 2), a13 = a.at(1, 3);
  const T a20 = a.at(2, 0), a21 = a.at(2, 1);
  const T a22 = a.at(2, 2), a23 = a.at(2, 3);
  const T a30 = a.at(3, 0), a31 = a.at(3, 1);
  const T a32 = a.at(3, 2), a33 = a.at(3, 3);

  const T s0 = a00 * a11 - a10 * a01;
  const T s1 = a00 * a12 - a10 * a02;
  const T s2 = a00 * a13 - a10 * a03;
  const T s3 = a01 * a12 - a11 * a02;
  const T s4 = a01 * a13 - a11 * a03;
  const T s5 = a02 * a13 - a12 * a03;

  const T c0 = a20 * a31 - a30 * a21;
  const T c1 = a20 * a32 - a30 * a22;
  const T c2 = a20 * a33 - a30 * a23;
  const T c3 = a21 * a32 - a31 * a22;
  const T c4 = a21 * a33 - a31 * a23;
  const T c5 = a22 * a33 - a32 * a23;

  const T d = s0 * c5 - s1 * c4 + s2 * c3 + s3 * c2 - s4 * c1 + s5 * c0;
  if (d == T{0}) {
    return detail::nan_matrix<4, T>();
  }

  const T s = T{1} / d;
  detail::mat<4, 4, T> r;

  r.at(0, 0) = (a11 * c5 - a12 * c4 + a13 * c3) * s;
  r.at(0, 1) = (-a01 * c5 + a02 * c4 - a03 * c3) * s;
  r.at(0, 2) = (a31 * s5 - a32 * s4 + a33 * s3) * s;
  r.at(0, 3) = (-a21 * s5 + a22 * s4 - a23 * s3) * s;

  r.at(1, 0) = (-a10 * c5 + a12 * c2 - a13 * c1) * s;
  r.at(1, 1) = (a00 * c5 - a02 * c2 + a03 * c1) * s;
  r.at(1, 2) = (-a30 * s5 + a32 * s2 - a33 * s1) * s;
  r.at(1, 3) = (a20 * s5 - a22 * s2 + a23 * s1) * s;

  r.at(2, 0) = (a10 * c4 - a11 * c2 + a13 * c0) * s;
  r.at(2, 1) = (-a00 * c4 + a01 * c2 - a03 * c0) * s;
  r.at(2, 2) = (a30 * s4 - a31 * s2 + a33 * s0) * s;
  r.at(2, 3) = (-a20 * s4 + a21 * s2 - a23 * s0) * s;

  r.at(3, 0) = (-a10 * c3 + a11 * c1 - a12 * c0) * s;
  r.at(3, 1) = (a00 * c3 - a01 * c1 + a02 * c0) * s;
  r.at(3, 2) = (-a30 * s3 + a31 * s1 - a32 * s0) * s;
  r.at(3, 3) = (a20 * s3 - a21 * s1 + a22 * s0) * s;

  return r;
}

template <uint32_t N, std::floating_point T>
[[nodiscard]] detail::mat<N, N, T>
inverse_transpose(const detail::mat<N, N, T> &input) noexcept {
  static_assert(N > 0);

  detail::mat<N, N, T> a = input;
  detail::mat<N, N, T> result = detail::mat<N, N, T>::identity();
  const auto at = [&a](uint32_t row, uint32_t column) -> T & {
    return a.at(column, row);
  };

  for (uint32_t i = 0; i < N; ++i) {
    uint32_t pivot = i;
    T max_abs = std::abs(at(i, i));
    for (uint32_t j = i + 1; j < N; ++j) {
      const T value = std::abs(at(j, i));
      if (value > max_abs) {
        max_abs = value;
        pivot = j;
      }
    }

    if (max_abs == T{0}) {
      const T nan = std::numeric_limits<T>::quiet_NaN();
      for (uint32_t row = 0; row < N; ++row) {
        for (uint32_t column = 0; column < N; ++column) {
          result.at(row, column) = nan;
        }
      }
      return result;
    }

    if (pivot != i) {
      for (uint32_t column = 0; column < N; ++column) {
        std::swap(at(i, column), at(pivot, column));
        std::swap(result.at(i, column), result.at(pivot, column));
      }
    }

    const T inverse_diagonal = T{1} / at(i, i);
    for (uint32_t column = i + 1; column < N; ++column) {
      const T value = at(i, column) * inverse_diagonal;
      at(i, column) = value;
      for (uint32_t row = 0; row < N; ++row) {
        if (row != i) {
          at(row, column) -= at(row, i) * value;
        }
      }
    }
    for (uint32_t column = 0; column < N; ++column) {
      const T value = result.at(i, column) * inverse_diagonal;
      result.at(i, column) = value;
      for (uint32_t row = 0; row < N; ++row) {
        if (row != i) {
          result.at(row, column) -= at(row, i) * value;
        }
      }
    }
  }

  return result;
}

template <std::floating_point T>
[[nodiscard]] detail::mat<4, 4, T>
affine_inverse(const detail::mat<4, 4, T> &input) noexcept {
  const T a00 = input.at(0, 0), a01 = input.at(0, 1);
  const T a02 = input.at(0, 2), a10 = input.at(1, 0);
  const T a11 = input.at(1, 1), a12 = input.at(1, 2);
  const T a20 = input.at(2, 0), a21 = input.at(2, 1);
  const T a22 = input.at(2, 2);

  const T c00 = a11 * a22 - a12 * a21;
  const T c01 = a02 * a21 - a01 * a22;
  const T c02 = a01 * a12 - a02 * a11;
  const T c10 = a12 * a20 - a10 * a22;
  const T c11 = a00 * a22 - a02 * a20;
  const T c12 = a02 * a10 - a00 * a12;
  const T c20 = a10 * a21 - a11 * a20;
  const T c21 = a01 * a20 - a00 * a21;
  const T c22 = a00 * a11 - a01 * a10;
  const T determinant = a00 * c00 + a01 * c10 + a02 * c20;

  if (determinant == T{0}) {
    return detail::nan_matrix<4, T>();
  }

  const T scale = T{1} / determinant;
  const T i00 = c00 * scale, i01 = c01 * scale, i02 = c02 * scale;
  const T i10 = c10 * scale, i11 = c11 * scale, i12 = c12 * scale;
  const T i20 = c20 * scale, i21 = c21 * scale, i22 = c22 * scale;
  const T tx = input.at(0, 3), ty = input.at(1, 3), tz = input.at(2, 3);

  detail::mat<4, 4, T> result = detail::mat<4, 4, T>::identity();
  result.at(0, 0) = i00;
  result.at(0, 1) = i01;
  result.at(0, 2) = i02;
  result.at(1, 0) = i10;
  result.at(1, 1) = i11;
  result.at(1, 2) = i12;
  result.at(2, 0) = i20;
  result.at(2, 1) = i21;
  result.at(2, 2) = i22;
  result.at(0, 3) = -(i00 * tx + i01 * ty + i02 * tz);
  result.at(1, 3) = -(i10 * tx + i11 * ty + i12 * tz);
  result.at(2, 3) = -(i20 * tx + i21 * ty + i22 * tz);
  return result;
}

template <uint32_t N, std::floating_point T>
[[nodiscard]] detail::mat<N, N, T>
adjugate(const detail::mat<N, N, T> &input) noexcept {
  static_assert(N > 0);
  detail::mat<N, N, T> result{};

  if constexpr (N == 1) {
    result.at(0, 0) = T{1};
  } else {
    for (uint32_t row = 0; row < N; ++row) {
      for (uint32_t column = 0; column < N; ++column) {
        detail::mat<N - 1, N - 1, T> minor{};
        uint32_t minor_row = 0;
        for (uint32_t input_row = 0; input_row < N; ++input_row) {
          if (input_row == column) {
            continue;
          }
          uint32_t minor_column = 0;
          for (uint32_t input_column = 0; input_column < N; ++input_column) {
            if (input_column == row) {
              continue;
            }
            minor.at(minor_row, minor_column) =
                input.at(input_row, input_column);
            ++minor_column;
          }
          ++minor_row;
        }

        const T cofactor = det(minor);
        result.at(row, column) = ((row + column) % 2 == 0) ? cofactor
                                                            : -cofactor;
      }
    }
  }
  return result;
}

using mat2x2 = detail::mat<2, 2, float>;
using mat2x3 = detail::mat<3, 2, float>;
using mat2x4 = detail::mat<4, 2, float>;
using mat3x2 = detail::mat<2, 3, float>;
using mat3x3 = detail::mat<3, 3, float>;
using mat3x4 = detail::mat<4, 3, float>;
using mat4x2 = detail::mat<2, 4, float>;
using mat4x3 = detail::mat<3, 4, float>;
using mat4x4 = detail::mat<4, 4, float>;

using dmat2x2 = detail::mat<2, 2, double>;
using dmat2x3 = detail::mat<3, 2, double>;
using dmat2x4 = detail::mat<4, 2, double>;
using dmat3x2 = detail::mat<2, 3, double>;
using dmat3x3 = detail::mat<3, 3, double>;
using dmat3x4 = detail::mat<4, 3, double>;
using dmat4x2 = detail::mat<2, 4, double>;
using dmat4x3 = detail::mat<3, 4, double>;
using dmat4x4 = detail::mat<4, 4, double>;

using imat2x2 = detail::mat<2, 2, int32_t>;
using imat2x3 = detail::mat<3, 2, int32_t>;
using imat2x4 = detail::mat<4, 2, int32_t>;
using imat3x2 = detail::mat<2, 3, int32_t>;
using imat3x3 = detail::mat<3, 3, int32_t>;
using imat3x4 = detail::mat<4, 3, int32_t>;
using imat4x2 = detail::mat<2, 4, int32_t>;
using imat4x3 = detail::mat<3, 4, int32_t>;
using imat4x4 = detail::mat<4, 4, int32_t>;

using umat2x2 = detail::mat<2, 2, uint32_t>;
using umat2x3 = detail::mat<3, 2, uint32_t>;
using umat2x4 = detail::mat<4, 2, uint32_t>;
using umat3x2 = detail::mat<2, 3, uint32_t>;
using umat3x3 = detail::mat<3, 3, uint32_t>;
using umat3x4 = detail::mat<4, 3, uint32_t>;
using umat4x2 = detail::mat<2, 4, uint32_t>;
using umat4x3 = detail::mat<3, 4, uint32_t>;
using umat4x4 = detail::mat<4, 4, uint32_t>;

using mat2 = mat2x2;
using mat3 = mat3x3;
using mat4 = mat4x4;
using dmat2 = dmat2x2;
using dmat3 = dmat3x3;
using dmat4 = dmat4x4;

} // namespace strobe
