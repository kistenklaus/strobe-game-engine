#include <strobe/flow/detail/compiler/add_resource_order.hpp>
#include <strobe/flow/detail/compiler/analyze_activation.hpp>
#include <strobe/flow/detail/compiler/analyze_components.hpp>
#include <strobe/flow/detail/compiler/analyze_lifetimes.hpp>
#include <strobe/flow/detail/compiler/assign_slots.hpp>
#include <strobe/flow/detail/compiler/build_edges.hpp>
#include <strobe/flow/detail/compiler/build_graph.hpp>
#include <strobe/flow/detail/compiler/build_interference.hpp>
#include <strobe/flow/detail/compiler/canonicalize.hpp>
#include <strobe/flow/detail/compiler/classify_nodes.hpp>
#include <strobe/flow/detail/compiler/classify_storage.hpp>
#include <strobe/flow/detail/compiler/hoist_loop_invariants.hpp>
#include <strobe/flow/detail/compiler/lower_cutoffs.hpp>
#include <strobe/flow/detail/compiler/lower_values.hpp>
#include <strobe/flow/detail/compiler/resolve_phis.hpp>

#include <gtest/gtest.h>

namespace flow = strobe::flow;
namespace compiler = strobe::flow::detail::compiler;
namespace graph = strobe::graph;
using Stream = flow::detail::OpStream;

