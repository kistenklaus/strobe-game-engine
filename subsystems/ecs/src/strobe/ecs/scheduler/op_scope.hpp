#pragma once

#include "strobe/ecs/scheduler/location.hpp"
#include <cstddef>
#include <tuple>

namespace strobe::ecs {

struct acquire_access {
  location loc;

  static constexpr size_t acquire_count = 1;
  static constexpr size_t release_count = 0;
  static constexpr size_t acq_rel_count = 0;

  template <typename Scope> void apply(Scope &scope) const noexcept {
    scope.acquire(loc);
  }

  explicit acquire_access(location loc) : loc(loc) {}
};

struct release_access {
  location loc;

  static constexpr size_t acquire_count = 0;
  static constexpr size_t release_count = 1;
  static constexpr size_t acq_rel_count = 0;

  template <typename Scope> void apply(Scope &scope) const noexcept {
    scope.release(loc);
  }

  explicit release_access(location loc) : loc(loc) {}
};

struct acq_rel_access {
  location loc;

  static constexpr size_t acquire_count = 0;
  static constexpr size_t release_count = 0;
  static constexpr size_t acq_rel_count = 1;

  template <typename Scope> void apply(Scope &scope) const noexcept {
    scope.acq_rel(loc);
  }
  explicit acq_rel_access(location loc) : loc(loc) {}
};

template <typename... Accesses> struct operation_scope_desc {
  std::tuple<Accesses...> accesses;

  static constexpr size_t acquire_count =
      (size_t{0} + ... + Accesses::acquire_count);

  static constexpr size_t release_count =
      (size_t{0} + ... + Accesses::release_count);

  static constexpr size_t acq_rel_count =
      (size_t{0} + ... + Accesses::acq_rel_count);

  explicit constexpr operation_scope_desc(Accesses... access_values) noexcept
      : accesses(access_values...) {}

  template <typename Scope> void apply(Scope &scope) const noexcept {
    std::apply(
        [&](const Accesses &...access) noexcept { (access.apply(scope), ...); },
        accesses);
  }
};

template <typename T> struct is_operation_scope_desc : std::false_type {};

template <typename... Accesses>
struct is_operation_scope_desc<operation_scope_desc<Accesses...>>
    : std::true_type {};


template <typename T>
inline constexpr bool is_operation_scope_desc_v =
    is_operation_scope_desc<std::remove_cvref_t<T>>::value;

inline constexpr acquire_access acquire(location loc) noexcept {
  return acquire_access{loc};
}

inline constexpr release_access release(location loc) noexcept {
  return release_access{loc};
}

inline constexpr acq_rel_access acq_rel(location loc) noexcept {
  return acq_rel_access{loc};
}

template <typename... Accesses>
constexpr auto op_scope(Accesses... accesses) noexcept {
  return operation_scope_desc<Accesses...>{accesses...};
}

} // namespace strobe::ecs
