#include "strobe/core/type_traits/types.hpp"
#include "strobe/ecs/system/stateful_system.hpp"
#include "strobe/ecs/system/system_update_traits.hpp"

namespace strobe::ecs::details {

template <typename S, typename = void> struct declared_sequenced_before {
  using type = Types<>;
};

template <typename S>
struct declared_sequenced_before<
    S, std::void_t<typename std::remove_cvref_t<S>::sequenced_before>> {
  using type = typename std::remove_cvref_t<S>::sequenced_before;
};

template <typename S, typename = void> struct declared_sequenced_after {
  using type = Types<>;
};

template <typename S>
struct declared_sequenced_after<
    S, std::void_t<typename std::remove_cvref_t<S>::sequenced_after>> {
  using type = typename std::remove_cvref_t<S>::sequenced_after;
};

template <typename U> struct canonical_system_type_mapper {
  using type = canonical_system_type_t<U>;
};

template <typename U>
using has_update_system_function =
    std::bool_constant<system_update_exists_v<canonical_system_type_t<U>>>;

} // namespace strobe::ecs::details
