#pragma once

#include "strobe/core/type_traits/types.hpp"
#include "strobe/core/type_traits/dependent_false.hpp"
#include "strobe/core/type_traits/member_function_traits.hpp"

#include "strobe/ecs/access_traits.hpp"

#include <concepts>
#include <type_traits>

namespace strobe {

namespace ecs {

namespace details {

template <typename S>
using system_type_t = std::remove_cvref_t<S>;

// =============================================================================
// run_before / run_after metadata
// =============================================================================

template <typename S>
concept HasRunBefore = requires {
  typename system_type_t<S>::run_before;
};

template <typename S>
concept HasRunAfter = requires {
  typename system_type_t<S>::run_after;
};

template <typename S>
struct RunBefore {
  using type = Types<>;
};

template <HasRunBefore S>
struct RunBefore<S> {
  using type = typename system_type_t<S>::run_before;

  static_assert(types<type>, "S::run_before must be declared as Types<...>");
};

template <typename S>
struct RunAfter {
  using type = Types<>;
};

template <HasRunAfter S>
struct RunAfter<S> {
  using type = typename system_type_t<S>::run_after;

  static_assert(types<type>, "S::run_after must be declared as Types<...>");
};

template <typename S>
using run_before_t = typename RunBefore<system_type_t<S>>::type;

template <typename S>
using run_after_t = typename RunAfter<system_type_t<S>>::type;

// =============================================================================
// Named lifecycle function detection
// =============================================================================
//
// Detects whether a member name exists separately from whether taking its address
// is valid. This lets us reject overloaded optional lifecycle functions instead
// of silently treating them as missing.
//
// Limitation: this fallback-base trick requires non-final system types.
//

#define STROBE_ECS_DEFINE_LIFECYCLE_DETECTION(PascalName, lower_name)          \
  template <typename T>                                                        \
  struct HasAny##PascalName {                                                  \
  private:                                                                     \
    struct Fallback {                                                          \
      void lower_name();                                                       \
    };                                                                         \
                                                                               \
    struct Derived : system_type_t<T>, Fallback {};                            \
                                                                               \
    template <typename U>                                                      \
    static std::false_type test(decltype(&U::lower_name)*);                    \
                                                                               \
    template <typename>                                                        \
    static std::true_type test(...);                                           \
                                                                               \
  public:                                                                      \
    static constexpr bool value = decltype(test<Derived>(nullptr))::value;     \
  };                                                                           \
                                                                               \
  template <typename T>                                                        \
  inline constexpr bool has_any_##lower_name##_v =                              \
      HasAny##PascalName<T>::value;                                            \
                                                                               \
  template <typename T>                                                        \
  concept HasUnique##PascalName = requires {                                   \
    &system_type_t<T>::lower_name;                                             \
  }

STROBE_ECS_DEFINE_LIFECYCLE_DETECTION(Setup, setup);
STROBE_ECS_DEFINE_LIFECYCLE_DETECTION(Start, start);
STROBE_ECS_DEFINE_LIFECYCLE_DETECTION(Update, update);
STROBE_ECS_DEFINE_LIFECYCLE_DETECTION(Stop, stop);
STROBE_ECS_DEFINE_LIFECYCLE_DETECTION(Teardown, teardown);

#undef STROBE_ECS_DEFINE_LIFECYCLE_DETECTION

// =============================================================================
// Lifecycle function validation
// =============================================================================

template <typename FnTraits>
struct LifecycleFunctionValidator {
  static_assert(std::same_as<typename FnTraits::return_type, void>,
                "System lifecycle functions must return void");

  static_assert(FnTraits::is_const,
                "System lifecycle functions must be const");

  static_assert(!FnTraits::is_volatile,
                "System lifecycle functions must not be volatile");

  static_assert(!FnTraits::is_rvalue_ref_qualified,
                "System lifecycle functions must not be &&-qualified");
};

// =============================================================================
// Optional / required lifecycle function traits
// =============================================================================

#define STROBE_ECS_DEFINE_OPTIONAL_LIFECYCLE_FUNCTION(PascalName, lower_name)  \
  template <typename S,                                                        \
            bool Exists = has_any_##lower_name##_v<S>,                         \
            bool Unique = HasUnique##PascalName<S>>                            \
  struct PascalName##Function;                                                 \
                                                                               \
  template <typename S>                                                        \
  struct PascalName##Function<S, false, false> {                               \
    static constexpr bool exists = false;                                      \
    static constexpr bool is_const = true;                                     \
    static constexpr bool is_volatile = false;                                 \
    static constexpr bool is_noexcept = false;                                 \
    static constexpr bool is_lvalue_ref_qualified = false;                     \
    static constexpr bool is_rvalue_ref_qualified = false;                     \
    static constexpr bool is_ref_qualified = false;                            \
                                                                               \
    using return_type = void;                                                  \
    using argument_types = Types<>;                                            \
  };                                                                           \
                                                                               \
  template <typename S>                                                        \
  struct PascalName##Function<S, true, true>                                   \
      : MemberFunctionTraits<decltype(&system_type_t<S>::lower_name)> {        \
  private:                                                                     \
    using traits =                                                             \
        MemberFunctionTraits<decltype(&system_type_t<S>::lower_name)>;         \
                                                                               \
  public:                                                                      \
    static constexpr bool exists = true;                                       \
                                                                               \
    [[maybe_unused]] static constexpr LifecycleFunctionValidator<traits>        \
        validation{};                                                          \
  };                                                                           \
                                                                               \
  template <typename S>                                                        \
  struct PascalName##Function<S, true, false> {                                \
    static_assert(dependent_false_v<S>,                                        \
                  "System " #lower_name "(...) must be a unique non-template " \
                  "member function; overloads are not allowed");               \
  }

