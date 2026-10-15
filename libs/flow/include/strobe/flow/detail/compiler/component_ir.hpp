#pragma once

#include "strobe/flow/detail/compiler/graph_ir.hpp"

#include <strobe/core/containers/vector.hpp>
#include <strobe/core/graph.hpp>

#include <cstdint>
#include <utility>

namespace strobe::flow::detail::compiler {

using ComponentId = std::uint32_t;

struct ComponentInfo {
  IndexRange nodes;
  bool cyclic;
  bool has_temporal_edge;
};

struct ComponentNode {
  ComponentId source;
  bool cyclic;
  bool has_temporal_edge;
};

struct ComponentDependency {
  bool value;
  bool resource_order;
  bool temporal;
};

using CondensationGraph =
    graph::ConstGraph<ComponentNode, ComponentDependency, allocator_ref>;

struct ComponentIR {
  ComponentIR(GraphIR input, Vector<ComponentId, allocator_ref> component_of,
              Vector<NodeId, allocator_ref> component_nodes,
              Vector<ComponentInfo, allocator_ref> components,
              CondensationGraph condensation,
              Vector<ComponentId, allocator_ref> topological_order)
      : input(std::move(input)), component_of(std::move(component_of)),
        component_nodes(std::move(component_nodes)),
        components(std::move(components)),
        condensation(std::move(condensation)),
        topological_order(std::move(topological_order)) {}

  GraphIR input;
  Vector<ComponentId, allocator_ref> component_of;
  Vector<NodeId, allocator_ref> component_nodes;
  Vector<ComponentInfo, allocator_ref> components;
  CondensationGraph condensation;
  Vector<ComponentId, allocator_ref> topological_order;
};

} // namespace strobe::flow::detail::compiler
