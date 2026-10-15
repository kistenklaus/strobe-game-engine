#pragma once

#include "strobe/flow/compile_context.hpp"
#include "strobe/flow/compile_options.hpp"
#include "strobe/flow/compiler_plugins.hpp"
#include "strobe/flow/detail/op_stream.hpp"
#include "strobe/flow/lifetime.hpp"
#include "strobe/flow/op_source.hpp"
#include "strobe/flow/program.hpp"
#include "strobe/flow/type_id.hpp"

namespace strobe::flow {

namespace detail {
OpStream &active_stream();
} // namespace detail

/**
 * \brief Starts a thread-local recording with its compilation configuration.
 * \code{.cpp}
 * void begin(const CompileOptions& options = {});
 * void begin(const CompilerPlugins& plugins,
 *            const CompileOptions& options = {});
 * void begin(const CompileOptions& options,
 *            const CompilerPlugins& plugins);
 * \endcode
 *
 * \param options Compilation options copied into the recording.
 * \param plugins Compiler plugin table copied into the recording.
 * \attention 1 No recording may already be active on the calling thread.
 */
void begin(const CompileOptions &options = {});

void begin(const CompilerPlugins &plugins, const CompileOptions &options = {});

void begin(const CompileOptions &options, const CompilerPlugins &plugins);

/**
 * \brief Declares an owned resource in the active recording.
 * \code{.cpp}
 * template<typename T>
 * Resource<T> resource();
 * \endcode
 *
 * \return The resource handle used by later recording operations.
 */
template <typename T> Resource<T> resource() {
  return detail::active_stream().resource<T>();
}

/**
 * \brief Declares a named externally owned resource.
 * \code{.cpp}
 * template<typename T>
 * Resource<T> external(std::string_view name);
 * \endcode
 *
 * \param name Name used to bind the object on the compiled program.
 * \return The borrowed resource handle.
 */
template <typename T> Resource<T> external(std::string_view name) {
  return detail::active_stream().external<T>(name);
}

/**
 * \brief Declares a value selected between an initial and feedback resource.
 * \code{.cpp}
 * template<typename T>
 * Resource<T> phi(const Resource<T>& initial);
 * \endcode
 *
 * \param initial Value selected for the first activation.
 * \return The phi output resource.
 */
template <typename T> Resource<T> phi(const Resource<T> &initial) {
  return detail::active_stream().phi(initial);
}

/**
 * \brief Declares a value that changes only when its input compares unequal.
 * \code{.cpp}
 * template<typename T>
 *   requires std::copy_constructible<T> &&
 *            requires(const T& left, const T& right) {
 *              { left == right } -> std::convertible_to<bool>;
 *            }
 * Resource<T> cutoff(const Resource<T>& input);
 * \endcode
 *
 * \param input Comparable input resource.
 * \return The memoization boundary output resource.
 */
template <typename T>
  requires std::copy_constructible<T> &&
           requires(const T &left, const T &right) {
             { left == right } -> std::convertible_to<bool>;
           }
Resource<T> cutoff(const Resource<T> &input) {
  return detail::active_stream().cutoff(input);
}

/**
 * \brief Connects a phi resource to its value for later activations.
 * \code{.cpp}
 * template<typename T>
 * void feedback(const Resource<T>& phi, const Resource<T>& value);
 * \endcode
 *
 * \param phi Phi output to connect.
 * \param value Feedback value selected after the first activation.
 */
template <typename T>
void feedback(const Resource<T> &phi, const Resource<T> &value) {
  detail::active_stream().feedback(phi, value);
}

/**
 * \brief Records a sequential loop controlled by a boolean resource.
 * \code{.cpp}
 * template<typename Body>
 *   requires std::invocable<Body&>
 * void while_(const Resource<bool>& condition, Body&& body);
 * \endcode
 *
 * The body is invoked while recording to delimit the loop operations. At
 * runtime the condition is tested before every iteration, and all operations
 * in the body execute sequentially within one aggregate job.
 *
 * \param condition Boolean resource tested before each iteration.
 * \param body Recording callback that emits the loop body.
 * \attention 1. The body must produce a new value of condition.
 * \attention 2. The body cannot contain acquire or yield operations.
 */
template <typename Body>
  requires std::invocable<Body &>
void while_(const Resource<bool> &condition, Body &&body) {
  detail::active_stream().while_(condition, std::forward<Body>(body));
}

/**
 * \brief Records a sequential runtime conditional.
 * \code{.cpp}
 * template<typename Then, typename Else>
 *   requires std::invocable<Then&> && std::invocable<Else&>
 * void if_(const Resource<bool>& condition,
 *          Then&& then_body,
 *          Else&& else_body);
 * \endcode
 *
 * Both callbacks are invoked while recording to delimit the two branches. At
 * runtime only the selected branch executes, sequentially within one
 * aggregate job.
 *
 * \param condition Boolean resource selecting the branch.
 * \param then_body Recording callback for the true branch.
 * \param else_body Recording callback for the false branch.
 * \attention 1. Branches cannot contain acquire or yield operations.
 * \attention 2. Branches cannot construct or discard resources.
 */
template <typename Then, typename Else>
  requires std::invocable<Then &> && std::invocable<Else &>
void if_(const Resource<bool> &condition, Then &&then_body, Else &&else_body) {
  detail::active_stream().if_(condition, std::forward<Then>(then_body),
                              std::forward<Else>(else_body));
}

/**
 * \brief Invalidates the current value of an owned resource.
 * \code{.cpp}
 * template<typename T>
 * std::size_t discard(const Resource<T>& resource);
 * \endcode
 *
 * \param resource Resource whose current value becomes unspecified.
 * \return The recorded operation index.
 */
template <typename T> std::size_t discard(const Resource<T> &resource) {
  return detail::active_stream().discard(resource);
}

/**
 * \brief Records the unique construction operation for an owned resource.
 * \code{.cpp}
 * template<typename T, typename... Args>
 *   requires std::is_constructible_v<T, std::decay_t<Args>...>
 * std::size_t construct(const Resource<T>& resource, Args&&... args);
 *
 * template<typename T, typename... Args>
 *   requires(sizeof...(Args) > 0 &&
 *            !std::is_constructible_v<T, std::decay_t<Args>...>)
 * std::size_t construct(const Resource<T>& resource, Args&&... args);
 *
 * template<typename T, typename... Args>
 * std::size_t construct(const Resource<T>& resource,
 *                       OpMetadata metadata,
 *                       Args&&... args);
 * \endcode
 *
 * The arguments may directly construct T, or may consist of resource accesses
 * followed by a construction callback.
 *
 * \param resource Resource defined by the construction.
 * \param metadata Operation metadata retained in the program.
 * \param args Constructor arguments, or accesses followed by a callback.
 * \return The recorded operation index.
 */
template <typename T, typename... Args>
  requires std::is_constructible_v<T, std::decay_t<Args>...>
std::size_t construct(const Resource<T> &resource, Args &&...args) {
  return detail::active_stream().construct(resource,
                                           std::forward<Args>(args)...);
}

template <typename T, typename... Args>
  requires(sizeof...(Args) > 0 &&
           !std::is_constructible_v<T, std::decay_t<Args>...>)
std::size_t construct(const Resource<T> &resource, Args &&...args) {
  return detail::active_stream().construct(resource,
                                           std::forward<Args>(args)...);
}

template <typename T, typename... Args>
std::size_t construct(const Resource<T> &resource, OpMetadata metadata,
                      Args &&...args) {
  return detail::active_stream().construct_with_metadata(
      resource, std::move(metadata), std::forward<Args>(args)...);
}

/**
 * \brief Records an operation and its declared resource accesses.
 * \code{.cpp}
 * template<typename... Args>
 * std::size_t op(Args&&... args);
 *
 * template<typename... Args>
 * std::size_t op(OpMetadata metadata, Args&&... args);
 * \endcode
 *
 * The final argument is the callback and every preceding argument is a
 * resource access.
 *
 * \param metadata Operation metadata retained in the program.
 * \param args Resource accesses followed by the callback.
 * \return The recorded operation index.
 */
template <typename... Args> std::size_t op(Args &&...args) {
  return detail::active_stream().op(std::forward<Args>(args)...);
}

template <typename... Args>
std::size_t op(OpMetadata metadata, Args &&...args) {
  return detail::active_stream().op_with_metadata(std::move(metadata),
                                                  std::forward<Args>(args)...);
}

/**
 * \brief Records an unconditional submission boundary.
 * \code{.cpp}
 * std::size_t yield();
 * \endcode
 *
 * \return The recorded operation index.
 */
std::size_t yield();

/**
 * \brief Records acquisition of one token from a named semaphore.
 * \code{.cpp}
 * std::size_t acquire(std::string_view name);
 * \endcode
 *
 * \param name Semaphore released through Program::release.
 * \return The recorded operation index.
 */
std::size_t acquire(std::string_view name);

/**
 * \brief Validates and compiles the active thread-local recording.
 * \code{.cpp}
 * Program end();
 * \endcode
 *
 * \return Executable program compiled with the configuration supplied to
 * begin().
 * \throws std::logic_error if the recording is invalid or none is active.
 */
Program end();

/**
 * \brief Returns the operations in the current thread-local recording.
 * \code{.cpp}
 * const Vector<detail::Operation, allocator_ref>& ops();
 * \endcode
 *
 * \return The recorded operation sequence.
 */
const Vector<detail::Operation, allocator_ref> &ops();

} // namespace strobe::flow
