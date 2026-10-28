#pragma once

#include "strobe/flow/flow.hpp"
#include "strobe/flow/inttypes.hpp"

#include <strobe/core/lina/vec.hpp>

#include <concepts>
#include <cstddef>
#include <memory>
#include <string_view>
#include <type_traits>
#include <utility>

namespace strobe::flow {

namespace detail {

template <typename T> struct vector_component {
  using type = Resource<T>;

  static type make(Resource<T> resource) noexcept { return resource; }
};

template <std::integral T> struct vector_component<T> {
  using type = integer_resource<T>;

  static type make(Resource<T> resource) noexcept { return type{resource}; }
};

template <typename T>
using vector_component_t = typename vector_component<T>::type;

template <std::size_t Index> constexpr std::string_view component_name() {
  static_assert(Index < 4);
  if constexpr (Index == 0) {
    return "x";
  } else if constexpr (Index == 1) {
    return "y";
  } else if constexpr (Index == 2) {
    return "z";
  } else {
    return "w";
  }
}

template <std::size_t N, typename T>
  requires std::is_arithmetic_v<T> && (N >= 2) && (N <= 4)
class vec {
  using runtime_type = strobe::details::vec<N, T>;
  using component_type = vector_component_t<T>;

public:
  template <typename... Values>
    requires(sizeof...(Values) == N) && (std::convertible_to<Values, T> && ...)
  vec(Values &&...values) noexcept : m_runtime(flow::resource<runtime_type>()) {
    flow::construct(m_runtime, runtime_type{static_cast<T>(
                                   std::forward<Values>(values))...});
  }

  template <typename... Components>
    requires(sizeof...(Components) == N) &&
            (std::same_as<std::remove_cvref_t<Components>, component_type> &&
             ...)
  vec(const Components &...components) noexcept
      : m_runtime(flow::resource<runtime_type>()) {
    flow::construct(
        m_runtime, flow::read(components.runtime())...,
        [](runtime_type *output, Lifetime, const auto &...values) noexcept {
          std::construct_at(output, runtime_type{values...});
        });
  }

  vec(Resource<runtime_type> runtime) noexcept : m_runtime(runtime) {}

  Resource<runtime_type> runtime() const noexcept { return m_runtime; }

  component_type x() const noexcept { return component<0>(); }

  component_type y() const noexcept { return component<1>(); }

  component_type z() const noexcept
    requires(N >= 3)
  {
    return component<2>();
  }

  component_type w() const noexcept
    requires(N >= 4)
  {
    return component<3>();
  }

  friend vec operator+(const vec &lhs, const vec &rhs) noexcept {
    return binary(
        lhs, rhs,
        [](const runtime_type &left, const runtime_type &right) noexcept {
          return left + right;
        });
  }

  friend vec operator-(const vec &lhs, const vec &rhs) noexcept {
    return binary(
        lhs, rhs,
        [](const runtime_type &left, const runtime_type &right) noexcept {
          return left - right;
        });
  }

  friend vec operator*(const vec &lhs, const vec &rhs) noexcept {
    return binary(
        lhs, rhs,
        [](const runtime_type &left, const runtime_type &right) noexcept {
          return left * right;
        });
  }

  friend vec operator/(const vec &lhs, const vec &rhs) noexcept {
    return binary(
        lhs, rhs,
        [](const runtime_type &left, const runtime_type &right) noexcept {
          return left / right;
        });
  }

  friend vec operator*(const vec &value, T scalar) noexcept {
    return scalar_binary(value, scalar,
                         [](const runtime_type &input, T factor) noexcept {
                           return input * factor;
                         });
  }

  friend vec operator*(T scalar, const vec &value) noexcept {
    return value * scalar;
  }

  friend vec operator/(const vec &value, T scalar) noexcept {
    return scalar_binary(value, scalar,
                         [](const runtime_type &input, T divisor) noexcept {
                           return input / divisor;
                         });
  }

private:
  template <std::size_t Index>
  component_type component() const noexcept
    requires(Index < N)
  {
    auto projection = flow::project<T>(
        m_runtime, component_name<Index>(), ProjectionKind::disjoint,
        [](runtime_type &value) noexcept -> T * { return &value[Index]; });
    return vector_component<T>::make(projection);
  }

  template <typename Operation>
  static vec binary(const vec &lhs, const vec &rhs,
                    Operation operation) noexcept {
    auto result = flow::resource<runtime_type>();
    flow::construct(result, flow::read(lhs.m_runtime),
                    flow::read(rhs.m_runtime),
                    [operation = std::move(operation)](
                        runtime_type *output, const runtime_type &left,
                        const runtime_type &right) noexcept {
                      std::construct_at(output, operation(left, right));
                    });
    return result;
  }

  template <typename Operation>
  static vec scalar_binary(const vec &value, T scalar,
                           Operation operation) noexcept {
    auto result = flow::resource<runtime_type>();
    flow::construct(
        result, flow::read(value.m_runtime),
        [operation = std::move(operation),
         scalar](runtime_type *output, const runtime_type &input) noexcept {
          std::construct_at(output, operation(input, scalar));
        });
    return result;
  }

  Resource<runtime_type> m_runtime;
};

} // namespace detail

using vec2 = detail::vec<2, float>;
using vec3 = detail::vec<3, float>;
using vec4 = detail::vec<4, float>;

using dvec2 = detail::vec<2, double>;
using dvec3 = detail::vec<3, double>;
using dvec4 = detail::vec<4, double>;

using uvec2 = detail::vec<2, unsigned int>;
using uvec3 = detail::vec<3, unsigned int>;
using uvec4 = detail::vec<4, unsigned int>;

using ivec2 = detail::vec<2, int>;
using ivec3 = detail::vec<3, int>;
using ivec4 = detail::vec<4, int>;

} // namespace strobe::flow
