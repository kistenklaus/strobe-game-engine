#pragma once

#include "strobe/core/type_traits/member_function_traits.hpp"

#include <type_traits>

namespace strobe {

namespace details {

template <typename>
inline constexpr bool call_operator_traits_false_v = false;

template <typename T>
using call_operator_owner_t = std::remove_cvref_t<T>;

} // namespace details

template <typename T>
concept has_unique_call_operator = requires {
  &details::call_operator_owner_t<T>::operator();
};

template <typename T>
struct CallOperatorTraits {
  static_assert(has_unique_call_operator<T>,
                "CallOperatorTraits<T> requires T to define exactly one "
                "non-template operator(); overloaded and templated call "
                "operators are not supported");

private:
  using owner_type = details::call_operator_owner_t<T>;
  using function_pointer_type = decltype(&owner_type::operator());

public:
  using owner = owner_type;
  using function_pointer = function_pointer_type;

  using member_function_traits = MemberFunctionTraits<function_pointer_type>;

  using class_type = typename member_function_traits::class_type;
  using return_type = typename member_function_traits::return_type;
  using argument_types = typename member_function_traits::argument_types;

  static constexpr bool is_const = member_function_traits::is_const;
  static constexpr bool is_volatile = member_function_traits::is_volatile;
  static constexpr bool is_cv_qualified =
      member_function_traits::is_cv_qualified;

  static constexpr bool is_noexcept = member_function_traits::is_noexcept;

  static constexpr bool is_lvalue_ref_qualified =
      member_function_traits::is_lvalue_ref_qualified;

  static constexpr bool is_rvalue_ref_qualified =
      member_function_traits::is_rvalue_ref_qualified;

  static constexpr bool is_ref_qualified =
      member_function_traits::is_ref_qualified;
};

template <typename T>
using call_operator_owner_t =
    typename CallOperatorTraits<std::remove_cvref_t<T>>::owner;

template <typename T>
using call_operator_function_pointer_t =
    typename CallOperatorTraits<std::remove_cvref_t<T>>::function_pointer;

template <typename T>
using call_operator_return_t =
    typename CallOperatorTraits<std::remove_cvref_t<T>>::return_type;

template <typename T>
using call_operator_argument_types_t =
    typename CallOperatorTraits<std::remove_cvref_t<T>>::argument_types;

template <typename T>
inline constexpr bool call_operator_is_const_v =
    CallOperatorTraits<std::remove_cvref_t<T>>::is_const;

template <typename T>
inline constexpr bool call_operator_is_volatile_v =
    CallOperatorTraits<std::remove_cvref_t<T>>::is_volatile;

template <typename T>
inline constexpr bool call_operator_is_cv_qualified_v =
    CallOperatorTraits<std::remove_cvref_t<T>>::is_cv_qualified;

template <typename T>
inline constexpr bool call_operator_is_noexcept_v =
    CallOperatorTraits<std::remove_cvref_t<T>>::is_noexcept;

template <typename T>
inline constexpr bool call_operator_is_lvalue_ref_qualified_v =
    CallOperatorTraits<std::remove_cvref_t<T>>::is_lvalue_ref_qualified;

template <typename T>
inline constexpr bool call_operator_is_rvalue_ref_qualified_v =
    CallOperatorTraits<std::remove_cvref_t<T>>::is_rvalue_ref_qualified;

template <typename T>
inline constexpr bool call_operator_is_ref_qualified_v =
    CallOperatorTraits<std::remove_cvref_t<T>>::is_ref_qualified;

} // namespace strobe
