#pragma once

#include "strobe/core/type_traits/types.hpp"
#include <type_traits>

namespace strobe {

namespace details {

template <typename>
inline constexpr bool member_function_traits_false_v = false;

} // namespace details

template <typename T> struct MemberFunctionTraits {
  static_assert(details::member_function_traits_false_v<T>,
                "MemberFunctionTraits<T> requires T to be a supported "
                "non-static member function pointer type");
};

template <typename C, typename R, bool IsConst, bool IsVolatile,
          bool IsNoexcept, bool IsLvalueRefQualified, bool IsRvalueRefQualified,
          typename... Args>
struct MemberFunctionTraitsBase {
  using class_type = C;
  using return_type = R;
  using argument_types = Types<Args...>;

  static constexpr bool is_const = IsConst;
  static constexpr bool is_volatile = IsVolatile;
  static constexpr bool is_cv_qualified = IsConst || IsVolatile;

  static constexpr bool is_noexcept = IsNoexcept;

  static constexpr bool is_lvalue_ref_qualified = IsLvalueRefQualified;
  static constexpr bool is_rvalue_ref_qualified = IsRvalueRefQualified;
  static constexpr bool is_ref_qualified =
      IsLvalueRefQualified || IsRvalueRefQualified;
};

// =============================================================================
// Unqualified
// =============================================================================

template <typename C, typename R, typename... Args>
struct MemberFunctionTraits<R (C::*)(Args...)>
    : MemberFunctionTraitsBase<C, R, false, false, false, false, false,
                               Args...> {};

template <typename C, typename R, typename... Args>
struct MemberFunctionTraits<R (C::*)(Args...) noexcept>
    : MemberFunctionTraitsBase<C, R, false, false, true, false, false,
                               Args...> {};

// =============================================================================
// const
// =============================================================================

template <typename C, typename R, typename... Args>
struct MemberFunctionTraits<R (C::*)(Args...) const>
    : MemberFunctionTraitsBase<C, R, true, false, false, false, false,
                               Args...> {};

template <typename C, typename R, typename... Args>
struct MemberFunctionTraits<R (C::*)(Args...) const noexcept>
    : MemberFunctionTraitsBase<C, R, true, false, true, false, false, Args...> {
};

// =============================================================================
// volatile
// =============================================================================

template <typename C, typename R, typename... Args>
struct MemberFunctionTraits<R (C::*)(Args...) volatile>
    : MemberFunctionTraitsBase<C, R, false, true, false, false, false,
                               Args...> {};

template <typename C, typename R, typename... Args>
struct MemberFunctionTraits<R (C::*)(Args...) volatile noexcept>
    : MemberFunctionTraitsBase<C, R, false, true, true, false, false, Args...> {
};

// =============================================================================
// const volatile
// =============================================================================

template <typename C, typename R, typename... Args>
struct MemberFunctionTraits<R (C::*)(Args...) const volatile>
    : MemberFunctionTraitsBase<C, R, true, true, false, false, false, Args...> {
};

template <typename C, typename R, typename... Args>
struct MemberFunctionTraits<R (C::*)(Args...) const volatile noexcept>
    : MemberFunctionTraitsBase<C, R, true, true, true, false, false, Args...> {
};

// =============================================================================
// &
// =============================================================================

template <typename C, typename R, typename... Args>
struct MemberFunctionTraits<R (C::*)(Args...) &>
    : MemberFunctionTraitsBase<C, R, false, false, false, true, false,
                               Args...> {};

template <typename C, typename R, typename... Args>
struct MemberFunctionTraits<R (C::*)(Args...) & noexcept>
    : MemberFunctionTraitsBase<C, R, false, false, true, true, false, Args...> {
};

template <typename C, typename R, typename... Args>
struct MemberFunctionTraits<R (C::*)(Args...) const &>
    : MemberFunctionTraitsBase<C, R, true, false, false, true, false, Args...> {
};

template <typename C, typename R, typename... Args>
struct MemberFunctionTraits<R (C::*)(Args...) const & noexcept>
    : MemberFunctionTraitsBase<C, R, true, false, true, true, false, Args...> {
};

template <typename C, typename R, typename... Args>
struct MemberFunctionTraits<R (C::*)(Args...) volatile &>
    : MemberFunctionTraitsBase<C, R, false, true, false, true, false, Args...> {
};

template <typename C, typename R, typename... Args>
struct MemberFunctionTraits<R (C::*)(Args...) volatile & noexcept>
    : MemberFunctionTraitsBase<C, R, false, true, true, true, false, Args...> {
};

template <typename C, typename R, typename... Args>
struct MemberFunctionTraits<R (C::*)(Args...) const volatile &>
    : MemberFunctionTraitsBase<C, R, true, true, false, true, false, Args...> {
};

template <typename C, typename R, typename... Args>
struct MemberFunctionTraits<R (C::*)(Args...) const volatile & noexcept>
    : MemberFunctionTraitsBase<C, R, true, true, true, true, false, Args...> {};

// =============================================================================
// &&
// =============================================================================

template <typename C, typename R, typename... Args>
struct MemberFunctionTraits<R (C::*)(Args...) &&>
    : MemberFunctionTraitsBase<C, R, false, false, false, false, true,
                               Args...> {};

template <typename C, typename R, typename... Args>
struct MemberFunctionTraits<R (C::*)(Args...) && noexcept>
    : MemberFunctionTraitsBase<C, R, false, false, true, false, true, Args...> {
};

template <typename C, typename R, typename... Args>
struct MemberFunctionTraits<R (C::*)(Args...) const &&>
    : MemberFunctionTraitsBase<C, R, true, false, false, false, true, Args...> {
};

template <typename C, typename R, typename... Args>
struct MemberFunctionTraits<R (C::*)(Args...) const && noexcept>
    : MemberFunctionTraitsBase<C, R, true, false, true, false, true, Args...> {
};

template <typename C, typename R, typename... Args>
struct MemberFunctionTraits<R (C::*)(Args...) volatile &&>
    : MemberFunctionTraitsBase<C, R, false, true, false, false, true, Args...> {
};

template <typename C, typename R, typename... Args>
struct MemberFunctionTraits<R (C::*)(Args...) volatile && noexcept>
    : MemberFunctionTraitsBase<C, R, false, true, true, false, true, Args...> {
};

template <typename C, typename R, typename... Args>
struct MemberFunctionTraits<R (C::*)(Args...) const volatile &&>
    : MemberFunctionTraitsBase<C, R, true, true, false, false, true, Args...> {
};

template <typename C, typename R, typename... Args>
struct MemberFunctionTraits<R (C::*)(Args...) const volatile && noexcept>
    : MemberFunctionTraitsBase<C, R, true, true, true, false, true, Args...> {};

// =============================================================================
// Convenience aliases / constants
// =============================================================================

template <typename T>
using member_function_class_t =
    typename MemberFunctionTraits<std::remove_cvref_t<T>>::class_type;

template <typename T>
using member_function_return_t =
    typename MemberFunctionTraits<std::remove_cvref_t<T>>::return_type;

template <typename T>
using member_function_argument_types_t =
    typename MemberFunctionTraits<std::remove_cvref_t<T>>::argument_types;

template <typename T>
inline constexpr bool member_function_is_const_v =
    MemberFunctionTraits<std::remove_cvref_t<T>>::is_const;

template <typename T>
inline constexpr bool member_function_is_volatile_v =
    MemberFunctionTraits<std::remove_cvref_t<T>>::is_volatile;

template <typename T>
inline constexpr bool member_function_is_cv_qualified_v =
    MemberFunctionTraits<std::remove_cvref_t<T>>::is_cv_qualified;

template <typename T>
inline constexpr bool member_function_is_noexcept_v =
    MemberFunctionTraits<std::remove_cvref_t<T>>::is_noexcept;

template <typename T>
inline constexpr bool member_function_is_lvalue_ref_qualified_v =
    MemberFunctionTraits<std::remove_cvref_t<T>>::is_lvalue_ref_qualified;

template <typename T>
inline constexpr bool member_function_is_rvalue_ref_qualified_v =
    MemberFunctionTraits<std::remove_cvref_t<T>>::is_rvalue_ref_qualified;

template <typename T>
inline constexpr bool member_function_is_ref_qualified_v =
    MemberFunctionTraits<std::remove_cvref_t<T>>::is_ref_qualified;

template <typename T>
concept member_function_pointer =
    std::is_member_function_pointer_v<std::remove_cvref_t<T>>;

} // namespace strobe
