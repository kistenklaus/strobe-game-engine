#pragma once

#include "strobe/core/type_traits/types.hpp"
#include <type_traits>

namespace strobe {

namespace ecs::details {

template <typename T>
struct IsReadQueryAccess
    : std::bool_constant<std::is_const_v<std::remove_reference_t<T>>> {};
template <typename T>
struct IsWriteQueryAccess
    : std::bool_constant<!std::is_const_v<std::remove_reference_t<T>>> {};

} // namespace ecs::details

template <typename... Ts>
  requires(std::is_reference_v<Ts> && ...)
struct Query {
public:
  using access_types = Types<Ts...>;
  using component_types = types_transform_t<access_types, std::remove_cvref>;

  using read_components = types_transform_t<
      types_filter_t<access_types, ecs::details::IsReadQueryAccess>,
      std::remove_cvref>;

  using write_components = types_transform_t<
      types_filter_t<access_types, ecs::details::IsWriteQueryAccess>,
      std::remove_cvref>;

private:
};

namespace ecs::details {
template <typename T> struct IsQuery : std::false_type {};
template <typename... Ts> struct IsQuery<Query<Ts...>> : std::true_type {};
} // namespace ecs::details

template <typename T>
concept query = ecs::details::IsQuery<std::remove_cvref_t<T>>::value;

} // namespace strobe
