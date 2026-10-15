#pragma once

#include "strobe/flow/compiler_plugins.hpp"
#include "strobe/flow/detail/compiler/add_resource_order.hpp"
#include "strobe/flow/detail/compiler/allocate_activation_states.hpp"
#include "strobe/flow/detail/compiler/allocate_storage.hpp"
#include "strobe/flow/detail/compiler/analyze_activation.hpp"
#include "strobe/flow/detail/compiler/analyze_components.hpp"
#include "strobe/flow/detail/compiler/analyze_lifetimes.hpp"
#include "strobe/flow/detail/compiler/assign_slots.hpp"
#include "strobe/flow/detail/compiler/build_edges.hpp"
#include "strobe/flow/detail/compiler/build_graph.hpp"
#include "strobe/flow/detail/compiler/build_interference.hpp"
#include "strobe/flow/detail/compiler/canonicalize.hpp"
#include "strobe/flow/detail/compiler/classify_nodes.hpp"
#include "strobe/flow/detail/compiler/classify_storage.hpp"
#include "strobe/flow/detail/compiler/emit_program.hpp"
#include "strobe/flow/detail/compiler/hoist_loop_invariants.hpp"
#include "strobe/flow/detail/compiler/lower_cutoffs.hpp"
#include "strobe/flow/detail/compiler/lower_values.hpp"
#include "strobe/flow/detail/compiler/resolve_phis.hpp"
#include "strobe/flow/detail/compiler/schedule_lifetimes.hpp"
#include "strobe/flow/detail/compiler/validate_ir.hpp"
#include "strobe/flow/detail/op_stream.hpp"
#include "strobe/flow/program.hpp"

namespace strobe::flow::detail {

template <typename IR, typename Passes>
inline void run_compiler_plugins(IR &ir, CompileContext &context,
                                 const Passes &passes) {
  for (const auto &pass : passes) {
    if (!pass) {
      throw std::logic_error("Flow compiler plugin is empty");
    }
    pass(ir, context);
    compiler::validate(ir);
  }
}
inline Program compile(OpStream &stream, const CompileOptions &options,
                       const CompilerPlugins &plugins) {
  CompileContext context;
  auto normalized = compiler::canonicalize(stream);
  run_compiler_plugins(normalized, context, plugins.normalized);
  auto values = compiler::lower_values(std::move(normalized));
  run_compiler_plugins(values, context, plugins.values);
  auto resolved = compiler::resolve_phis(std::move(values));
  compiler::validate_feedback_loops(resolved);
  auto cutoffs = compiler::lower_cutoffs(std::move(resolved));
  auto dependencies = compiler::build_edges(std::move(cutoffs));
  run_compiler_plugins(dependencies, context, plugins.dependencies);
  auto ordered = compiler::add_resource_order(std::move(dependencies));
  run_compiler_plugins(ordered, context, plugins.ordered);
  auto classified = compiler::hoist_loop_invariants(
      compiler::classify_nodes(std::move(ordered)));
  auto graph = compiler::build_graph(std::move(classified));
  run_compiler_plugins(graph, context, plugins.graph);
  auto components = compiler::analyze_components(std::move(graph));
  auto activation = compiler::analyze_activation(std::move(components));
  auto storage = compiler::classify_storage(std::move(activation));
  run_compiler_plugins(storage, context, plugins.storage);
  auto lifetimes = compiler::analyze_lifetimes(std::move(storage));
  run_compiler_plugins(lifetimes, context, plugins.lifetimes);
  auto interference = compiler::build_interference(std::move(lifetimes));
  auto slots = compiler::assign_slots(std::move(interference), options);
  auto layout = compiler::allocate_storage(std::move(slots));
  run_compiler_plugins(layout, context, plugins.storage_layout);
  auto schedule = compiler::schedule_lifetimes(std::move(layout));
  auto activations =
      compiler::allocate_activation_states(std::move(schedule), options);
  run_compiler_plugins(activations, context, plugins.activation_layout);
  auto data = compiler::emit_program(std::move(activations), stream);
  run_compiler_plugins(data, context, plugins.program);
  data.plugin_data = std::move(context);
  return Program(std::move(data));
}

inline Program compile(OpStream &stream, const CompileOptions &options = {}) {
  return compile(stream, options, CompilerPlugins{});
}

inline Program compile(OpStream &stream, const CompilerPlugins &plugins,
                       const CompileOptions &options = {}) {
  return compile(stream, options, plugins);
}

} // namespace strobe::flow::detail
