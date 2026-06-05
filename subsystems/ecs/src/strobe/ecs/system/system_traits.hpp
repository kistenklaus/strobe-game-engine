#pragma once

#include "strobe/core/type_traits/types.hpp"
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

private:
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
