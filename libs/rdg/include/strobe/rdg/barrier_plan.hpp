#pragma once

#include <strobe/flow/compiler_plugins.hpp>
#include <strobe/flow/program.hpp>
#include <strobe/rhi/types/access_scope.hpp>
#include <strobe/rhi/types/memory_barrier.hpp>

#include <strobe/core/containers/vector.hpp>

#include <string_view>

namespace strobe::rdg {

/**
 * \brief Identifies the RDG barrier-plan compiler plugin state.
 * \code{.cpp}
 * inline constexpr std::string_view barrier_plan_plugin_id;
 * \endcode
 */
inline constexpr std::string_view barrier_plan_plugin_id =
    "strobe.rdg.barrier-plan";

/**
 * \brief Describes an ordered dependency and its required RHI barrier.
 * \code{.cpp}
 * struct BarrierDependency;
 * \endcode
 */
struct BarrierDependency {
  flow::detail::NodeId before;
  flow::detail::NodeId after;
  flow::ResourceId resource;
  flow::TypeId type;
  rhi::AccessScope source_scope = rhi::access::none;
  rhi::AccessScope destination_scope = rhi::access::none;
  rhi::MemoryBarrier barrier;
  bool materialized = false;
};

/**
 * \brief Stores the barriers derived while compiling one RDG program.
 * \code{.cpp}
 * struct BarrierPlan;
 * \endcode
 */
struct BarrierPlan {
  /**
   * \brief Constructs an empty barrier plan.
   * \code{.cpp}
   * BarrierPlan();
   * \endcode
   */
  BarrierPlan();

  Vector<BarrierDependency, flow::allocator_ref> dependencies;
};

/**
 * \brief Returns the RDG barrier plan attached to a compilation context.
 * \code{.cpp}
 * BarrierPlan& barrier_plan(flow::CompileContext& context);
 * const BarrierPlan& barrier_plan(const flow::CompileContext& context);
 * \endcode
 *
 * \param context Compilation context populated by the RDG compiler plugin.
 * \return The attached barrier plan.
 * \throws std::out_of_range if the context has no RDG barrier plan.
 */
BarrierPlan &barrier_plan(flow::CompileContext &context);
const BarrierPlan &barrier_plan(const flow::CompileContext &context);

/**
 * \brief Returns the RDG barrier plan attached to program data.
 * \code{.cpp}
 * BarrierPlan& barrier_plan(flow::detail::ProgramData& program);
 * const BarrierPlan& barrier_plan(const flow::detail::ProgramData& program);
 * \endcode
 *
 * \param program Compiled program data.
 * \return The attached barrier plan.
 * \throws std::out_of_range if the program has no RDG barrier plan.
 */
BarrierPlan &barrier_plan(flow::detail::ProgramData &program);
const BarrierPlan &barrier_plan(const flow::detail::ProgramData &program);

/**
 * \brief Returns the RDG barrier plan attached to a compiled program.
 * \code{.cpp}
 * BarrierPlan& barrier_plan(flow::Program& program);
 * const BarrierPlan& barrier_plan(const flow::Program& program);
 * \endcode
 *
 * \param program Compiled program.
 * \return The attached barrier plan.
 * \throws std::out_of_range if the program has no RDG barrier plan.
 */
BarrierPlan &barrier_plan(flow::Program &program);
const BarrierPlan &barrier_plan(const flow::Program &program);

/**
 * \brief Adds RDG barrier analysis to a Flow compiler plugin table.
 * \code{.cpp}
 * flow::CompilerPlugins compiler_plugins(
 *     flow::CompilerPlugins plugins = {});
 * \endcode
 *
 * \param plugins Existing compiler plugins to preserve.
 * \return The plugin table with RDG barrier analysis appended.
 */
flow::CompilerPlugins compiler_plugins(flow::CompilerPlugins plugins = {});

} // namespace strobe::rdg
