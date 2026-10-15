#pragma once

#include "strobe/rdg/access.hpp"
#include "strobe/rdg/barrier_plan.hpp"
#include "strobe/rdg/lifetime.hpp"
#include "strobe/rdg/resources.hpp"

#include <strobe/flow/flow.hpp>

#include <strobe/core/containers/span.hpp>
#include <strobe/core/containers/vector.hpp>

#include <tuple>
#include <utility>

namespace strobe::rdg {

using flow::acquire;
using flow::CompileOptions;
using flow::CompilerPlugins;
using flow::construct;
using flow::cutoff;
using flow::discard;
using flow::external;
using flow::external_op;
using flow::feedback;
using flow::if_;
using flow::Lifetime;
using flow::op;
using flow::ops;
using flow::phi;
using flow::Program;
using flow::read;
using flow::read_write;
using flow::Resource;
using flow::resource;
using flow::while_;
using flow::write;
using flow::yield;

namespace detail {
void record_scopes(std::size_t operation, span<const rhi::AccessScope> scopes);

template <typename Tuple, std::size_t... I>
std::size_t scoped_op(Tuple values, std::index_sequence<I...>) {
  Vector<rhi::AccessScope, flow::allocator_ref> scopes(flow::allocator_ref{});
  scopes.reserve(sizeof...(I));
  (scopes.push_back(std::get<I>(values).scope()), ...);
  const auto operation = flow::op(std::move(std::get<I>(values)).release()...,
                                  std::move(std::get<sizeof...(I)>(values)));
  record_scopes(operation, {scopes.data(), scopes.size()});
  return operation;
}
} // namespace detail

/**
 * \brief Records an operation whose resource accesses carry RHI scopes.
 * \code{.cpp}
 * template<typename... Args>
 * std::size_t op(Args&&... args);
 * \endcode
 *
 * The final argument is the callback. Every preceding argument must be an
 * RDG ScopedAccess produced by the two-argument read, write, or read_write
 * overload.
 *
 * \param args Scoped resource accesses followed by the callback.
 * \return Recorded source operation index.
 */
template <typename... Args>
  requires(sizeof...(Args) > 1 &&
           detail::scoped_access_prefix<std::tuple<std::decay_t<Args>...>>(
               std::make_index_sequence<sizeof...(Args) - 1>{}))
std::size_t op(Args &&...args) {
  auto values = std::tuple<std::decay_t<Args>...>(std::forward<Args>(args)...);
  return detail::scoped_op(std::move(values),
                           std::make_index_sequence<sizeof...(Args) - 1>{});
}

/**
 * \brief Starts an RDG recording with RHI compiler passes enabled.
 * \code{.cpp}
 * void begin(const CompileOptions& options = {});
 * void begin(const CompilerPlugins& plugins,
 *            const CompileOptions& options = {});
 * \endcode
 *
 * \param options Flow compilation options.
 * \param plugins Additional Flow compiler plugins.
 */
void begin(const CompileOptions &options = {});

void begin(const CompilerPlugins &plugins, const CompileOptions &options = {});

/**
 * \brief Validates and compiles the active RDG recording.
 * \code{.cpp}
 * Program end();
 * \endcode
 *
 * \return Compiled program.
 */
Program end();

} // namespace strobe::rdg
