#pragma once

#include "strobe/flow/detail/compiler/graph_ir.hpp"

#include <strobe/core/graph.hpp>

#include <stdexcept>
#include <utility>

namespace strobe::flow::detail::compiler {

inline GraphIR build_graph(ClassifiedIR classified) {
  using Builder =
      graph::AdjGraph<DependencyNode, DependencyEdge, allocator_ref>;
  Builder builder{allocator_ref{}};
  const auto &ordered = classified.ordered;
  const auto &dependencies = ordered.dependencies;
  const auto &resolved = dependencies.lowered.resolved;
  const auto &values = resolved.values;

  for (NodeId node = 0; node < values.nodes.size(); ++node) {
    const auto id = builder.add_node(DependencyNode{
        .source = node,
        .kind = values.nodes[node].kind,
        .classification = classified.nodes[node],
    });
    if (id.value != node) {
      throw std::logic_error("dependency graph node mapping is not dense");
    }
  }

  const auto add_unique = [&](NodeId source, NodeId destination,
                              DependencyEdge payload) {
    if (source >= values.nodes.size() || destination >= values.nodes.size()) {
      throw std::logic_error("dependency graph edge is out of range");
    }
    for (std::uint32_t index = 0; index < builder.edge_capacity(); ++index) {
      const graph::EdgeId edge{index};
      if (!builder.contains(edge) || builder.source(edge).value != source ||
          builder.destination(edge).value != destination) {
        continue;
      }
      const auto &existing = builder.get(edge);
      if (existing.kind == payload.kind && existing.value == payload.value &&
          existing.resource == payload.resource) {
        return;
      }
    }
    builder.add_edge(graph::NodeId{source}, graph::NodeId{destination},
                     payload);
  };

  for (const auto &edge : dependencies.edges) {
    add_unique(edge.producer, edge.consumer,
               DependencyEdge{
                   .kind = DependencyKind::value,
                   .value = edge.value,
                   .resource = null_resource_id,
               });
  }
  for (const auto &constraint : ordered.resource_order) {
    add_unique(constraint.before, constraint.after,
               DependencyEdge{
                   .kind = DependencyKind::resource_order,
                   .value = null_value_id,
                   .resource = constraint.resource,
               });
  }
  for (const auto &backedge : resolved.backedges) {
    if (backedge.value >= values.values.size() ||
        backedge.target >= values.nodes.size()) {
      throw std::logic_error("temporal dependency is out of range");
    }
    const auto producer = values.values[backedge.value].producer;
    if (producer == null_node_id) {
      continue;
    }
    add_unique(producer, backedge.target,
               DependencyEdge{
                   .kind = DependencyKind::temporal,
                   .value = backedge.value,
                   .resource = null_resource_id,
               });
    if (backedge.condition != null_value_id) {
      if (backedge.condition >= values.values.size()) {
        throw std::logic_error("temporal condition is out of range");
      }
      const auto condition_producer =
          values.values[backedge.condition].producer;
      if (condition_producer == null_node_id ||
          condition_producer >= values.nodes.size()) {
        throw std::logic_error("temporal condition producer is invalid");
      }
      add_unique(condition_producer, backedge.target,
                 DependencyEdge{
                     .kind = DependencyKind::temporal,
                     .value = backedge.condition,
                     .resource = null_resource_id,
                 });
    }
  }

  for (NodeId acquire = 0; acquire < values.nodes.size(); ++acquire) {
    const auto &gate = values.nodes[acquire];
    if (gate.kind != NodeKind::acquire ||
        gate.source_operation == null_source_operation_id) {
      continue;
    }
    for (NodeId node = acquire + 1; node < values.nodes.size(); ++node) {
      const auto &candidate = values.nodes[node];
      if (candidate.source_operation == null_source_operation_id ||
          candidate.source_operation <= gate.source_operation) {
        continue;
      }
      const bool loop_invariant = classified.nodes[node].loop_invariant;
      add_unique(loop_invariant ? node : acquire,
                 loop_invariant ? acquire : node,
                 DependencyEdge{
                     .kind = DependencyKind::resource_order,
                     .value = null_value_id,
                     .resource = null_resource_id,
                 });
    }
  }

  CompactDependencyGraph graph(std::move(builder), allocator_ref{});
  return GraphIR(std::move(classified), std::move(graph));
}

} // namespace strobe::flow::detail::compiler