namespace {

TEST(FlowCanonicalize, ProducesDenseNodesResourcesAndAccesses) {
  Stream stream;
  auto input = stream.external<int>("input");
  auto output = stream.resource<int>();
  stream.construct(
      output, flow::read(input),
      [](int *output, const int &input) { std::construct_at(output, input); });
  stream.op(flow::read(input), flow::write(output),
            [](const int &input, int &output) { output = input; });
  stream.validate();

  const auto ir = compiler::canonicalize(stream);
  ASSERT_EQ(ir.resources.size(), 2);
  EXPECT_EQ(ir.resources[0].kind, compiler::ResourceKind::external);
  EXPECT_EQ(ir.resources[1].kind, compiler::ResourceKind::owned);
  ASSERT_EQ(ir.nodes.size(), 2);
  EXPECT_EQ(ir.nodes[0].kind, compiler::NodeKind::construct);
  EXPECT_EQ(ir.nodes[1].kind, compiler::NodeKind::callback);
  EXPECT_EQ(ir.nodes[0].source_operation, 0);
  EXPECT_EQ(ir.nodes[1].source_operation, 1);
  ASSERT_EQ(ir.accesses.size(), 4);
  EXPECT_EQ(ir.accesses[0].resource, 1);
  EXPECT_EQ(ir.accesses[0].mode, flow::AccessMode::write);
  EXPECT_EQ(ir.accesses[1].resource, 0);
  EXPECT_EQ(ir.accesses[1].mode, flow::AccessMode::read);
}

TEST(FlowCanonicalize, TurnsPhiRelationsIntoNodes) {
  Stream stream;
  auto initial = stream.resource<int>();
  stream.construct(initial, 0);
  auto state = stream.phi(initial);
  auto next = stream.resource<int>();
  stream.construct(next, 1);
  stream.feedback(state, next);
  stream.validate();

  const auto ir = compiler::canonicalize(stream);
  ASSERT_EQ(ir.nodes.size(), 3);
  ASSERT_EQ(ir.phis.size(), 1);
  const auto &phi = ir.phis[0];
  EXPECT_EQ(phi.node, 1);
  EXPECT_EQ(ir.nodes[phi.node].kind, compiler::NodeKind::phi);
  EXPECT_EQ(ir.nodes[phi.node].source_operation,
            compiler::null_source_operation_id);
  EXPECT_EQ(phi.initial, 0);
  EXPECT_EQ(phi.output, 1);
  EXPECT_EQ(phi.feedback, 2);
  EXPECT_EQ(ir.nodes[phi.node].accesses.count, 3);
}

TEST(FlowCanonicalize, PreservesCutoffAndExternalRelations) {
  Stream stream;
  auto external = stream.external<int>("external");
  auto stable = stream.cutoff(external);
  stream.op(flow::read(stable), [](const int &) {});
  stream.validate();

  const auto ir = compiler::canonicalize(stream);
  ASSERT_EQ(ir.externals.size(), 1);
  EXPECT_EQ(ir.externals[0].resource, 0);
  ASSERT_EQ(ir.cutoffs.size(), 1);
  EXPECT_EQ(ir.cutoffs[0].node, 0);
  EXPECT_EQ(ir.nodes[ir.cutoffs[0].node].kind, compiler::NodeKind::cutoff);
  EXPECT_EQ(ir.cutoffs[0].input, 0);
  EXPECT_EQ(ir.cutoffs[0].output, 1);
}

TEST(FlowCanonicalize, PreservesPhiDeclarationOrderBeforeConsumers) {
  Stream stream;
  auto initial = stream.resource<int>();
  stream.construct(initial, 0);
  auto state = stream.phi(initial);
  auto next = stream.resource<int>();
  stream.construct(next, 0);
  stream.op(flow::read(state), flow::write(next),
            [](const int &state, int &next) { next = state + 1; });
  stream.feedback(state, next);
  stream.validate();

  const auto ir = compiler::canonicalize(stream);
  ASSERT_EQ(ir.nodes.size(), 4);
  EXPECT_EQ(ir.nodes[0].kind, compiler::NodeKind::construct);
  EXPECT_EQ(ir.nodes[0].source_operation, 0);
  EXPECT_EQ(ir.nodes[1].kind, compiler::NodeKind::phi);
  EXPECT_EQ(ir.nodes[1].source_operation, compiler::null_source_operation_id);
  EXPECT_EQ(ir.nodes[2].kind, compiler::NodeKind::construct);
  EXPECT_EQ(ir.nodes[2].source_operation, 1);
  EXPECT_EQ(ir.nodes[3].kind, compiler::NodeKind::callback);
  EXPECT_EQ(ir.nodes[3].source_operation, 2);
  EXPECT_EQ(ir.phis[0].node, 1);
}

TEST(FlowCanonicalize, PreservesOrderOfPhisAtTheSamePosition) {
  Stream stream;
  auto first_initial = stream.resource<int>();
  auto second_initial = stream.resource<int>();
  stream.construct(first_initial, 1);
  stream.construct(second_initial, 2);
  auto first = stream.phi(first_initial);
  auto second = stream.phi(second_initial);
  stream.feedback(first, first);
  stream.feedback(second, second);
  stream.validate();

  const auto ir = compiler::canonicalize(stream);
  ASSERT_EQ(ir.phis.size(), 2);
  EXPECT_LT(ir.phis[0].node, ir.phis[1].node);
  EXPECT_EQ(ir.phis[0].output, 2);
  EXPECT_EQ(ir.phis[1].output, 3);
  EXPECT_EQ(ir.nodes[ir.phis[0].node].kind, compiler::NodeKind::phi);
  EXPECT_EQ(ir.nodes[ir.phis[1].node].kind, compiler::NodeKind::phi);
}

TEST(FlowCanonicalize, NormalizesDiscardYieldAndAccessRanges) {
  Stream stream;
  auto value = stream.resource<int>();
  stream.construct(value, 1);
  stream.discard(value);
  stream.op(flow::write(value), [](int &value) { value = 2; });
  stream.yield();
  stream.validate();

  const auto ir = compiler::canonicalize(stream);
  ASSERT_EQ(ir.nodes.size(), 4);
  EXPECT_EQ(ir.nodes[0].kind, compiler::NodeKind::construct);
  EXPECT_EQ(ir.nodes[1].kind, compiler::NodeKind::discard);
  EXPECT_EQ(ir.nodes[2].kind, compiler::NodeKind::callback);
  EXPECT_EQ(ir.nodes[3].kind, compiler::NodeKind::yield);
  EXPECT_EQ(ir.nodes[0].accesses.offset, 0);
  EXPECT_EQ(ir.nodes[0].accesses.count, 1);
  EXPECT_EQ(ir.nodes[1].accesses.offset, 1);
  EXPECT_EQ(ir.nodes[1].accesses.count, 1);
  EXPECT_EQ(ir.nodes[2].accesses.offset, 2);
  EXPECT_EQ(ir.nodes[2].accesses.count, 1);
  EXPECT_EQ(ir.nodes[3].accesses.offset, 3);
  EXPECT_EQ(ir.nodes[3].accesses.count, 0);
  ASSERT_EQ(ir.accesses.size(), 3);
  EXPECT_EQ(ir.accesses[0].mode, flow::AccessMode::write);
  EXPECT_EQ(ir.accesses[1].mode, flow::AccessMode::write);
  EXPECT_EQ(ir.accesses[2].mode, flow::AccessMode::write);
}

TEST(FlowLowerValues, AssignsInputsAndOutputsToMutableAccesses) {
  Stream stream;
  auto value = stream.resource<int>();
  stream.construct(value, 1);
  stream.op(flow::read(value), [](const int &) {});
  stream.op(flow::write(value), [](int &value) { value = 2; });
  stream.op(flow::read_write(value), [](int &value) { ++value; });
  stream.discard(value);
  stream.op(flow::write(value), [](int &value) { value = 3; });
  stream.validate();

  const auto ir = compiler::lower_values(compiler::canonicalize(stream));
  ASSERT_EQ(ir.values.size(), 4);
  EXPECT_EQ(ir.values[0].version, 0);
  EXPECT_EQ(ir.values[1].version, 1);
  EXPECT_EQ(ir.values[2].version, 2);
  EXPECT_EQ(ir.values[3].version, 3);

  const auto construct = ir.nodes[0].accesses.offset;
  EXPECT_EQ(ir.accesses[construct].input, flow::detail::null_value_id);
  EXPECT_EQ(ir.accesses[construct].output, 0);
  const auto read = ir.nodes[1].accesses.offset;
  EXPECT_EQ(ir.accesses[read].input, 0);
  EXPECT_EQ(ir.accesses[read].output, flow::detail::null_value_id);
  const auto write = ir.nodes[2].accesses.offset;
  EXPECT_EQ(ir.accesses[write].input, 0);
  EXPECT_EQ(ir.accesses[write].output, 1);
  const auto read_write = ir.nodes[3].accesses.offset;
  EXPECT_EQ(ir.accesses[read_write].input, 1);
  EXPECT_EQ(ir.accesses[read_write].output, 2);
  const auto discard = ir.nodes[4].accesses.offset;
  EXPECT_EQ(ir.accesses[discard].input, flow::detail::null_value_id);
  EXPECT_EQ(ir.accesses[discard].output, flow::detail::null_value_id);
  const auto fresh_write = ir.nodes[5].accesses.offset;
  EXPECT_EQ(ir.accesses[fresh_write].input, flow::detail::null_value_id);
  EXPECT_EQ(ir.accesses[fresh_write].output, 3);
}

TEST(FlowLowerValues, CreatesInitialValuesForExternalResources) {
  Stream stream;
  auto external = stream.external<int>("external");
  auto stable = stream.cutoff(external);
  stream.op(flow::read(stable), [](const int &) {});
  stream.validate();

  const auto ir = compiler::lower_values(compiler::canonicalize(stream));
  ASSERT_EQ(ir.values.size(), 2);
  EXPECT_EQ(ir.values[0].resource, 0);
  EXPECT_EQ(ir.values[0].producer, flow::detail::null_node_id);
  EXPECT_EQ(ir.values[0].flags, compiler::StaticValueFlags::external);
  EXPECT_EQ(ir.values[1].resource, 1);
  EXPECT_EQ(ir.values[1].producer, 0);
  const auto cutoff = ir.nodes[0].accesses.offset;
  EXPECT_EQ(ir.accesses[cutoff].input, 0);
  EXPECT_EQ(ir.accesses[cutoff + 1].output, 1);
  const auto consumer = ir.nodes[1].accesses.offset;
  EXPECT_EQ(ir.accesses[consumer].input, 1);
}

TEST(FlowLowerValues, ResolvesFeedbackAtItsDeclarationPosition) {
  Stream stream;
  auto initial = stream.resource<int>();
  stream.construct(initial, 0);
  auto state = stream.phi(initial);
  auto next = stream.resource<int>();
  stream.construct(next, 1);
  stream.feedback(state, next);
  stream.op(flow::write(next), [](int &next) { next = 2; });
  stream.validate();

  const auto ir = compiler::lower_values(compiler::canonicalize(stream));
  ASSERT_EQ(ir.phis.size(), 1);
  EXPECT_EQ(ir.phis[0].initial, 0);
  EXPECT_EQ(ir.phis[0].output, 1);
  EXPECT_EQ(ir.phis[0].feedback, 2);
  ASSERT_EQ(ir.values.size(), 4);
  EXPECT_EQ(ir.values[2].producer, 2);
  EXPECT_EQ(ir.values[3].producer, 3);
}

TEST(FlowResolvePhis, EmitsInitialEdgesAndTemporalBackedges) {
  Stream stream;
  auto initial = stream.resource<int>();
  stream.construct(initial, 0);
  auto state = stream.phi(initial);
  auto next = stream.resource<int>();
  stream.construct(next, 0);
  stream.op(flow::read(state), flow::write(next),
            [](const int &state, int &next) { next = state + 1; });
  stream.feedback(state, next);
  stream.validate();

  const auto ir = compiler::resolve_phis(
      compiler::lower_values(compiler::canonicalize(stream)));
  ASSERT_EQ(ir.phi_edges.size(), 2);
  EXPECT_EQ(ir.phi_edges[0].kind, compiler::PhiEdgeKind::initial);
  EXPECT_EQ(ir.phi_edges[0].value, ir.values.phis[0].initial);
  EXPECT_EQ(ir.phi_edges[0].phi, ir.values.phis[0].node);
  EXPECT_EQ(ir.phi_edges[1].kind, compiler::PhiEdgeKind::feedback);
  EXPECT_EQ(ir.phi_edges[1].value, ir.values.phis[0].feedback);
  ASSERT_EQ(ir.backedges.size(), 1);
  EXPECT_EQ(ir.backedges[0].value, ir.values.phis[0].feedback);
  EXPECT_EQ(ir.backedges[0].target, ir.values.phis[0].node);
}

TEST(FlowResolvePhis, KeepsSelfFeedbackAsATemporalBackedge) {
  Stream stream;
  auto initial = stream.resource<int>();
  stream.construct(initial, 0);
  auto state = stream.phi(initial);
  stream.feedback(state, state);
  stream.validate();

  const auto ir = compiler::resolve_phis(
      compiler::lower_values(compiler::canonicalize(stream)));
  ASSERT_EQ(ir.backedges.size(), 1);
  EXPECT_EQ(ir.backedges[0].value, ir.values.phis[0].output);
  EXPECT_EQ(ir.values.values[ir.backedges[0].value].producer,
            ir.backedges[0].target);
}

TEST(FlowResolvePhis, SupportsIndependentFeedbackCycles) {
  Stream stream;
  auto first_initial = stream.resource<int>();
  auto second_initial = stream.resource<int>();
  stream.construct(first_initial, 1);
  stream.construct(second_initial, 2);
  auto first = stream.phi(first_initial);
  auto second = stream.phi(second_initial);
  stream.feedback(first, first);
  stream.feedback(second, second);
  stream.validate();

  const auto ir = compiler::resolve_phis(
      compiler::lower_values(compiler::canonicalize(stream)));
  ASSERT_EQ(ir.backedges.size(), 2);
  EXPECT_NE(ir.backedges[0].target, ir.backedges[1].target);
  EXPECT_EQ(ir.phi_edges.size(), 4);
}

TEST(FlowLowerCutoffs, ResolvesInputOutputValuesAndComparator) {
  Stream stream;
  auto external = stream.external<int>("external");
  auto stable = stream.cutoff(external);
  stream.op(flow::read(stable), [](const int &) {});
  stream.validate();

  const auto ir = compiler::lower_cutoffs(compiler::resolve_phis(
      compiler::lower_values(compiler::canonicalize(stream))));
  ASSERT_EQ(ir.cutoffs.size(), 1);
  const auto &cutoff = ir.cutoffs[0];
  EXPECT_EQ(cutoff.node, 0);
  EXPECT_EQ(cutoff.input, 0);
  EXPECT_EQ(cutoff.output, 1);
  EXPECT_EQ(cutoff.source_cutoff, 0);
  EXPECT_EQ(ir.resolved.values.values[cutoff.input].resource, 0);
  EXPECT_EQ(ir.resolved.values.values[cutoff.output].resource, 1);
  EXPECT_EQ(ir.resolved.values.values[cutoff.output].producer, cutoff.node);
}

TEST(FlowLowerCutoffs, PreservesChainedCutoffs) {
  Stream stream;
  auto external = stream.external<int>("external");
  auto first = stream.cutoff(external);
  stream.cutoff(first);
  stream.validate();

  const auto ir = compiler::lower_cutoffs(compiler::resolve_phis(
      compiler::lower_values(compiler::canonicalize(stream))));
  ASSERT_EQ(ir.cutoffs.size(), 2);
  EXPECT_EQ(ir.cutoffs[0].output, ir.cutoffs[1].input);
  EXPECT_EQ(ir.cutoffs[0].source_cutoff, 0);
  EXPECT_EQ(ir.cutoffs[1].source_cutoff, 1);
}

TEST(FlowLowerCutoffs, AcceptsPhiValuesAsInputs) {
  Stream stream;
  auto initial = stream.resource<int>();
  stream.construct(initial, 0);
  auto state = stream.phi(initial);
  auto stable = stream.cutoff(state);
  stream.feedback(state, stable);
  stream.validate();

  const auto ir = compiler::lower_cutoffs(compiler::resolve_phis(
      compiler::lower_values(compiler::canonicalize(stream))));
  ASSERT_EQ(ir.cutoffs.size(), 1);
  ASSERT_EQ(ir.resolved.backedges.size(), 1);
  EXPECT_EQ(ir.cutoffs[0].input, ir.resolved.values.phis[0].output);
  EXPECT_EQ(ir.resolved.backedges[0].value, ir.cutoffs[0].output);
}

TEST(FlowBuildEdges, ConnectsEachProducedValueToItsConsumers) {
  Stream stream;
  auto first = stream.resource<int>();
  auto second = stream.resource<int>();
  stream.construct(first, 1);
  stream.construct(
      second, flow::read(first),
      [](int *second, const int &first) { std::construct_at(second, first); });
  stream.op(flow::read(first), flow::read(second),
            [](const int &, const int &) {});
  stream.validate();

  const auto ir =
      compiler::build_edges(compiler::lower_cutoffs(compiler::resolve_phis(
          compiler::lower_values(compiler::canonicalize(stream)))));
  ASSERT_EQ(ir.edges.size(), 3);
  EXPECT_EQ(ir.edges[0].producer, 0);
  EXPECT_EQ(ir.edges[0].consumer, 1);
  EXPECT_EQ(ir.edges[0].value, 0);
  EXPECT_EQ(ir.edges[1].producer, 0);
  EXPECT_EQ(ir.edges[1].consumer, 2);
  EXPECT_EQ(ir.edges[1].value, 0);
  EXPECT_EQ(ir.edges[2].producer, 1);
  EXPECT_EQ(ir.edges[2].consumer, 2);
  EXPECT_EQ(ir.edges[2].value, 1);
}

TEST(FlowBuildEdges, DoesNotCreateEdgesForExternalProducers) {
  Stream stream;
  auto external = stream.external<int>("external");
  auto output = stream.resource<int>();
  stream.construct(output, flow::read(external),
                   [](int *output, const int &external) {
                     std::construct_at(output, external);
                   });
  stream.validate();

  const auto ir =
      compiler::build_edges(compiler::lower_cutoffs(compiler::resolve_phis(
          compiler::lower_values(compiler::canonicalize(stream)))));
  EXPECT_TRUE(ir.edges.empty());
}

TEST(FlowBuildEdges, KeepsPhiFeedbackOutOfDirectDependencies) {
  Stream stream;
  auto initial = stream.resource<int>();
  stream.construct(initial, 0);
  auto state = stream.phi(initial);
  auto next = stream.resource<int>();
  stream.construct(next, 1);
  stream.op(flow::read(state), flow::write(next),
            [](const int &state, int &next) { next = state + 1; });
  stream.feedback(state, next);
  stream.validate();

  const auto ir =
      compiler::build_edges(compiler::lower_cutoffs(compiler::resolve_phis(
          compiler::lower_values(compiler::canonicalize(stream)))));
  ASSERT_EQ(ir.lowered.resolved.backedges.size(), 1);
  ASSERT_EQ(ir.edges.size(), 3);
  EXPECT_EQ(ir.edges[0].producer, 0);
  EXPECT_EQ(ir.edges[0].consumer, 1);
  EXPECT_EQ(ir.edges[1].producer, 1);
  EXPECT_EQ(ir.edges[1].consumer, 3);
  EXPECT_EQ(ir.edges[2].producer, 2);
  EXPECT_EQ(ir.edges[2].consumer, 3);
  for (const auto &edge : ir.edges) {
    EXPECT_FALSE(edge.producer == 3 && edge.consumer == 1);
  }
}

TEST(FlowBuildEdges, ConnectsCutoffOutputsToConsumers) {
  Stream stream;
  auto source = stream.resource<int>();
  stream.construct(source, 1);
  auto stable = stream.cutoff(source);
  stream.op(flow::read(stable), [](const int &) {});
  stream.validate();

  const auto ir =
      compiler::build_edges(compiler::lower_cutoffs(compiler::resolve_phis(
          compiler::lower_values(compiler::canonicalize(stream)))));
  ASSERT_EQ(ir.edges.size(), 2);
  EXPECT_EQ(ir.edges[0].producer, 0);
  EXPECT_EQ(ir.edges[0].consumer, 1);
  EXPECT_EQ(ir.edges[1].producer, 1);
  EXPECT_EQ(ir.edges[1].consumer, 2);
  EXPECT_EQ(ir.edges[1].value, ir.lowered.cutoffs[0].output);
}

TEST(FlowResourceOrder, MakesAWriterWaitForAllOutstandingReaders) {
  Stream stream;
  auto value = stream.resource<int>();
  stream.construct(value, 1);
  stream.op(flow::read(value), [](const int &) {});
  stream.op(flow::read(value), [](const int &) {});
  stream.op(flow::write(value), [](int &value) { value = 2; });
  stream.validate();

  const auto ir = compiler::add_resource_order(
      compiler::build_edges(compiler::lower_cutoffs(compiler::resolve_phis(
          compiler::lower_values(compiler::canonicalize(stream))))));
  ASSERT_EQ(ir.resource_order.size(), 2);
  EXPECT_EQ(ir.resource_order[0].before, 1);
  EXPECT_EQ(ir.resource_order[0].after, 3);
  EXPECT_EQ(ir.resource_order[1].before, 2);
  EXPECT_EQ(ir.resource_order[1].after, 3);
}

TEST(FlowResourceOrder, LeavesIndependentReadersParallel) {
  Stream stream;
  auto value = stream.resource<int>();
  stream.construct(value, 1);
  stream.op(flow::read(value), [](const int &) {});
  stream.op(flow::read(value), [](const int &) {});
  stream.validate();

  const auto ir = compiler::add_resource_order(
      compiler::build_edges(compiler::lower_cutoffs(compiler::resolve_phis(
          compiler::lower_values(compiler::canonicalize(stream))))));
  EXPECT_TRUE(ir.resource_order.empty());
}

TEST(FlowResourceOrder, OrdersFreshWritesAfterDiscardedValueReaders) {
  Stream stream;
  auto value = stream.resource<int>();
  stream.construct(value, 1);
  stream.op(flow::read(value), [](const int &) {});
  stream.discard(value);
  stream.op(flow::write(value), [](int &value) { value = 2; });
  stream.validate();

  const auto ir = compiler::add_resource_order(
      compiler::build_edges(compiler::lower_cutoffs(compiler::resolve_phis(
          compiler::lower_values(compiler::canonicalize(stream))))));
  ASSERT_EQ(ir.resource_order.size(), 2);
  EXPECT_EQ(ir.resource_order[0].before, 1);
  EXPECT_EQ(ir.resource_order[0].after, 2);
  EXPECT_EQ(ir.resource_order[1].before, 2);
  EXPECT_EQ(ir.resource_order[1].after, 3);
}

TEST(FlowResourceOrder, DoesNotTurnPhiFeedbackIntoAConstraint) {
  Stream stream;
  auto initial = stream.resource<int>();
  stream.construct(initial, 0);
  auto state = stream.phi(initial);
  stream.op(flow::read_write(state), [](int &state) { ++state; });
  stream.feedback(state, state);
  stream.validate();

  const auto ir = compiler::add_resource_order(
      compiler::build_edges(compiler::lower_cutoffs(compiler::resolve_phis(
          compiler::lower_values(compiler::canonicalize(stream))))));
  ASSERT_EQ(ir.dependencies.lowered.resolved.backedges.size(), 1);
  for (const auto &constraint : ir.resource_order) {
    EXPECT_FALSE(constraint.before == 2 && constraint.after == 1);
  }
}

TEST(FlowClassifyNodes, FindsRootsAcrossDataAndOrderDependencies) {
  Stream stream;
  auto first = stream.resource<int>();
  auto second = stream.resource<int>();
  stream.construct(first, 1);
  stream.construct(second, 2);
  stream.op(flow::read(first), [](const int &) {});
  stream.op(flow::read(first), [](const int &) {});
  stream.op(flow::write(first), flow::read(second),
            [](int &first, const int &second) { first = second; });
  stream.validate();

  const auto ir = compiler::classify_nodes(compiler::add_resource_order(
      compiler::build_edges(compiler::lower_cutoffs(compiler::resolve_phis(
          compiler::lower_values(compiler::canonicalize(stream)))))));
  ASSERT_EQ(ir.roots.size(), 2);
  EXPECT_EQ(ir.roots[0], 0);
  EXPECT_EQ(ir.roots[1], 1);
  EXPECT_TRUE(ir.nodes[0].root);
  EXPECT_TRUE(ir.nodes[1].root);
  EXPECT_FALSE(ir.nodes[4].root);
}

TEST(FlowClassifyNodes, MarksDeclaredSideEffectsAndYieldAsObservableEffects) {
  Stream stream;
  auto value = stream.resource<int>();
  stream.construct(value, 1);
  stream.op_with_metadata(
      flow::OpMetadata("Observe", "", flow::OpSemantics::side_effect),
      flow::read(value), [](const int &) {});
  stream.yield();
  stream.discard(value);
  stream.validate();

  const auto ir = compiler::classify_nodes(compiler::add_resource_order(
      compiler::build_edges(compiler::lower_cutoffs(compiler::resolve_phis(
          compiler::lower_values(compiler::canonicalize(stream)))))));
  ASSERT_EQ(ir.observable_effects.size(), 2);
  EXPECT_EQ(ir.observable_effects[0], 1);
  EXPECT_EQ(ir.observable_effects[1], 2);
  EXPECT_FALSE(ir.nodes[0].observable_effect);
  EXPECT_TRUE(ir.nodes[1].observable_effect);
  EXPECT_TRUE(ir.nodes[2].observable_effect);
  EXPECT_FALSE(ir.nodes[3].observable_effect);
}

TEST(FlowClassifyNodes, RecordsExternalAccessDirections) {
  Stream stream;
  auto external = stream.external<int>("external");
  stream.op(flow::read(external), [](const int &) {});
  stream.op(flow::write(external), [](int &external) { external = 1; });
  stream.op(flow::read_write(external), [](int &external) { ++external; });
  stream.validate();

  const auto ir = compiler::classify_nodes(compiler::add_resource_order(
      compiler::build_edges(compiler::lower_cutoffs(compiler::resolve_phis(
          compiler::lower_values(compiler::canonicalize(stream)))))));
  ASSERT_EQ(ir.nodes.size(), 3);
  EXPECT_TRUE(ir.nodes[0].reads_external);
  EXPECT_FALSE(ir.nodes[0].writes_external);
  EXPECT_FALSE(ir.nodes[1].reads_external);
  EXPECT_TRUE(ir.nodes[1].writes_external);
  EXPECT_TRUE(ir.nodes[2].reads_external);
  EXPECT_TRUE(ir.nodes[2].writes_external);
}

TEST(FlowClassifyNodes, IgnoresTemporalBackedgesWhenFindingRoots) {
  Stream stream;
  auto initial = stream.resource<int>();
  stream.construct(initial, 0);
  auto state = stream.phi(initial);
  stream.feedback(state, state);
  stream.validate();

  const auto ir = compiler::classify_nodes(compiler::add_resource_order(
      compiler::build_edges(compiler::lower_cutoffs(compiler::resolve_phis(
          compiler::lower_values(compiler::canonicalize(stream)))))));
  ASSERT_EQ(ir.roots.size(), 1);
  EXPECT_EQ(ir.roots[0], 0);
  EXPECT_FALSE(ir.nodes[1].root);
}

TEST(FlowLicm, HoistsImmutableConstructionAcrossAcquire) {
  Stream stream;
  stream.acquire("frame");
  auto initial = stream.resource<int>();
  stream.construct(initial, 0);
  auto state = stream.phi(initial);
  auto current = stream.resource<int>();
  stream.construct(current, 0);
  stream.op(flow::read(state), flow::write(current),
            [](const int &previous, int &next) { next = previous + 1; });
  stream.feedback(state, current);
  stream.validate();

  auto classified = compiler::hoist_loop_invariants(
      compiler::classify_nodes(compiler::add_resource_order(
          compiler::build_edges(compiler::lower_cutoffs(compiler::resolve_phis(
              compiler::lower_values(compiler::canonicalize(stream))))))));
  ASSERT_EQ(classified.nodes.size(), 5);
  EXPECT_FALSE(classified.nodes[0].loop_invariant);
  EXPECT_TRUE(classified.nodes[1].loop_invariant);
  EXPECT_FALSE(classified.nodes[3].loop_invariant);

  const auto graph_ir = compiler::build_graph(std::move(classified));
  bool invariant_precedes_acquire = false;
  bool acquire_precedes_mutable_construction = false;
  for (std::uint32_t index = 0; index < graph_ir.graph.edge_count(); ++index) {
    const graph::EdgeId edge{index};
    invariant_precedes_acquire |= graph_ir.graph.source(edge).value == 1 &&
                                  graph_ir.graph.destination(edge).value == 0;
    acquire_precedes_mutable_construction |=
        graph_ir.graph.source(edge).value == 0 &&
        graph_ir.graph.destination(edge).value == 3;
  }
  EXPECT_TRUE(invariant_precedes_acquire);
  EXPECT_TRUE(acquire_precedes_mutable_construction);
}

TEST(FlowLicm, PropagatesInvarianceThroughConstructionInputs) {
  Stream stream;
  stream.acquire("frame");
  auto first = stream.resource<int>();
  stream.construct(first, 4);
  auto second = stream.resource<int>();
  stream.construct(second, flow::read(first),
                   [](int *output, const int &input) {
                     std::construct_at(output, input + 1);
                   });
  auto transformed = stream.resource<int>();
  stream.construct(transformed, 0);
  stream.op_with_metadata(
      flow::OpMetadata("Transform"), flow::read(second),
      flow::write(transformed),
      [](const int &input, int &output) { output = input * 2; });
  auto external = stream.external<int>("external");
  auto dynamic = stream.resource<int>();
  stream.construct(
      dynamic, flow::read(external),
      [](int *output, const int &input) { std::construct_at(output, input); });
  auto effect_output = stream.resource<int>();
  stream.construct(effect_output, 0);
  stream.op_with_metadata(
      flow::OpMetadata("Effect", "", flow::OpSemantics::side_effect),
      flow::read(second), flow::write(effect_output),
      [](const int &input, int &output) { output = input; });
  stream.validate();

  const auto ir = compiler::hoist_loop_invariants(
      compiler::classify_nodes(compiler::add_resource_order(
          compiler::build_edges(compiler::lower_cutoffs(compiler::resolve_phis(
              compiler::lower_values(compiler::canonicalize(stream))))))));
  ASSERT_EQ(ir.nodes.size(), 8);
  EXPECT_TRUE(ir.nodes[1].loop_invariant);
  EXPECT_TRUE(ir.nodes[2].loop_invariant);
  EXPECT_TRUE(ir.nodes[3].loop_invariant);
  EXPECT_TRUE(ir.nodes[4].loop_invariant);
  EXPECT_FALSE(ir.nodes[5].loop_invariant);
  EXPECT_FALSE(ir.nodes[6].loop_invariant);
  EXPECT_FALSE(ir.nodes[7].loop_invariant);
}

TEST(FlowLicm, HoistsConstructionSeparatedByDiscardFromLoopWrites) {
  Stream stream;
  stream.acquire("frame");
  auto value = stream.resource<int>();
  stream.construct(value, 0);
  stream.discard(value);
  stream.op(flow::write(value), [](int &output) { output = 1; });
  stream.op(flow::read(value), [](const int &) {});
  stream.validate();

  const auto ir = compiler::hoist_loop_invariants(
      compiler::classify_nodes(compiler::add_resource_order(
          compiler::build_edges(compiler::lower_cutoffs(compiler::resolve_phis(
              compiler::lower_values(compiler::canonicalize(stream))))))));
  ASSERT_EQ(ir.nodes.size(), 5);
  ASSERT_EQ(ir.preconstructed_resources.size(), 1);
  EXPECT_TRUE(ir.nodes[1].loop_invariant);
  EXPECT_TRUE(ir.preconstructed_resources[0]);
  EXPECT_FALSE(ir.nodes[2].loop_invariant);
  EXPECT_FALSE(ir.nodes[3].loop_invariant);
}

TEST(FlowBuildGraph, BuildsDenseNodesAndAllDependencyKinds) {
  Stream stream;
  auto initial = stream.resource<int>();
  stream.construct(initial, 0);
  auto state = stream.phi(initial);
  auto next = stream.resource<int>();
  stream.construct(next, 1);
  stream.op(flow::read(state), flow::write(next),
            [](const int &state, int &next) { next = state + 1; });
  stream.feedback(state, next);
  stream.op(flow::read(next), [](const int &) {});
  stream.op(flow::write(next), [](int &next) { ++next; });
  stream.validate();

  const auto ir = compiler::build_graph(
      compiler::classify_nodes(compiler::add_resource_order(
          compiler::build_edges(compiler::lower_cutoffs(compiler::resolve_phis(
              compiler::lower_values(compiler::canonicalize(stream))))))));
  EXPECT_EQ(ir.graph.node_count(), ir.classified.nodes.size());

  bool found_value = false;
  bool found_order = false;
  bool found_temporal = false;
  for (std::uint32_t index = 0; index < ir.graph.edge_count(); ++index) {
    const auto &edge = ir.graph.get(graph::EdgeId{index});
    found_value |= edge.kind == compiler::DependencyKind::value;
    found_order |= edge.kind == compiler::DependencyKind::resource_order;
    found_temporal |= edge.kind == compiler::DependencyKind::temporal;
  }
  EXPECT_TRUE(found_value);
  EXPECT_TRUE(found_order);
  EXPECT_TRUE(found_temporal);
}

TEST(FlowBuildGraph, PreservesDistinctValuesBetweenTheSameNodes) {
  Stream stream;
  auto first = stream.resource<int>();
  auto second = stream.resource<int>();
  stream.construct(first, 0);
  stream.construct(second, 0);
  stream.op(flow::write(first), flow::write(second),
            [](int &first, int &second) {
              first = 1;
              second = 2;
            });
  stream.op(flow::read(first), flow::read(second),
            [](const int &, const int &) {});
  stream.validate();

  const auto ir = compiler::build_graph(
      compiler::classify_nodes(compiler::add_resource_order(
          compiler::build_edges(compiler::lower_cutoffs(compiler::resolve_phis(
              compiler::lower_values(compiler::canonicalize(stream))))))));
  std::size_t shared_edges = 0;
  for (const auto edge : ir.graph.outgoing(graph::NodeId{2})) {
    if (ir.graph.destination(edge) == graph::NodeId{3} &&
        ir.graph.get(edge).kind == compiler::DependencyKind::value) {
      ++shared_edges;
    }
  }
  EXPECT_EQ(shared_edges, 2);
}

TEST(FlowBuildGraph, KeepsSourceLessTemporalInputsOutOfNodeEdges) {
  Stream stream;
  auto initial = stream.resource<int>();
  stream.construct(initial, 0);
  auto state = stream.phi(initial);
  auto external = stream.external<int>("external");
  stream.feedback(state, external);
  stream.validate();

  const auto ir = compiler::build_graph(
      compiler::classify_nodes(compiler::add_resource_order(
          compiler::build_edges(compiler::lower_cutoffs(compiler::resolve_phis(
              compiler::lower_values(compiler::canonicalize(stream))))))));
  ASSERT_EQ(
      ir.classified.ordered.dependencies.lowered.resolved.backedges.size(), 1);
  for (std::uint32_t index = 0; index < ir.graph.edge_count(); ++index) {
    EXPECT_NE(ir.graph.get(graph::EdgeId{index}).kind,
              compiler::DependencyKind::temporal);
  }
}

TEST(FlowComponents, FindsFeedbackComponentsAndBuildsACondensationDag) {
  Stream stream;
  auto initial = stream.resource<int>();
  stream.construct(initial, 0);
  auto state = stream.phi(initial);
  auto next = stream.resource<int>();
  stream.construct(next, 1);
  stream.op(flow::read(state), flow::write(next),
            [](const int &state, int &next) { next = state + 1; });
  stream.feedback(state, next);
  stream.op(flow::read(next), [](const int &) {});
  stream.validate();

  const auto ir = compiler::analyze_components(compiler::build_graph(
      compiler::classify_nodes(compiler::add_resource_order(
          compiler::build_edges(compiler::lower_cutoffs(compiler::resolve_phis(
              compiler::lower_values(compiler::canonicalize(stream)))))))));
  const auto phi_component = ir.component_of[1];
  const auto update_component = ir.component_of[3];
  EXPECT_EQ(phi_component, update_component);
  EXPECT_TRUE(ir.components[phi_component].cyclic);
  EXPECT_TRUE(ir.components[phi_component].has_temporal_edge);
  EXPECT_EQ(ir.components[phi_component].nodes.count, 2);
  EXPECT_TRUE(graph::topological_sort(ir.condensation).acyclic);
  EXPECT_EQ(ir.topological_order.size(), ir.components.size());
}

TEST(FlowComponents, RecognizesTemporalSelfLoops) {
  Stream stream;
  auto initial = stream.resource<int>();
  stream.construct(initial, 0);
  auto state = stream.phi(initial);
  stream.feedback(state, state);
  stream.validate();

  const auto ir = compiler::analyze_components(compiler::build_graph(
      compiler::classify_nodes(compiler::add_resource_order(
          compiler::build_edges(compiler::lower_cutoffs(compiler::resolve_phis(
              compiler::lower_values(compiler::canonicalize(stream)))))))));
  const auto component = ir.component_of[1];
  EXPECT_TRUE(ir.components[component].cyclic);
  EXPECT_TRUE(ir.components[component].has_temporal_edge);
  EXPECT_EQ(ir.components[component].nodes.count, 1);
}

TEST(FlowComponents, RejectsImmediateCycles) {
  using Builder =
      graph::AdjGraph<compiler::DependencyNode, compiler::DependencyEdge,
                      flow::allocator_ref>;
  Builder builder{flow::allocator_ref{}};
  const compiler::NodeClassification classification{};
  builder.add_node({0, compiler::NodeKind::callback, classification});
  builder.add_node({1, compiler::NodeKind::callback, classification});
  const compiler::DependencyEdge dependency{
      .kind = compiler::DependencyKind::value,
      .value = 0,
      .resource = flow::null_resource_id,
  };
  builder.add_edge(graph::NodeId{0}, graph::NodeId{1}, dependency);
  builder.add_edge(graph::NodeId{1}, graph::NodeId{0}, dependency);
  compiler::CompactDependencyGraph compact_graph(std::move(builder),
                                                 flow::allocator_ref{});
  compiler::ClassifiedIR classified;
  compiler::GraphIR input(std::move(classified), std::move(compact_graph));

  EXPECT_THROW(compiler::analyze_components(std::move(input)),
               std::logic_error);
}

TEST(FlowComponents, RejectsImmediateSubcyclesInsideFeedbackComponents) {
  using Builder =
      graph::AdjGraph<compiler::DependencyNode, compiler::DependencyEdge,
                      flow::allocator_ref>;
  Builder builder{flow::allocator_ref{}};
  const compiler::NodeClassification classification{};
  for (flow::detail::NodeId node = 0; node < 3; ++node) {
    builder.add_node({node, compiler::NodeKind::callback, classification});
  }
  const compiler::DependencyEdge immediate{
      .kind = compiler::DependencyKind::value,
      .value = 0,
      .resource = flow::null_resource_id,
  };
  const compiler::DependencyEdge temporal{
      .kind = compiler::DependencyKind::temporal,
      .value = 1,
      .resource = flow::null_resource_id,
  };
  builder.add_edge(graph::NodeId{0}, graph::NodeId{1}, immediate);
  builder.add_edge(graph::NodeId{1}, graph::NodeId{0}, immediate);
  builder.add_edge(graph::NodeId{1}, graph::NodeId{2}, immediate);
  builder.add_edge(graph::NodeId{2}, graph::NodeId{0}, temporal);
  compiler::CompactDependencyGraph compact_graph(std::move(builder),
                                                 flow::allocator_ref{});
  compiler::ClassifiedIR classified;
  compiler::GraphIR input(std::move(classified), std::move(compact_graph));

  EXPECT_THROW(compiler::analyze_components(std::move(input)),
               std::logic_error);
}

TEST(FlowComponents, MergesDependencyKindsBetweenComponents) {
  using Builder =
      graph::AdjGraph<compiler::DependencyNode, compiler::DependencyEdge,
                      flow::allocator_ref>;
  Builder builder{flow::allocator_ref{}};
  const compiler::NodeClassification classification{};
  builder.add_node({0, compiler::NodeKind::callback, classification});
  builder.add_node({1, compiler::NodeKind::callback, classification});
  builder.add_edge(
      graph::NodeId{0}, graph::NodeId{1},
      {compiler::DependencyKind::value, 0, flow::null_resource_id});
  builder.add_edge(graph::NodeId{0}, graph::NodeId{1},
                   {compiler::DependencyKind::resource_order,
                    flow::detail::null_value_id, 0});
  compiler::CompactDependencyGraph compact_graph(std::move(builder),
                                                 flow::allocator_ref{});
  compiler::ClassifiedIR classified;
  compiler::GraphIR input(std::move(classified), std::move(compact_graph));

  const auto ir = compiler::analyze_components(std::move(input));
  ASSERT_EQ(ir.condensation.edge_count(), 1);
  const auto &edge = ir.condensation.get(graph::EdgeId{0});
  EXPECT_TRUE(edge.value);
  EXPECT_TRUE(edge.resource_order);
  EXPECT_FALSE(edge.temporal);
}

TEST(FlowActivation, ClassifiesConstantChainsAsRunningOnce) {
  Stream stream;
  auto first = stream.resource<int>();
  auto second = stream.resource<int>();
  stream.construct(first, 1);
  stream.construct(
      second, flow::read(first),
      [](int *second, const int &first) { std::construct_at(second, first); });
  stream.op(flow::read(second), [](const int &) {});
  stream.validate();

  const auto ir = compiler::analyze_activation(
      compiler::analyze_components(compiler::build_graph(
          compiler::classify_nodes(compiler::add_resource_order(
              compiler::build_edges(compiler::lower_cutoffs(
                  compiler::resolve_phis(compiler::lower_values(
                      compiler::canonicalize(stream))))))))));
  for (const auto &component : ir.components) {
    EXPECT_TRUE(component.runs_once);
    EXPECT_FALSE(component.externally_triggered);
    EXPECT_EQ(component.feedback_sources.count, 0);
  }
}

TEST(FlowActivation, PropagatesMultipleFeedbackSources) {
  Stream stream;
  auto first_initial = stream.resource<int>();
  auto second_initial = stream.resource<int>();
  stream.construct(first_initial, 1);
  stream.construct(second_initial, 2);
  auto first = stream.phi(first_initial);
  auto second = stream.phi(second_initial);
  stream.feedback(first, first);
  stream.feedback(second, second);
  stream.op(flow::read(first), flow::read(second),
            [](const int &, const int &) {});
  stream.validate();

  const auto ir = compiler::analyze_activation(
      compiler::analyze_components(compiler::build_graph(
          compiler::classify_nodes(compiler::add_resource_order(
              compiler::build_edges(compiler::lower_cutoffs(
                  compiler::resolve_phis(compiler::lower_values(
                      compiler::canonicalize(stream))))))))));
  const auto consumer_component = ir.input.component_of[4];
  const auto &activation = ir.components[consumer_component];
  EXPECT_FALSE(activation.runs_once);
  ASSERT_EQ(activation.feedback_sources.count, 2);
  const auto first_component = ir.input.component_of[2];
  const auto second_component = ir.input.component_of[3];
  const auto first_source =
      ir.feedback_sources[activation.feedback_sources.offset];
  const auto second_source =
      ir.feedback_sources[activation.feedback_sources.offset + 1];
  EXPECT_TRUE(
      (first_source == first_component && second_source == second_component) ||
      (first_source == second_component && second_source == first_component));
}

TEST(FlowActivation, PropagatesExternalAndCutoffActivation) {
  Stream stream;
  auto external = stream.external<int>("external");
  auto stable = stream.cutoff(external);
  auto output = stream.resource<int>();
  stream.construct(output, flow::read(stable),
                   [](int *output, const int &stable) {
                     std::construct_at(output, stable);
                   });
  stream.op(flow::read(output), [](const int &) {});
  stream.validate();

  const auto ir = compiler::analyze_activation(
      compiler::analyze_components(compiler::build_graph(
          compiler::classify_nodes(compiler::add_resource_order(
              compiler::build_edges(compiler::lower_cutoffs(
                  compiler::resolve_phis(compiler::lower_values(
                      compiler::canonicalize(stream))))))))));
  const auto cutoff_component = ir.input.component_of[0];
  const auto construct_component = ir.input.component_of[1];
  const auto consumer_component = ir.input.component_of[2];
  EXPECT_TRUE(ir.components[cutoff_component].externally_triggered);
  EXPECT_TRUE(ir.components[cutoff_component].memoized);
  EXPECT_TRUE(ir.components[construct_component].externally_triggered);
  EXPECT_TRUE(ir.components[construct_component].memoized);
  EXPECT_TRUE(ir.components[consumer_component].memoized);
  EXPECT_FALSE(ir.components[consumer_component].runs_once);
}

TEST(FlowActivation, PropagatesYieldReachabilityBackward) {
  Stream stream;
  auto value = stream.resource<int>();
  stream.construct(value, 1);
  stream.op(flow::read(value), [](const int &) {});
  stream.yield();
  stream.validate();

  const auto ir = compiler::analyze_activation(
      compiler::analyze_components(compiler::build_graph(
          compiler::classify_nodes(compiler::add_resource_order(
              compiler::build_edges(compiler::lower_cutoffs(
                  compiler::resolve_phis(compiler::lower_values(
                      compiler::canonicalize(stream))))))))));
  const auto construct_component = ir.input.component_of[0];
  const auto callback_component = ir.input.component_of[1];
  const auto yield_component = ir.input.component_of[2];
  EXPECT_TRUE(ir.components[yield_component].contains_yield);
  EXPECT_TRUE(ir.components[yield_component].reaches_yield);
  EXPECT_FALSE(ir.components[callback_component].reaches_yield);
  EXPECT_FALSE(ir.components[construct_component].reaches_yield);
}

TEST(FlowStorage, ClassifiesFiniteOwnedValuesAsTransient) {
  Stream stream;
  auto first = stream.resource<int>();
  auto second = stream.resource<int>();
  stream.construct(first, 1);
  stream.construct(
      second, flow::read(first),
      [](int *second, const int &first) { std::construct_at(second, first); });
  stream.validate();

  const auto ir = compiler::classify_storage(compiler::analyze_activation(
      compiler::analyze_components(compiler::build_graph(
          compiler::classify_nodes(compiler::add_resource_order(
              compiler::build_edges(compiler::lower_cutoffs(
                  compiler::resolve_phis(compiler::lower_values(
                      compiler::canonicalize(stream)))))))))));
  ASSERT_EQ(ir.values.size(), 2);
  EXPECT_EQ(ir.values[0].storage, compiler::StorageClass::transient);
  EXPECT_EQ(ir.values[1].storage, compiler::StorageClass::transient);
  EXPECT_EQ(ir.resources[0].storage, compiler::StorageClass::transient);
  EXPECT_EQ(ir.resources[1].storage, compiler::StorageClass::transient);
}

TEST(FlowStorage, RetainsFeedbackAndCrossActivationValues) {
  Stream stream;
  auto initial = stream.resource<int>();
  stream.construct(initial, 0);
  auto state = stream.phi(initial);
  auto next = stream.resource<int>();
  stream.construct(next, 1);
  stream.op(flow::read(state), flow::write(next),
            [](const int &state, int &next) { next = state + 1; });
  stream.feedback(state, next);
  stream.validate();

  const auto ir = compiler::classify_storage(compiler::analyze_activation(
      compiler::analyze_components(compiler::build_graph(
          compiler::classify_nodes(compiler::add_resource_order(
              compiler::build_edges(compiler::lower_cutoffs(
                  compiler::resolve_phis(compiler::lower_values(
                      compiler::canonicalize(stream)))))))))));
  ASSERT_EQ(ir.values.size(), 4);
  EXPECT_EQ(ir.values[0].storage, compiler::StorageClass::persistent);
  EXPECT_TRUE(ir.values[0].crosses_activation);
  EXPECT_EQ(ir.values[1].storage, compiler::StorageClass::virtual_value);
  EXPECT_EQ(ir.values[3].storage, compiler::StorageClass::persistent);
  EXPECT_TRUE(ir.values[3].retained_by_feedback);
  EXPECT_EQ(ir.resources[0].storage, compiler::StorageClass::persistent);
  EXPECT_EQ(ir.resources[2].storage, compiler::StorageClass::persistent);
}

TEST(FlowStorage, SeparatesExternalAndVirtualValues) {
  Stream stream;
  auto external = stream.external<int>("external");
  auto stable = stream.cutoff(external);
  stream.op(flow::read(stable), [](const int &) {});
  stream.validate();

  const auto ir = compiler::classify_storage(compiler::analyze_activation(
      compiler::analyze_components(compiler::build_graph(
          compiler::classify_nodes(compiler::add_resource_order(
              compiler::build_edges(compiler::lower_cutoffs(
                  compiler::resolve_phis(compiler::lower_values(
                      compiler::canonicalize(stream)))))))))));
  ASSERT_EQ(ir.values.size(), 2);
  EXPECT_EQ(ir.values[0].storage, compiler::StorageClass::external);
  EXPECT_EQ(ir.values[1].storage, compiler::StorageClass::virtual_value);
  EXPECT_EQ(ir.resources[0].storage, compiler::StorageClass::external);
  EXPECT_EQ(ir.resources[1].storage, compiler::StorageClass::virtual_value);
}

TEST(FlowStorage, DoesNotRetainCutoffInputs) {
  Stream stream;
  auto source = stream.resource<int>();
  stream.construct(source, 1);
  stream.cutoff(source);
  stream.validate();

  const auto ir = compiler::classify_storage(compiler::analyze_activation(
      compiler::analyze_components(compiler::build_graph(
          compiler::classify_nodes(compiler::add_resource_order(
              compiler::build_edges(compiler::lower_cutoffs(
                  compiler::resolve_phis(compiler::lower_values(
                      compiler::canonicalize(stream)))))))))));
  EXPECT_EQ(ir.values[0].storage, compiler::StorageClass::transient);
}

TEST(FlowStorage, RetainsOutputsOfMemoizedOperations) {
  Stream stream;
  auto external = stream.external<int>("external");
  auto stable = stream.cutoff(external);
  auto result = stream.resource<int>();
  stream.construct(
      result, flow::read(stable),
      [](int *output, const int &input) { std::construct_at(output, input); });
  stream.validate();

  const auto ir = compiler::classify_storage(compiler::analyze_activation(
      compiler::analyze_components(compiler::build_graph(
          compiler::classify_nodes(compiler::add_resource_order(
              compiler::build_edges(compiler::lower_cutoffs(
                  compiler::resolve_phis(compiler::lower_values(
                      compiler::canonicalize(stream)))))))))));
  EXPECT_EQ(ir.values.back().storage, compiler::StorageClass::persistent);
  EXPECT_TRUE(ir.values.back().retained_by_memoization);
  EXPECT_EQ(ir.resources.back().storage, compiler::StorageClass::persistent);
}

TEST(FlowLifetimes, BuildsDefinitionUseAndLastUseRanges) {
  Stream stream;
  auto first = stream.resource<int>();
  auto second = stream.resource<int>();
  stream.construct(first, 1);
  stream.construct(
      second, flow::read(first),
      [](int *second, const int &first) { std::construct_at(second, first); });
  stream.op(flow::read(second), [](const int &) {});
  stream.validate();

  const auto ir = compiler::analyze_lifetimes(
      compiler::classify_storage(compiler::analyze_activation(
          compiler::analyze_components(compiler::build_graph(
              compiler::classify_nodes(compiler::add_resource_order(
                  compiler::build_edges(compiler::lower_cutoffs(
                      compiler::resolve_phis(compiler::lower_values(
                          compiler::canonicalize(stream))))))))))));
  ASSERT_EQ(ir.resources.size(), 2);
  EXPECT_EQ(ir.resources[0].definitions.count, 1);
  EXPECT_EQ(ir.definitions[ir.resources[0].definitions.offset], 0);
  EXPECT_EQ(ir.resources[0].uses.count, 2);
  ASSERT_EQ(ir.resources[0].last_uses.count, 1);
  EXPECT_EQ(ir.last_uses[ir.resources[0].last_uses.offset], 1);
  ASSERT_EQ(ir.resources[1].last_uses.count, 1);
  EXPECT_EQ(ir.last_uses[ir.resources[1].last_uses.offset], 2);
}

TEST(FlowLifetimes, KeepsParallelTerminalReadersInTheFrontier) {
  Stream stream;
  auto value = stream.resource<int>();
  stream.construct(value, 1);
  stream.op(flow::read(value), [](const int &) {});
  stream.op(flow::read(value), [](const int &) {});
  stream.validate();

  const auto ir = compiler::analyze_lifetimes(
      compiler::classify_storage(compiler::analyze_activation(
          compiler::analyze_components(compiler::build_graph(
              compiler::classify_nodes(compiler::add_resource_order(
                  compiler::build_edges(compiler::lower_cutoffs(
                      compiler::resolve_phis(compiler::lower_values(
                          compiler::canonicalize(stream))))))))))));
  const auto lifetime = ir.resources[0];
  ASSERT_EQ(lifetime.last_uses.count, 2);
  EXPECT_EQ(ir.last_uses[lifetime.last_uses.offset], 1);
  EXPECT_EQ(ir.last_uses[lifetime.last_uses.offset + 1], 2);
}

TEST(FlowLifetimes, RetainsPersistentFeedbackResourcesUntilShutdown) {
  Stream stream;
  auto initial = stream.resource<int>();
  stream.construct(initial, 0);
  auto state = stream.phi(initial);
  auto next = stream.resource<int>();
  stream.construct(next, 1);
  stream.op(flow::read(state), flow::write(next),
            [](const int &state, int &next) { next = state + 1; });
  stream.feedback(state, next);
  stream.validate();

  const auto ir = compiler::analyze_lifetimes(
      compiler::classify_storage(compiler::analyze_activation(
          compiler::analyze_components(compiler::build_graph(
              compiler::classify_nodes(compiler::add_resource_order(
                  compiler::build_edges(compiler::lower_cutoffs(
                      compiler::resolve_phis(compiler::lower_values(
                          compiler::canonicalize(stream))))))))))));
  EXPECT_TRUE(ir.resources[0].destroy_at_shutdown);
  EXPECT_TRUE(ir.resources[2].destroy_at_shutdown);
  EXPECT_FALSE(ir.resources[1].destroy_at_shutdown);
  EXPECT_EQ(ir.resources[2].activation_sources.count, 1);
}

TEST(FlowLifetimes, DetectsPotentiallyOverlappingTransientGenerations) {
  Stream stream;
  auto initial = stream.resource<int>();
  stream.construct(initial, 0);
  auto state = stream.phi(initial);
  auto scratch = stream.resource<int>();
  stream.construct(scratch, flow::read(state),
                   [](int *scratch, const int &state) {
                     std::construct_at(scratch, state);
                   });
  auto next = stream.resource<int>();
  stream.construct(next, 0);
  stream.op(flow::read(scratch), flow::write(next),
            [](const int &scratch, int &next) { next = scratch + 1; });
  stream.feedback(state, next);
  stream.validate();

  const auto ir = compiler::analyze_lifetimes(
      compiler::classify_storage(compiler::analyze_activation(
          compiler::analyze_components(compiler::build_graph(
              compiler::classify_nodes(compiler::add_resource_order(
                  compiler::build_edges(compiler::lower_cutoffs(
                      compiler::resolve_phis(compiler::lower_values(
                          compiler::canonicalize(stream))))))))))));
  EXPECT_EQ(ir.resources[2].storage, compiler::StorageClass::transient);
  EXPECT_TRUE(ir.resources[2].may_overlap_generations);
  EXPECT_EQ(ir.resources[2].activation_sources.count, 1);
}

TEST(FlowInterference, OmitsEdgesForStrictlyOrderedLifetimes) {
  Stream stream;
  auto first = stream.resource<int>();
  auto bridge = stream.resource<int>();
  auto second = stream.resource<int>();
  stream.construct(first, 1);
  stream.construct(
      bridge, flow::read(first),
      [](int *bridge, const int &first) { std::construct_at(bridge, first); });
  stream.construct(second, flow::read(bridge),
                   [](int *second, const int &bridge) {
                     std::construct_at(second, bridge);
                   });
  stream.validate();

  const auto ir = compiler::build_interference(compiler::analyze_lifetimes(
      compiler::classify_storage(compiler::analyze_activation(
          compiler::analyze_components(compiler::build_graph(
              compiler::classify_nodes(compiler::add_resource_order(
                  compiler::build_edges(compiler::lower_cutoffs(
                      compiler::resolve_phis(compiler::lower_values(
                          compiler::canonicalize(stream)))))))))))));
  ASSERT_EQ(ir.graph.node_count(), 3);
  bool first_interferes_with_second = false;
  for (const auto edge : ir.graph.outgoing(ir.resource_to_node[0])) {
    first_interferes_with_second |=
        ir.graph.destination(edge) == ir.resource_to_node[2];
  }
  EXPECT_FALSE(first_interferes_with_second);
  EXPECT_EQ(ir.graph.edge_count(), 2);
}

TEST(FlowInterference, ConnectsUnorderedTransientResources) {
  Stream stream;
  auto first = stream.resource<int>();
  auto second = stream.resource<float>();
  stream.construct(first, 1);
  stream.construct(second, 2.0F);
  stream.validate();

  const auto ir = compiler::build_interference(compiler::analyze_lifetimes(
      compiler::classify_storage(compiler::analyze_activation(
          compiler::analyze_components(compiler::build_graph(
              compiler::classify_nodes(compiler::add_resource_order(
                  compiler::build_edges(compiler::lower_cutoffs(
                      compiler::resolve_phis(compiler::lower_values(
                          compiler::canonicalize(stream)))))))))))));
  ASSERT_EQ(ir.graph.edge_count(), 1);
  EXPECT_EQ(ir.graph.source(graph::EdgeId{0}), ir.resource_to_node[0]);
  EXPECT_EQ(ir.graph.destination(graph::EdgeId{0}), ir.resource_to_node[1]);
  EXPECT_EQ(ir.graph.get(ir.resource_to_node[0]).size, sizeof(int));
  EXPECT_EQ(ir.graph.get(ir.resource_to_node[1]).alignment, alignof(float));
}

TEST(FlowInterference, ExcludesExternalAndVirtualResources) {
  Stream stream;
  auto external = stream.external<int>("external");
  auto stable = stream.cutoff(external);
  auto owned = stream.resource<int>();
  stream.construct(
      owned, flow::read(stable),
      [](int *owned, const int &stable) { std::construct_at(owned, stable); });
  stream.validate();

  const auto ir = compiler::build_interference(compiler::analyze_lifetimes(
      compiler::classify_storage(compiler::analyze_activation(
          compiler::analyze_components(compiler::build_graph(
              compiler::classify_nodes(compiler::add_resource_order(
                  compiler::build_edges(compiler::lower_cutoffs(
                      compiler::resolve_phis(compiler::lower_values(
                          compiler::canonicalize(stream)))))))))))));
  EXPECT_EQ(ir.resource_to_node[0], compiler::null_interference_node);
  EXPECT_EQ(ir.resource_to_node[1], compiler::null_interference_node);
  EXPECT_NE(ir.resource_to_node[2], compiler::null_interference_node);
  EXPECT_EQ(ir.graph.node_count(), 1);
}

TEST(FlowInterference, ConservativelySeparatesPersistentResources) {
  Stream stream;
  auto first_initial = stream.resource<int>();
  auto second_initial = stream.resource<int>();
  stream.construct(first_initial, 1);
  stream.construct(second_initial, 2);
  auto first = stream.phi(first_initial);
  auto second = stream.phi(second_initial);
  stream.feedback(first, first);
  stream.feedback(second, second);
  stream.validate();

  const auto ir = compiler::build_interference(compiler::analyze_lifetimes(
      compiler::classify_storage(compiler::analyze_activation(
          compiler::analyze_components(compiler::build_graph(
              compiler::classify_nodes(compiler::add_resource_order(
                  compiler::build_edges(compiler::lower_cutoffs(
                      compiler::resolve_phis(compiler::lower_values(
                          compiler::canonicalize(stream)))))))))))));
  EXPECT_EQ(ir.input.resources[0].storage, compiler::StorageClass::persistent);
  EXPECT_EQ(ir.input.resources[1].storage, compiler::StorageClass::persistent);
  ASSERT_EQ(ir.graph.node_count(), 2);
  EXPECT_EQ(ir.graph.edge_count(), 1);
}

TEST(FlowSlots, UsesTwoSlotsForFeedbackWithoutAYield) {
  Stream stream;
  auto initial = stream.resource<int>();
  stream.construct(initial, 0);
  auto state = stream.phi(initial);
  auto next = stream.resource<int>();
  stream.construct(next, flow::read(state), [](int *next, const int &state) {
    std::construct_at(next, state + 1);
  });
  stream.feedback(state, next);
  stream.validate();

  const auto ir = compiler::assign_slots(
      compiler::build_interference(compiler::analyze_lifetimes(
          compiler::classify_storage(compiler::analyze_activation(
              compiler::analyze_components(compiler::build_graph(
                  compiler::classify_nodes(compiler::add_resource_order(
                      compiler::build_edges(compiler::lower_cutoffs(
                          compiler::resolve_phis(compiler::lower_values(
                              compiler::canonicalize(stream))))))))))))));
  EXPECT_EQ(ir.resources[0].slot_count, 1);
  EXPECT_EQ(ir.resources[2].slot_count, 2);
}

TEST(FlowSlots, UsesConfiguredBoundForCyclicDefinitionsAcrossYield) {
  Stream stream;
  auto initial = stream.resource<int>();
  stream.construct(initial, 0);
  auto state = stream.phi(initial);
  auto next = stream.resource<int>();
  stream.construct(next, flow::read(state), [](int *next, const int &state) {
    std::construct_at(next, state + 1);
  });
  stream.yield();
  stream.feedback(state, next);
  stream.validate();

  const auto ir = compiler::assign_slots(
      compiler::build_interference(compiler::analyze_lifetimes(
          compiler::classify_storage(compiler::analyze_activation(
              compiler::analyze_components(compiler::build_graph(
                  compiler::classify_nodes(compiler::add_resource_order(
                      compiler::build_edges(compiler::lower_cutoffs(
                          compiler::resolve_phis(compiler::lower_values(
                              compiler::canonicalize(stream))))))))))))),
      flow::CompileOptions{.max_slots = 4});
  EXPECT_EQ(ir.resources[0].slot_count, 1);
  EXPECT_FALSE(ir.resources[0].bounded_overlap);
  EXPECT_EQ(ir.resources[1].slot_count, 0);
  EXPECT_EQ(ir.resources[2].slot_count, 4);
  EXPECT_TRUE(ir.resources[2].bounded_overlap);
}

TEST(FlowSlots, UsesTheFeedbackBoundWhenYieldIsOutsideTheInterval) {
  Stream stream;
  stream.yield();
  auto initial = stream.resource<int>();
  stream.construct(initial, 0);
  auto state = stream.phi(initial);
  auto next = stream.resource<int>();
  stream.construct(next, flow::read(state), [](int *next, const int &state) {
    std::construct_at(next, state + 1);
  });
  stream.feedback(state, next);
  stream.validate();

  const auto ir = compiler::assign_slots(
      compiler::build_interference(compiler::analyze_lifetimes(
          compiler::classify_storage(compiler::analyze_activation(
              compiler::analyze_components(compiler::build_graph(
                  compiler::classify_nodes(compiler::add_resource_order(
                      compiler::build_edges(compiler::lower_cutoffs(
                          compiler::resolve_phis(compiler::lower_values(
                              compiler::canonicalize(stream))))))))))))),
      flow::CompileOptions{.max_slots = 4});
  EXPECT_EQ(ir.resources[2].slot_count, 2);
  EXPECT_TRUE(ir.resources[2].bounded_overlap);
}

TEST(FlowSlots, UsesConfiguredBoundForAcquireDrivenResources) {
  Stream stream;
  stream.acquire("frame");
  auto value = stream.resource<int>();
  stream.construct(value, 1);
  stream.op(flow::read(value), [](const int &) {});
  stream.validate();

  const auto ir = compiler::assign_slots(
      compiler::build_interference(compiler::analyze_lifetimes(
          compiler::classify_storage(compiler::analyze_activation(
              compiler::analyze_components(compiler::build_graph(
                  compiler::classify_nodes(compiler::add_resource_order(
                      compiler::build_edges(compiler::lower_cutoffs(
                          compiler::resolve_phis(compiler::lower_values(
                              compiler::canonicalize(stream))))))))))))),
      flow::CompileOptions{.max_slots = 4});
  EXPECT_EQ(ir.resources[0].slot_count, 4);
  EXPECT_TRUE(ir.resources[0].bounded_overlap);
}

TEST(FlowSlots, RejectsAZeroSlotLimit) {
  Stream stream;
  stream.validate();
  auto normalized = compiler::canonicalize(stream);
  auto values = compiler::lower_values(std::move(normalized));
  auto resolved = compiler::resolve_phis(std::move(values));
  auto cutoffs = compiler::lower_cutoffs(std::move(resolved));
  auto dependencies = compiler::build_edges(std::move(cutoffs));
  auto ordered = compiler::add_resource_order(std::move(dependencies));
  auto classified = compiler::classify_nodes(std::move(ordered));
  auto graph_ir = compiler::build_graph(std::move(classified));
  auto components = compiler::analyze_components(std::move(graph_ir));
  auto activation = compiler::analyze_activation(std::move(components));
  auto storage = compiler::classify_storage(std::move(activation));
  auto lifetimes = compiler::analyze_lifetimes(std::move(storage));
  auto interference = compiler::build_interference(std::move(lifetimes));
  EXPECT_THROW(compiler::assign_slots(std::move(interference),
                                      flow::CompileOptions{.max_slots = 0}),
               std::invalid_argument);
}

TEST(FlowResolvePhis, RequiresAcquireForUnconditionalFeedback) {
  Stream stream;
  auto initial = stream.resource<int>();
  stream.construct(initial, 0);
  auto state = stream.phi(initial);
  auto next = stream.resource<int>();
  stream.construct(next, 1);
  stream.feedback(state, next);

  const auto ir = compiler::resolve_phis(
      compiler::lower_values(compiler::canonicalize(stream)));
  EXPECT_THROW(compiler::validate_feedback_loops(ir), std::logic_error);
}

TEST(FlowResolvePhis, RejectsBoundariesInsideWhile) {
  Stream stream;
  auto condition = stream.resource<bool>();
  stream.construct(condition, true);
  stream.while_(condition, [&] {
    stream.acquire("step");
    stream.op(flow::write(condition), [](bool &repeat) { repeat = false; });
  });

  const auto ir = compiler::resolve_phis(
      compiler::lower_values(compiler::canonicalize(stream)));
  EXPECT_THROW(compiler::validate_feedback_loops(ir), std::logic_error);
}

} // namespace
