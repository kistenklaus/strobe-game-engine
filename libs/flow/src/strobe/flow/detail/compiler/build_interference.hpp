#pragma once

#include "strobe/flow/detail/compiler/interference_ir.hpp"

#include <strobe/core/containers/vector.hpp>
#include <strobe/core/graph.hpp>

#include <stdexcept>
#include <utility>

namespace strobe::flow::detail::compiler {

inline InterferenceIR build_interference(LifetimeIR input) {
  using Builder =
      graph::AdjGraph<InterferenceResource, graph::EmptyEdge, allocator_ref>;
  Builder builder{allocator_ref{}};
  Vector<graph::NodeId, allocator_ref> resource_to_node(
      input.resources.size(), null_interference_node, allocator_ref{});

  for (ResourceId resource = 0; resource < input.resources.size(); ++resource) {
    const auto &lifetime = input.resources[resource];
    if (lifetime.storage != StorageClass::transient &&
        lifetime.storage != StorageClass::persistent) {
      continue;
    }
    const auto &storage = input.input.resources[resource];
    const auto node = builder.add_node(InterferenceResource{
        .resource = resource,
        .size = storage.size,
        .alignment = storage.alignment,
        .storage = lifetime.storage,
        .may_overlap_generations = lifetime.may_overlap_generations,
    });
    resource_to_node[resource] = node;
  }

  const auto &node_graph = input.input.input.input.input.graph;
  const auto reaches = [&](NodeId source, NodeId destination) {
    if (source >= node_graph.node_count() ||
        destination >= node_graph.node_count()) {
      throw std::logic_error("interference lifetime node is out of range");
    }
    Vector<std::uint8_t, allocator_ref> visited(node_graph.node_count(), 0,
                                                allocator_ref{});
    Vector<NodeId, allocator_ref> stack;
    visited[source] = 1;
    stack.push_back(source);
    while (!stack.empty()) {
      const auto node = stack.back();
      stack.pop_back();
      for (const auto edge : node_graph.outgoing(graph::NodeId{node})) {
        if (node_graph.get(edge).kind == DependencyKind::temporal) {
          continue;
        }
        const auto next = node_graph.destination(edge).value;
        if (next == destination) {
          return true;
        }
        if (!visited[next]) {
          visited[next] = 1;
          stack.push_back(next);
        }
      }
    }
    return false;
  };

  const auto ordered_before = [&](const ResourceLifetime &before,
                                  const ResourceLifetime &after) {
    if (before.may_overlap_generations || before.last_uses.count == 0 ||
        after.definitions.count == 0) {
      return false;
    }
    if (before.last_uses.offset + before.last_uses.count >
            input.last_uses.size() ||
        after.definitions.offset + after.definitions.count >
            input.definitions.size()) {
      throw std::logic_error("resource lifetime range is out of bounds");
    }
    for (std::uint32_t last_index = 0; last_index < before.last_uses.count;
         ++last_index) {
      const auto last = input.last_uses[before.last_uses.offset + last_index];
      for (std::uint32_t definition_index = 0;
           definition_index < after.definitions.count; ++definition_index) {
        const auto definition =
            input.definitions[after.definitions.offset + definition_index];
        if (!reaches(last, definition)) {
          return false;
        }
      }
    }
    return true;
  };

  for (ResourceId left = 0; left < input.resources.size(); ++left) {
    if (resource_to_node[left] == null_interference_node) {
      continue;
    }
    for (ResourceId right = left + 1; right < input.resources.size(); ++right) {
      if (resource_to_node[right] == null_interference_node) {
        continue;
      }
      const auto &left_lifetime = input.resources[left];
      const auto &right_lifetime = input.resources[right];
      bool interferes = left_lifetime.storage == StorageClass::persistent ||
                        right_lifetime.storage == StorageClass::persistent;
      if (!interferes) {
        interferes = !ordered_before(left_lifetime, right_lifetime) &&
                     !ordered_before(right_lifetime, left_lifetime);
      }
      if (interferes) {
        builder.add_edge(resource_to_node[left], resource_to_node[right]);
      }
    }
  }

  InterferenceGraph graph(std::move(builder), allocator_ref{});
  return InterferenceIR(std::move(input), std::move(graph),
                        std::move(resource_to_node));
}

} // namespace strobe::flow::detail::compiler