#define STROBE_ECS_DEFINE_REQUIRED_LIFECYCLE_FUNCTION(PascalName, lower_name)  \
  template <typename S,                                                        \
            bool Exists = has_any_##lower_name##_v<S>,                         \
            bool Unique = HasUnique##PascalName<S>>                            \
  struct PascalName##Function;                                                 \
                                                                               \
  template <typename S>                                                        \
  struct PascalName##Function<S, false, false> {                               \
    static_assert(dependent_false_v<S>,                                        \
                  "System must define " #lower_name "(...) const");            \
  };                                                                           \
                                                                               \
  template <typename S>                                                        \
  struct PascalName##Function<S, true, true>                                   \
      : MemberFunctionTraits<decltype(&system_type_t<S>::lower_name)> {        \
  private:                                                                     \
    using traits =                                                             \
        MemberFunctionTraits<decltype(&system_type_t<S>::lower_name)>;         \
                                                                               \
  public:                                                                      \
    static constexpr bool exists = true;                                       \
                                                                               \
    [[maybe_unused]] static constexpr LifecycleFunctionValidator<traits>        \
        validation{};                                                          \
  };                                                                           \
                                                                               \
  template <typename S>                                                        \
  struct PascalName##Function<S, true, false> {                                \
    static_assert(dependent_false_v<S>,                                        \
                  "System " #lower_name "(...) must be a unique non-template " \
                  "member function; overloads are not allowed");               \
  }

STROBE_ECS_DEFINE_OPTIONAL_LIFECYCLE_FUNCTION(Setup, setup);
STROBE_ECS_DEFINE_OPTIONAL_LIFECYCLE_FUNCTION(Start, start);
STROBE_ECS_DEFINE_REQUIRED_LIFECYCLE_FUNCTION(Update, update);
STROBE_ECS_DEFINE_OPTIONAL_LIFECYCLE_FUNCTION(Stop, stop);
STROBE_ECS_DEFINE_OPTIONAL_LIFECYCLE_FUNCTION(Teardown, teardown);

#undef STROBE_ECS_DEFINE_OPTIONAL_LIFECYCLE_FUNCTION
#undef STROBE_ECS_DEFINE_REQUIRED_LIFECYCLE_FUNCTION

} // namespace details

// =============================================================================
// SystemTraits
// =============================================================================

template <typename S>
struct SystemTraits {
  using system_type = std::remove_cvref_t<S>;

  static_assert(std::is_class_v<system_type>,
                "System must be a class or struct type");

  static_assert(!std::is_final_v<system_type>,
                "System types must not be final because lifecycle detection "
                "uses fallback-base introspection");

  // ---------------------------------------------------------------------------
  // Update graph dependency metadata
  // ---------------------------------------------------------------------------
  //
  // Barriers are modeled as graph nodes, just like systems:
  //
  //   using run_after = Types<OtherSystem, Barrier<SomeBarrier>>;
  //
  // The scheduler can later interpret Barrier<T> specially. At the trait level,
  // dependencies remain exactly as written by the user.
  // ---------------------------------------------------------------------------

  using run_before = details::run_before_t<system_type>;
  using run_after = details::run_after_t<system_type>;

  static_assert(types<run_before>,
                "S::run_before must be declared as Types<...>");

  static_assert(types<run_after>,
                "S::run_after must be declared as Types<...>");

  static_assert(types_unique_v<run_before>,
                "S::run_before contains duplicate graph nodes");

  static_assert(types_unique_v<run_after>,
                "S::run_after contains duplicate graph nodes");

  static_assert(types_disjoint_v<run_before, run_after>,
                "A system cannot list the same graph node in both run_before "
                "and run_after");

  static_assert(!types_contains_v<system_type, run_before>,
                "A system cannot run before itself");

  static_assert(!types_contains_v<system_type, run_after>,
                "A system cannot run after itself");

  // ---------------------------------------------------------------------------
  // Lifecycle signatures
  // ---------------------------------------------------------------------------

