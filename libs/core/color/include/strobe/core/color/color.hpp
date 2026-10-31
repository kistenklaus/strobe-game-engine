#pragma once

#include "strobe/core/lina/vec.hpp"
#include <cmath>
#include <type_traits>

namespace strobe {

template <typename T>
  requires std::is_arithmetic_v<T>
using color = strobe::detail::vec<3, T>;

template <typename T>
  requires std::is_arithmetic_v<T>
color<T> linear_srgb_to_nonlinear_srgb(const color<T> &linear_rgb) {
  color<T> result;
  for (int i = 0; i < 3; ++i) {
    const T x = linear_rgb[i];
    result[i] = x <= T(0.0031308)
                    ? T(12.92) * x
                    : T(1.055) * std::pow(x, T(1) / T(2.4)) - T(0.055);
  }
  return result;
}

template <typename T>
  requires std::is_floating_point_v<T>
color<T> nonlinear_srgb_to_linear_srgb(const color<T> &srgb) {
  color<T> result;
  for (int i = 0; i < 3; ++i) {
    const T x = srgb[i];
    result[i] = x <= T(0.04045) ? x / T(12.92)
                                : std::pow((x + T(0.055)) / T(1.055), T(2.4));
  }
  return result;
}

} // namespace strobe
