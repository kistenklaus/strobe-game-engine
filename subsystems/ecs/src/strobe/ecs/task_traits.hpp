#pragma once

#include "strobe/core/type_traits/types.hpp"
#include "strobe/core/type_traits/call_operator_traits.hpp"
#include "strobe/core/type_traits/dependent_false.hpp"

#include "strobe/ecs/access_traits.hpp"

#include <concepts>
#include <type_traits>

namespace strobe::ecs {

namespace details {

template <typename T>
using task_type_t = std::remove_cvref_t<T>;

// =============================================================================
// Validation
// =============================================================================

template <typename CallableTraits>
struct SyncTaskCallableValidator {
  static_assert(std::same_as<typename CallableTraits::return_type, void>,
                "Synchronous task operator()(...) must return void");

  static_assert(!CallableTraits::is_volatile,
                "Task operator()(...) must not be volatile");

  static_assert(!CallableTraits::is_rvalue_ref_qualified,
                "Task operator()(...) must not be &&-qualified");
};

template <typename CallableTraits>
struct AsyncTaskWorkerValidator {
  static_assert(types_empty_v<typename CallableTraits::argument_types>,
                "Async task operator() must not take ECS parameters");

  static_assert(!std::same_as<typename CallableTraits::return_type, void>,
                "Async task operator() must return a completion callable");

  static_assert(!CallableTraits::is_volatile,
                "Async task operator() must not be volatile");

  static_assert(!CallableTraits::is_rvalue_ref_qualified,
                "Async task operator() must not be &&-qualified");
};

template <typename CallableTraits>
struct AsyncTaskCompletionValidator {
  static_assert(std::same_as<typename CallableTraits::return_type, void>,
                "Async task completion callable must return void");

  static_assert(!CallableTraits::is_volatile,
                "Async task completion callable must not be volatile");

  static_assert(!CallableTraits::is_rvalue_ref_qualified,
                "Async task completion callable must not be &&-qualified");
};

// =============================================================================
// Async completion argument handling
// =============================================================================
//
// Async completion callables may optionally take the completed task object as
// their first argument:
//
//   [](MyTask&& self, ResourceCommands resources) { ... }
//
// The leading MyTask&& is not an ECS parameter and is stripped before deriving
// access semantics.
//

template <typename Task, typename Args>
struct AsyncCompletionArguments;

template <typename Task>
struct AsyncCompletionArguments<Task, Types<>> {
  static constexpr bool has_task_argument = false;
  using ecs_argument_types = Types<>;
};

template <typename Task, typename First, typename... Rest>
struct AsyncCompletionArguments<Task, Types<First, Rest...>> {
private:
  using task_type = std::remove_cvref_t<Task>;

  static constexpr bool is_exact_task_rvalue_ref =
      std::is_same_v<First, task_type&&>;

  static constexpr bool is_some_task_reference =
      std::is_same_v<std::remove_cvref_t<First>, task_type>;

public:
  static_assert(!is_some_task_reference || is_exact_task_rvalue_ref,
                "If an async task completion callable takes the task object, "
                "it must take it as Task&&");

  static constexpr bool has_task_argument = is_exact_task_rvalue_ref;

  using ecs_argument_types =
      std::conditional_t<has_task_argument,
                         Types<Rest...>,
                         Types<First, Rest...>>;
};

// =============================================================================
// Task implementation split
// =============================================================================

template <typename T, bool IsAsync>
struct TaskTraitsImpl;

// -----------------------------------------------------------------------------
// Synchronous task
// -----------------------------------------------------------------------------

template <typename T>
struct TaskTraitsImpl<T, false> {
  using task_type = std::remove_cvref_t<T>;

  static constexpr bool is_async = false;

  using callable = CallOperatorTraits<task_type>;

  [[maybe_unused]] static constexpr SyncTaskCallableValidator<callable>
      validation{};

  using worker_callable = callable;
  using worker_return_type = typename worker_callable::return_type;

  using completion_type = void;
  using completion_callable = void;
  using completion_argument_types = Types<>;

  static constexpr bool completion_takes_task = false;

  using argument_types = typename callable::argument_types;

  static constexpr bool is_const_call_operator = callable::is_const;
  static constexpr bool is_noexcept = callable::is_noexcept;

  static constexpr bool worker_is_const_call_operator = callable::is_const;
  static constexpr bool worker_is_noexcept = callable::is_noexcept;

  static constexpr bool completion_is_const_call_operator = false;
  static constexpr bool completion_is_noexcept = false;
};

// -----------------------------------------------------------------------------
// Asynchronous task
// -----------------------------------------------------------------------------

template <typename T>
struct TaskTraitsImpl<T, true> {
  using task_type = std::remove_cvref_t<T>;

  static constexpr bool is_async = true;

