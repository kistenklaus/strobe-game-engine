#pragma once

#include "strobe/ecs/object/object_function.hpp"
#include "strobe/ecs/resource.hpp"
#include "strobe/ecs/system/stateful_system.hpp"
#include <type_traits>

namespace strobe::ecs {

template <typename S, typename = void> struct system_teardown_function {
  static constexpr bool exists = false;
  using type = void;
};

template <typename S>
struct system_teardown_function<
    S, std::void_t<decltype(&std::remove_cvref_t<S>::teardown)>> {
  using system_type = std::remove_cvref_t<S>;
  static constexpr bool exists = true;
  using type = decltype(&system_type::teardown);
};

template <typename S>
struct system_teardown_function<stateful_system<S>, void>
    : system_teardown_function<std::remove_cvref_t<S>> {};


template <typename S>
using system_teardown_function_t = typename system_teardown_function<S>::type;

template <typename S>
inline constexpr bool system_teardown_exists_v =
    system_teardown_function<S>::exists;

template <typename S>
struct system_teardown_traits
    : object_function_traits<system_teardown_function_t<S>> {
  using system_type = std::remove_cvref_t<S>;
  using base = object_function_traits<system_teardown_function_t<S>>;

  using resource_arguments = types_filter_t<typename base::argument_types,
                                            details::is_resource_object>;
  using lifetime_arguments = types_union_t<resource_arguments>;
};

} // namespace strobe::ecs
