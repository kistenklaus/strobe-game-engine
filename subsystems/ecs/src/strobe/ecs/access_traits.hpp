#pragma once

#include "strobe/core/type_traits/dependent_false.hpp"
#include "strobe/core/type_traits/types.hpp"

#include "strobe/ecs/commands.hpp"
#include "strobe/ecs/events.hpp"
#include "strobe/ecs/query.hpp"
#include "strobe/ecs/resource.hpp"

#include <type_traits>

namespace strobe {

namespace ecs {

// =============================================================================
// Access metadata
// =============================================================================
//
// Access describes the semantic access requirements/capabilities of one
// callable invocation.
//
// Notes:
//   - read/write component/resource/event lists are normalized later.
//   - command handles are not modeled as reads/writes; they are deferred
//   mutation
//     capabilities.
//   - if a type is both read and written, normalization keeps it in the write
//   set
//     only.
//

template <typename ReadComponents, typename WriteComponents,
          typename ReadResources, typename WriteResources, typename ReadEvents,
          typename WriteEvents, bool RecordsEntityCommands,
          bool RecordsResourceCommands, bool RecordsSystemCommands,
          bool RecordsTaskCommands>
struct Access {
  using read_components = ReadComponents;
  using write_components = WriteComponents;

  using read_resources = ReadResources;
  using write_resources = WriteResources;

  using read_events = ReadEvents;
  using write_events = WriteEvents;

  static constexpr bool records_entity_commands = RecordsEntityCommands;
  static constexpr bool records_resource_commands = RecordsResourceCommands;
  static constexpr bool records_system_commands = RecordsSystemCommands;
  static constexpr bool records_task_commands = RecordsTaskCommands;

  static constexpr bool records_runtime_commands =
      RecordsResourceCommands || RecordsSystemCommands || RecordsTaskCommands;

  static constexpr bool records_any_commands =
      RecordsEntityCommands || records_runtime_commands;
};

using EmptyAccess = Access<Types<>, Types<>, Types<>, Types<>, Types<>, Types<>,
                           false, false, false, false>;

// =============================================================================
// Access normalization
// =============================================================================

template <typename A> struct NormalizeAccess {
private:
  using write_components = types_unique_t<typename A::write_components>;
  using write_resources = types_unique_t<typename A::write_resources>;
  using write_events = types_unique_t<typename A::write_events>;

  using read_components_without_writes =
      types_subtract_t<typename A::read_components, write_components>;

  using read_resources_without_writes =
      types_subtract_t<typename A::read_resources, write_resources>;

  using read_events_without_writes =
      types_subtract_t<typename A::read_events, write_events>;

public:
  using type =
      Access<types_unique_t<read_components_without_writes>, write_components,

             types_unique_t<read_resources_without_writes>, write_resources,

             types_unique_t<read_events_without_writes>, write_events,

             A::records_entity_commands, A::records_resource_commands,
             A::records_system_commands, A::records_task_commands>;
};

template <typename A>
using normalize_access_t = typename NormalizeAccess<A>::type;

// =============================================================================
// Access merging
// =============================================================================

template <typename... Accesses> struct MergeAccess;

template <> struct MergeAccess<> {
  using type = EmptyAccess;
};

template <typename A> struct MergeAccess<A> {
  using type = normalize_access_t<A>;
};

template <typename A, typename B, typename... Rest>
struct MergeAccess<A, B, Rest...> {
private:
  using merged = Access<
      types_concat_t<typename A::read_components, typename B::read_components>,
      types_concat_t<typename A::write_components,
                     typename B::write_components>,

      types_concat_t<typename A::read_resources, typename B::read_resources>,
      types_concat_t<typename A::write_resources, typename B::write_resources>,

      types_concat_t<typename A::read_events, typename B::read_events>,
      types_concat_t<typename A::write_events, typename B::write_events>,

