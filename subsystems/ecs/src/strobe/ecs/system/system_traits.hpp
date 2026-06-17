#pragma once

#include "strobe/core/type_traits/dependent_false.hpp"
#include "strobe/core/type_traits/fixed_string.hpp"
#include "strobe/core/type_traits/types.hpp"
#include "strobe/core/type_traits/type_name.hpp"
#include "strobe/ecs/system/system_sequence_traits.hpp"
#include "strobe/ecs/system/system_setup_traits.hpp"
#include "strobe/ecs/system/system_start_traits.hpp"
#include "strobe/ecs/system/system_stop_traits.hpp"
#include "strobe/ecs/system/system_teardown_traits.hpp"
#include "strobe/ecs/system/system_update_traits.hpp"
#include <type_traits>

namespace strobe::ecs {

namespace details {

template <typename S> struct system_declaration_type {
  using type = std::remove_cvref_t<S>;
};

template <typename S> struct system_declaration_type<stateful_system<S>> {
  using type = std::remove_cvref_t<S>;
};

template <typename S>
using system_declaration_type_t =
    typename system_declaration_type<std::remove_cvref_t<S>>::type;

template <typename S>
using has_update_fn =
    std::bool_constant<system_update_exists_v<std::remove_cvref_t<S>>>;

template <typename U>
using has_update_system_function =
    std::bool_constant<system_update_exists_v<canonical_system_type_t<U>>>;

template <std::size_t N>
consteval void accept_fixed_string(const fixed_string<N> &) noexcept {}

template <typename S>
concept has_fixed_string_name = requires { accept_fixed_string(S::name); };

template <typename T> struct is_fixed_string : std::false_type {};

template <std::size_t N>
struct is_fixed_string<fixed_string<N>> : std::true_type {};

template <typename T>
inline constexpr bool is_fixed_string_v =
    is_fixed_string<std::remove_cvref_t<T>>::value;

template <typename S>
concept has_system_name = requires { S::name; };

template <typename S>
[[nodiscard]]
consteval auto resolve_system_name() {
  using declaration_type = system_declaration_type_t<S>;
  if constexpr (!has_system_name<declaration_type>) {
    return type_name<declaration_type>();
  } else {
    using raw_name_type = decltype(declaration_type::name);
    using name_type = std::remove_cvref_t<raw_name_type>;
    if constexpr (is_fixed_string_v<name_type>) {
      if constexpr (declaration_type::name.empty()) {
        return type_name<declaration_type>();
      } else {
        return declaration_type::name;
      }
    }
    else if constexpr (std::is_array_v<raw_name_type> &&
                       std::is_same_v<std::remove_cv_t<
                                          std::remove_extent_t<raw_name_type>>,
                                      char>) {
      constexpr std::size_t storage_size = std::extent_v<raw_name_type>;
      static_assert(storage_size > 0,
                    "System::name must contain a null terminator");
      static_assert(declaration_type::name[storage_size - 1] == '\0',
                    "System::name must be null terminated");
      if constexpr (storage_size == 1) {
        return type_name<declaration_type>();
      } else {
        return fixed_string{declaration_type::name};
      }
    }
    else if constexpr (std::is_same_v<name_type, std::string_view>) {
      constexpr std::string_view name = declaration_type::name;
      if constexpr (name.empty()) {
        return type_name<declaration_type>();
      } else {
        return fixed_string_from_view<name.size()>(name);
      }
    }
    else {
      static_assert(dependent_false_v<declaration_type>,
                    "System::name must be a fixed_string, "
                    "std::string_view, or char array");
    }
  }
}
} // namespace details

template <typename S> struct system_traits {
  using system_type = std::remove_cvref_t<S>;
  using declaration_type = details::system_declaration_type_t<system_type>;

  using setup_traits = system_setup_traits<system_type>;
  using start_traits = system_start_traits<system_type>;
  using update_traits = system_update_traits<system_type>;
  using stop_traits = system_stop_traits<system_type>;
  using teardown_traits = system_teardown_traits<system_type>;

  using ready_requirements =
      types_union_t<typename setup_traits::lifetime_arguments,
                    typename teardown_traits::lifetime_arguments>;
  using ready_persistent_requirements =
      types_union_t<typename teardown_traits::lifetime_arguments>;
  using ready_entry_only_requirements =
      types_subtract_t<ready_requirements, ready_persistent_requirements>;
  using active_requirements =
      types_union_t<typename start_traits::lifetime_arguments,
                    typename update_traits::lifetime_arguments,
                    typename stop_traits::lifetime_arguments>;
  using active_persistent_requirements =
      types_union_t<typename update_traits::lifetime_arguments,
                    typename stop_traits::lifetime_arguments>;
  using active_entry_only_requirements =
      types_subtract_t<active_requirements, active_persistent_requirements>;

  inline static constexpr auto name =
      details::resolve_system_name<system_type>();

private:
  using raw_sequenced_before =
      typename details::declared_sequenced_before<declaration_type>::type;

  using raw_sequenced_after =
      typename details::declared_sequenced_after<declaration_type>::type;

  static_assert(types<raw_sequenced_before>,
                "S::sequenced_before must be strobe::Types<...>");

  static_assert(types<raw_sequenced_after>,
                "S::sequenced_after must be strobe::Types<...>");

  template <typename U> struct canonical_system_type_mapper {
    using type = canonical_system_type_t<U>;
  };

  template <typename U>
  using has_update_system_function =
      std::bool_constant<system_update_exists_v<std::remove_cvref_t<U>>>;

  using normalized_sequenced_after =
      types_transform_t<raw_sequenced_after, canonical_system_type_mapper>;

  using normalized_sequenced_before =
      types_transform_t<raw_sequenced_before, canonical_system_type_mapper>;

public:
  using sequenced_after = types_unique_t<
      types_filter_t<normalized_sequenced_after, has_update_system_function>>;

  using sequenced_before = types_unique_t<
      types_filter_t<normalized_sequenced_before, has_update_system_function>>;
};

} // namespace strobe::ecs