  // Worker callable: runs outside ECS.
  using worker_callable = CallOperatorTraits<task_type>;

  [[maybe_unused]] static constexpr AsyncTaskWorkerValidator<worker_callable>
      worker_validation{};

  using worker_return_type = typename worker_callable::return_type;
  using completion_type = std::remove_cvref_t<worker_return_type>;

  static constexpr bool worker_is_const_call_operator =
      worker_callable::is_const;

  static constexpr bool worker_is_noexcept = worker_callable::is_noexcept;

  static_assert(has_unique_call_operator<completion_type>,
                "Async task operator() must return a completion callable with "
                "exactly one non-template operator(); overloads are not "
                "allowed");

  // Completion callable: runs at ECS control point.
  using completion_callable = CallOperatorTraits<completion_type>;

  [[maybe_unused]] static constexpr
      AsyncTaskCompletionValidator<completion_callable>
          completion_validation{};

  using completion_argument_types =
      typename completion_callable::argument_types;

  using completion_arguments =
      AsyncCompletionArguments<task_type, completion_argument_types>;

  static constexpr bool completion_takes_task =
      completion_arguments::has_task_argument;

  using argument_types = typename completion_arguments::ecs_argument_types;

  static constexpr bool completion_is_const_call_operator =
      completion_callable::is_const;

  static constexpr bool completion_is_noexcept =
      completion_callable::is_noexcept;

  // For generic ECS-visible invocation code, this describes the callable that
  // receives ECS parameters.
  static constexpr bool is_const_call_operator =
      completion_is_const_call_operator;

  static constexpr bool is_noexcept = completion_is_noexcept;
};

} // namespace details

// =============================================================================
// TaskTraits
// =============================================================================
//
// A task has one user-facing trait type.
//
// If T::operator()(...) returns void:
//   - the task is synchronous
//   - ECS access is derived from T::operator()(...) arguments
//
// If T::operator()() returns a non-void completion callable:
//   - the task is asynchronous
//   - T::operator() must take no ECS parameters
//   - ECS access is derived from the returned completion callable
//   - the completion callable may optionally take T&& as first argument
//

template <typename T>
struct TaskTraits {
  using task_type = std::remove_cvref_t<T>;

  static_assert(std::is_class_v<task_type>,
                "Task must be a class or struct type");

  static_assert(has_unique_call_operator<task_type>,
                "Task must define exactly one non-template operator(); "
                "overloads are not allowed");

private:
  using raw_callable = CallOperatorTraits<task_type>;
  using raw_return_type = typename raw_callable::return_type;

public:
  static constexpr bool is_async =
      !std::same_as<raw_return_type, void>;

private:
  using impl = details::TaskTraitsImpl<task_type, is_async>;

public:
  using callable = raw_callable;

  using worker_callable = typename impl::worker_callable;
  using worker_return_type = typename impl::worker_return_type;

  using completion_type = typename impl::completion_type;
  using completion_callable = typename impl::completion_callable;
  using completion_argument_types = typename impl::completion_argument_types;

  static constexpr bool completion_takes_task =
      impl::completion_takes_task;

  using argument_types = typename impl::argument_types;

  static constexpr bool worker_is_const_call_operator =
      impl::worker_is_const_call_operator;

  static constexpr bool worker_is_noexcept =
      impl::worker_is_noexcept;

  static constexpr bool completion_is_const_call_operator =
      impl::completion_is_const_call_operator;

  static constexpr bool completion_is_noexcept =
      impl::completion_is_noexcept;

  static constexpr bool is_const_call_operator =
      impl::is_const_call_operator;

  static constexpr bool is_noexcept =
      impl::is_noexcept;

  // ---------------------------------------------------------------------------
  // Access metadata
  // ---------------------------------------------------------------------------

  using access = access_from_arguments_t<argument_types>;

  using read_components = typename access::read_components;
  using write_components = typename access::write_components;

  using read_resources = typename access::read_resources;
  using write_resources = typename access::write_resources;

  using read_events = typename access::read_events;
  using write_events = typename access::write_events;

  using required_resources = required_resources_of_access_t<access>;

  static constexpr bool records_entity_commands =
      access::records_entity_commands;

  static constexpr bool records_resource_commands =
      access::records_resource_commands;

  static constexpr bool records_system_commands =
      access::records_system_commands;

  static constexpr bool records_task_commands =
      access::records_task_commands;

  static constexpr bool records_runtime_commands =
      access::records_runtime_commands;

  static constexpr bool records_any_commands =
      access::records_any_commands;

  static constexpr bool valid = true;
};

template <typename T>
using Task = TaskTraits<T>;

template <typename T>
concept task = TaskTraits<std::remove_cvref_t<T>>::valid;

} // namespace strobe::ecs