      A::records_entity_commands || B::records_entity_commands,
      A::records_resource_commands || B::records_resource_commands,
      A::records_system_commands || B::records_system_commands,
      A::records_task_commands || B::records_task_commands>;

public:
  using type = typename MergeAccess<merged, Rest...>::type;
};

template <typename... Accesses>
using merge_access_t = typename MergeAccess<Accesses...>::type;

// =============================================================================
// Parameter access derivation
// =============================================================================

template <typename P, typename Raw = std::remove_cvref_t<P>>
struct ParameterAccess {
  static_assert(dependent_false_v<P>,
                "Unsupported ECS callable parameter type");
};

// -----------------------------------------------------------------------------
// EntityCommands
// -----------------------------------------------------------------------------

template <typename P> struct ParameterAccess<P, strobe::EntityCommands> {
private:
  static constexpr bool is_const = std::is_const_v<std::remove_reference_t<P>>;

public:
  static_assert(!is_const, "EntityCommands must not be accepted as const");

  using type = Access<Types<>, Types<>, Types<>, Types<>, Types<>, Types<>,
                      true, false, false, false>;
};

// -----------------------------------------------------------------------------
// ResourceCommands
// -----------------------------------------------------------------------------

template <typename P> struct ParameterAccess<P, strobe::ResourceCommands> {
private:
  static constexpr bool is_const = std::is_const_v<std::remove_reference_t<P>>;

public:
  static_assert(!is_const, "ResourceCommands must not be accepted as const");

  using type = Access<Types<>, Types<>, Types<>, Types<>, Types<>, Types<>,
                      false, true, false, false>;
};

// -----------------------------------------------------------------------------
// SystemCommands
// -----------------------------------------------------------------------------

template <typename P> struct ParameterAccess<P, strobe::SystemCommands> {
private:
  static constexpr bool is_const = std::is_const_v<std::remove_reference_t<P>>;

public:
  static_assert(!is_const, "SystemCommands must not be accepted as const");

  using type = Access<Types<>, Types<>, Types<>, Types<>, Types<>, Types<>,
                      false, false, true, false>;
};

// -----------------------------------------------------------------------------
// TaskCommands
// -----------------------------------------------------------------------------

template <typename P> struct ParameterAccess<P, strobe::TaskCommands> {
private:
  static constexpr bool is_const = std::is_const_v<std::remove_reference_t<P>>;

public:
  static_assert(!is_const, "TaskCommands must not be accepted as const");

  using type = Access<Types<>, Types<>, Types<>, Types<>, Types<>, Types<>,
                      false, false, false, true>;
};

// -----------------------------------------------------------------------------
// Query<...>
// -----------------------------------------------------------------------------

template <typename P, typename... Cs>
struct ParameterAccess<P, strobe::Query<Cs...>> {
private:
  using query_type = strobe::Query<Cs...>;

public:
  using type = Access<typename query_type::read_components,
                      typename query_type::write_components, Types<>, Types<>,
                      Types<>, Types<>, false, false, false, false>;
};

// -----------------------------------------------------------------------------
// Resource<R>
// -----------------------------------------------------------------------------

template <typename P, typename R>
struct ParameterAccess<P, strobe::Resource<R>> {
private:
  using resource_type = typename strobe::Resource<R>::value_type;

  using read_resources =
      types_if_t<strobe::Resource<R>::is_read_only, resource_type>;

  using write_resources =
      types_if_t<strobe::Resource<R>::is_writable, resource_type>;

public:
  using type = Access<Types<>, Types<>, read_resources, write_resources,
                      Types<>, Types<>, false, false, false, false>;
};

// -----------------------------------------------------------------------------
// Events<E>
// -----------------------------------------------------------------------------

template <typename P, typename E> struct ParameterAccess<P, strobe::Events<E>> {
private:
  using event_type = typename strobe::Events<E>::value_type;

  using read_events = types_if_t<strobe::Events<E>::is_read_stream, event_type>;

  using write_events =
      types_if_t<strobe::Events<E>::is_write_stream, event_type>;

public:
  using type = Access<Types<>, Types<>, Types<>, Types<>, read_events,
                      write_events, false, false, false, false>;
};

template <typename P>
using parameter_access_t = typename ParameterAccess<P>::type;

// =============================================================================
// Access derivation from argument list
// =============================================================================

template <typename Args> struct AccessFromArguments;

template <typename... Args> struct AccessFromArguments<Types<Args...>> {
private:
  using merged = merge_access_t<parameter_access_t<Args>...>;

public:
  using type = normalize_access_t<merged>;
};

template <types Args>
using access_from_arguments_t =
    typename AccessFromArguments<std::remove_cvref_t<Args>>::type;

// =============================================================================
// Access resource helpers
// =============================================================================

template <typename A> struct RequiredResourcesOfAccess {
  using type =
      types_union_t<typename A::read_resources, typename A::write_resources>;
};

template <typename A>
using required_resources_of_access_t =
    typename RequiredResourcesOfAccess<A>::type;

// =============================================================================
// Access conflict helpers
// =============================================================================
//
// These are conservative helpers for scheduling conflicts.
// They only cover immediate read/write conflicts in
// components/resources/events. Command handles have deferred semantics and are
// intentionally not treated as immediate read/write conflicts here.
//

template <typename A, typename B>
struct AccessComponentConflict
    : std::bool_constant<types_overlap_v<typename A::write_components,
                                         typename B::write_components> ||
                         types_overlap_v<typename A::write_components,
                                         typename B::read_components> ||
                         types_overlap_v<typename A::read_components,
                                         typename B::write_components>> {};

template <typename A, typename B>
inline constexpr bool access_component_conflict_v =
    AccessComponentConflict<A, B>::value;

template <typename A, typename B>
struct AccessResourceConflict
    : std::bool_constant<types_overlap_v<typename A::write_resources,
                                         typename B::write_resources> ||
                         types_overlap_v<typename A::write_resources,
                                         typename B::read_resources> ||
                         types_overlap_v<typename A::read_resources,
                                         typename B::write_resources>> {};

template <typename A, typename B>
inline constexpr bool access_resource_conflict_v =
    AccessResourceConflict<A, B>::value;

template <typename A, typename B>
struct AccessEventConflict
    : std::bool_constant<
          types_overlap_v<typename A::write_events, typename B::write_events> ||
          types_overlap_v<typename A::write_events, typename B::read_events> ||
          types_overlap_v<typename A::read_events, typename B::write_events>> {
};

template <typename A, typename B>
inline constexpr bool access_event_conflict_v =
    AccessEventConflict<A, B>::value;

template <typename A, typename B>
struct AccessConflict : std::bool_constant<access_component_conflict_v<A, B> ||
                                           access_resource_conflict_v<A, B> ||
                                           access_event_conflict_v<A, B>> {};

template <typename A, typename B>
inline constexpr bool access_conflict_v = AccessConflict<A, B>::value;

} // namespace ecs

} // namespace strobe