  using setup_function = details::SetupFunction<system_type>;
  using start_function = details::StartFunction<system_type>;
  using update_function = details::UpdateFunction<system_type>;
  using stop_function = details::StopFunction<system_type>;
  using teardown_function = details::TeardownFunction<system_type>;

  static constexpr bool has_setup = setup_function::exists;
  static constexpr bool has_start = start_function::exists;
  static constexpr bool has_update = update_function::exists;
  static constexpr bool has_stop = stop_function::exists;
  static constexpr bool has_teardown = teardown_function::exists;

  using setup_argument_types = typename setup_function::argument_types;
  using start_argument_types = typename start_function::argument_types;
  using update_argument_types = typename update_function::argument_types;
  using stop_argument_types = typename stop_function::argument_types;
  using teardown_argument_types = typename teardown_function::argument_types;

  static constexpr bool setup_is_noexcept = setup_function::is_noexcept;
  static constexpr bool start_is_noexcept = start_function::is_noexcept;
  static constexpr bool update_is_noexcept = update_function::is_noexcept;
  static constexpr bool stop_is_noexcept = stop_function::is_noexcept;
  static constexpr bool teardown_is_noexcept = teardown_function::is_noexcept;

  // ---------------------------------------------------------------------------
  // Per-lifecycle access metadata
  // ---------------------------------------------------------------------------

  using setup_access = access_from_arguments_t<setup_argument_types>;
  using start_access = access_from_arguments_t<start_argument_types>;
  using update_access = access_from_arguments_t<update_argument_types>;
  using stop_access = access_from_arguments_t<stop_argument_types>;
  using teardown_access = access_from_arguments_t<teardown_argument_types>;

  // ---------------------------------------------------------------------------
  // Lifecycle semantic groups
  // ---------------------------------------------------------------------------

  using install_access = setup_access;

  using activation_access = merge_access_t<start_access, update_access>;

  using steady_state_access = update_access;

  using deactivation_access = stop_access;

  using uninstall_access = teardown_access;

  using setup_required_resources =
      required_resources_of_access_t<setup_access>;

  using start_required_resources =
      required_resources_of_access_t<start_access>;

  using update_required_resources =
      required_resources_of_access_t<update_access>;

  using stop_required_resources =
      required_resources_of_access_t<stop_access>;

  using teardown_required_resources =
      required_resources_of_access_t<teardown_access>;

  using install_required_resources = setup_required_resources;

  using activation_required_resources =
      types_union_t<start_required_resources, update_required_resources>;

  using steady_state_required_resources = update_required_resources;

  using deactivation_required_resources = stop_required_resources;

  using uninstall_required_resources = teardown_required_resources;

  static_assert(
      types_subset_v<stop_required_resources, update_required_resources>,
      "stop(...) may not require resources that update(...) does not require; "
      "otherwise the system might be impossible to stop before resource "
      "destruction");

  // ---------------------------------------------------------------------------
  // Expanded lifecycle-specific aliases
  // ---------------------------------------------------------------------------

#define STROBE_ECS_DEFINE_ACCESS_ALIASES(prefix)                               \
  using prefix##_read_components = typename prefix##_access::read_components;  \
  using prefix##_write_components = typename prefix##_access::write_components;\
                                                                               \
  using prefix##_read_resources = typename prefix##_access::read_resources;    \
  using prefix##_write_resources = typename prefix##_access::write_resources;  \
                                                                               \
  using prefix##_read_events = typename prefix##_access::read_events;          \
  using prefix##_write_events = typename prefix##_access::write_events;        \
                                                                               \
  static constexpr bool prefix##_records_entity_commands =                     \
      prefix##_access::records_entity_commands;                                \
                                                                               \
  static constexpr bool prefix##_records_resource_commands =                   \
      prefix##_access::records_resource_commands;                              \
                                                                               \
  static constexpr bool prefix##_records_system_commands =                     \
      prefix##_access::records_system_commands;                                \
                                                                               \
  static constexpr bool prefix##_records_task_commands =                       \
      prefix##_access::records_task_commands;                                  \
                                                                               \
  static constexpr bool prefix##_records_runtime_commands =                    \
      prefix##_access::records_runtime_commands;                               \
                                                                               \
  static constexpr bool prefix##_records_any_commands =                        \
      prefix##_access::records_any_commands

  STROBE_ECS_DEFINE_ACCESS_ALIASES(setup);
  STROBE_ECS_DEFINE_ACCESS_ALIASES(start);
  STROBE_ECS_DEFINE_ACCESS_ALIASES(update);
  STROBE_ECS_DEFINE_ACCESS_ALIASES(stop);
  STROBE_ECS_DEFINE_ACCESS_ALIASES(teardown);

#undef STROBE_ECS_DEFINE_ACCESS_ALIASES

  static constexpr bool valid = true;
};

template <typename S>
using System = SystemTraits<S>;

template <typename S>
concept system = SystemTraits<std::remove_cvref_t<S>>::valid;

} // namespace ecs

} // namespace strobe
