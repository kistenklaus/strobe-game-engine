#pragma once

#include "strobe/core/type_traits/types.hpp"
#include "strobe/ecs/object/object_function.hpp"
#include "strobe/ecs/resource.hpp"
#include "strobe/ecs/system/stateful_system.hpp"
#include <type_traits>

namespace strobe::ecs {

template <typename S, typename = void> struct system_setup_function {
  static constexpr bool exists = false;
  using type = void;
};

template <typename S>
struct system_setup_function<
    S, std::void_t<decltype(&std::remove_cvref_t<S>::setup)>> {
  using system_type = std::remove_cvref_t<S>;
  static constexpr bool exists = true;
  using type = decltype(&system_type::setup);
};

template <typename S>
struct system_setup_function<stateful_system<S>, void>
    : system_setup_function<std::remove_cvref_t<S>> {};

template <typename S>
using system_setup_function_t = typename system_setup_function<S>::type;

template <typename S>
inline constexpr bool system_setup_exists_v = system_setup_function<S>::exists;

template <typename S>
struct system_setup_traits
    : object_function_traits<system_setup_function_t<S>> {
  using system_type = std::remove_cvref_t<S>;
  using base = object_function_traits<system_setup_function_t<S>>;

  using resource_arguments = types_filter_t<typename base::argument_types,
                                            details::is_resource_object>;

  using lifetime_arguments = types_union_t<resource_arguments>;
};

} // namespace strobe::ecs
