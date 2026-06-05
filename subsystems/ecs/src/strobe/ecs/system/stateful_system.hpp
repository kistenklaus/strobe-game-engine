#pragma once

#include "strobe/ecs/object/object_function.hpp"
#include <type_traits>

namespace strobe::ecs {

template <typename S> class stateful_system {
  using state_type = std::remove_cvref_t<S>;
};

template <typename S> struct is_stateful_system : std::false_type {};

template <typename S>
struct is_stateful_system<stateful_system<S>> : std::true_type {};

template <typename S>
inline constexpr bool is_stateful_system_v =
    is_stateful_system<std::remove_cvref_t<S>>::value;

template <typename S> struct canonical_system_type {
private:
  using system_type = std::remove_cvref_t<S>;

public:
  using type = std::conditional_t<
      is_stateful_system_v<system_type>, system_type,
      std::conditional_t<stateless_system_object<system_type>, system_type,
                         stateful_system<system_type>>>;
};

template <typename S>
using canonical_system_type_t = typename canonical_system_type<S>::type;

} // namespace strobe::ecs
