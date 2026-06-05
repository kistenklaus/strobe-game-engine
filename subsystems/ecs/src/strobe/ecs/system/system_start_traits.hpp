#pragma once
#include "strobe/ecs/object/object_function.hpp"
#include "strobe/ecs/resource.hpp"
#include "strobe/ecs/system/stateful_system.hpp"
#include <type_traits>

namespace strobe::ecs {

template <typename S, typename = void> struct system_start_function {
  static constexpr bool exists = false;
  using type = void;
};

template <typename S>
struct system_start_function<
    S, std::void_t<decltype(&std::remove_cvref_t<S>::start)>> {
  using system_type = std::remove_cvref_t<S>;
  static constexpr bool exists = true;
  using type = decltype(&system_type::start);
};

template <typename S>
struct system_start_function<stateful_system<S>, void>
    : system_start_function<std::remove_cvref_t<S>> {};

template <typename S>
using system_start_function_t = typename system_start_function<S>::type;

template <typename S>
inline constexpr bool system_start_exists_v = system_start_function<S>::exists;

template <typename S>
struct system_start_traits
    : object_function_traits<system_start_function_t<S>> {
  using system_type = std::remove_cvref_t<S>;
  using base = object_function_traits<system_start_function_t<S>>;
  using resource_arguments = types_filter_t<typename base::argument_types,
                                            details::is_resource_object>;
  using lifetime_arguments = types_union_t<resource_arguments>;
};

} // namespace strobe::ecs
