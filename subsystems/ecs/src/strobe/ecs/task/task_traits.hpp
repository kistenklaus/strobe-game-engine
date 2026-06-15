#pragma once

#include "strobe/core/type_traits/call_operator_traits.hpp"
#include "strobe/ecs/object/object_function.hpp"
#include <type_traits>

namespace strobe::ecs {

template <typename Fn>
concept task_fn =
    has_unique_call_operator<std::remove_cvref_t<Fn>> &&
    std::is_empty_v<std::remove_cvref_t<Fn>> &&
    std::is_trivially_default_constructible_v<std::remove_cvref_t<Fn>> &&
    std::is_trivially_destructible_v<std::remove_cvref_t<Fn>> &&
    call_operator_is_noexcept_v<std::remove_cvref_t<Fn>> &&
    !call_operator_is_volatile_v<std::remove_cvref_t<Fn>> &&
    !call_operator_is_ref_qualified_v<std::remove_cvref_t<Fn>>;

template <task_fn Fn>
struct task_traits
    : object_function_traits<
          strobe::call_operator_function_pointer_t<std::remove_cvref_t<Fn>>> {
  using task_type = std::remove_cvref_t<Fn>;
  using function_pointer = strobe::call_operator_function_pointer_t<task_type>;
  using base = object_function_traits<function_pointer>;
  using return_type = typename base::return_type;
  using argument_types = typename base::argument_types;

  using resource_arguments = types_filter_t<typename base::argument_types,
                                            details::is_resource_object>;
  using lifetime_arguments = types_union_t<resource_arguments>;

  static constexpr bool is_const = strobe::call_operator_is_const_v<task_type>;
  static constexpr bool is_noexcept =
      strobe::call_operator_is_noexcept_v<task_type>;
};

} // namespace strobe::ecs
