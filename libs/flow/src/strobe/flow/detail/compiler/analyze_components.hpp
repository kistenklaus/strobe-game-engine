#pragma once

#include "strobe/flow/detail/compiler/component_ir.hpp"

#include <strobe/core/graph.hpp>

#include <limits>
#include <stdexcept>
#include <utility>

namespace strobe::flow::detail::compiler {

inline ComponentIR analyze_components(GraphIR input) {
  graph::AdjGraph<graph::EmptyEdge, graph::EmptyEdge, allocator_ref>
      immediate_builder{allocator_ref{}};
  for (std::uint32_t node = 0; node < input.graph.node_count(); ++node) {
    immediate_builder.add_node({});
  }
  for (std::uint32_t index = 0; index < input.graph.edge_count(); ++index) {
    const graph::EdgeId edge{index};
    if (input.graph.get(edge).kind != DependencyKind::temporal) {
      immediate_builder.add_edge(input.graph.source(edge),
                                 input.graph.destination(edge));
    }
  }
  graph::ConstGraph<graph::EmptyEdge, graph::EmptyEdge, allocator_ref>
      immediate_graph(std::move(immediate_builder), allocator_ref{});
  if (!graph::topological_sort(immediate_graph, allocator_ref{}).acyclic) {
    throw std::logic_error("dependency cycle does not contain a temporal edge");
  }

  const auto partition = graph::strongly_connected_components(input.graph);
  Vector<ComponentId, allocator_ref> component_of;
  Vector<NodeId, allocator_ref> component_nodes;
  Vector<ComponentInfo, allocator_ref> components;
  component_of.resize(input.graph.node_count());
  component_nodes.reserve(input.graph.node_count());
  components.reserve(partition.component_count());

  for (ComponentId component = 0; component < partition.component_count();
       ++component) {
    const auto members = partition.component(component);
    const auto offset = component_nodes.size();
    for (const auto member : members) {
      component_of[member.value] = component;
      component_nodes.push_back(static_cast<NodeId>(member.value));
    }
    components.push_back(ComponentInfo{
        .nodes = {.offset = static_cast<std::uint32_t>(offset),
                  .count = static_cast<std::uint32_t>(members.size())},
        .cyclic = members.size() > 1,
        .has_temporal_edge = false,
    });
  }

  for (std::uint32_t index = 0; index < input.graph.edge_count(); ++index) {
    const graph::EdgeId edge{index};
    const auto source = input.graph.source(edge).value;
    const auto destination = input.graph.destination(edge).value;
    const auto source_component = component_of[source];
    if (source_component != component_of[destination]) {
      continue;
    }
    auto &component = components[source_component];
    component.cyclic |= source == destination;
    component.has_temporal_edge |=
        input.graph.get(edge).kind == DependencyKind::temporal;
  }

  for (const auto &component : components) {
    if (component.cyclic && !component.has_temporal_edge) {
      throw std::logic_error(
          "dependency cycle does not contain a temporal edge");
    }
  }

  using Builder =
      graph::AdjGraph<ComponentNode, ComponentDependency, allocator_ref>;
  Builder builder{allocator_ref{}};
  for (ComponentId component = 0; component < components.size(); ++component) {
    const auto &info = components[component];
    const auto node = builder.add_node(ComponentNode{
        .source = component,
        .cyclic = info.cyclic,
        .has_temporal_edge = info.has_temporal_edge,
    });
    if (node.value != component) {
      throw std::logic_error("component graph node mapping is not dense");
    }
  }

  const auto mark_kind = [](ComponentDependency &dependency,
                            DependencyKind kind) {
    dependency.value |= kind == DependencyKind::value;
    dependency.resource_order |= kind == DependencyKind::resource_order;
    dependency.temporal |= kind == DependencyKind::temporal;
  };
  for (std::uint32_t index = 0; index < input.graph.edge_count(); ++index) {
    const graph::EdgeId edge{index};
    const auto source = component_of[input.graph.source(edge).value];
    const auto destination = component_of[input.graph.destination(edge).value];
    if (source == destination) {
      continue;
    }

    bool found = false;
    for (std::uint32_t candidate = 0; candidate < builder.edge_capacity();
         ++candidate) {
      const graph::EdgeId existing{candidate};
      if (builder.contains(existing) &&
          builder.source(existing).value == source &&
          builder.destination(existing).value == destination) {
        mark_kind(builder.get(existing), input.graph.get(edge).kind);
        found = true;
        break;
      }
    }
    if (!found) {
      ComponentDependency dependency{};
      mark_kind(dependency, input.graph.get(edge).kind);
      builder.add_edge(graph::NodeId{source}, graph::NodeId{destination},
                       dependency);
    }
  }

  CondensationGraph condensation(std::move(builder), allocator_ref{});
  const auto sorted = graph::topological_sort(condensation, allocator_ref{});
  if (!sorted.acyclic) {
    throw std::logic_error("component condensation graph contains a cycle");
  }
  Vector<ComponentId, allocator_ref> topological_order;
  topological_order.reserve(sorted.order.size());
  for (const auto component : sorted.order) {
    topological_order.push_back(component.value);
  }

  return ComponentIR(std::move(input), std::move(component_of),
                     std::move(component_nodes), std::move(components),
                     std::move(condensation), std::move(topological_order));
}

} // namespace strobe::flow::detail::compiler
