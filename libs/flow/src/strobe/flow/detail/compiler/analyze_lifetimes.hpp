#pragma once

#include "strobe/flow/detail/compiler/lifetime_ir.hpp"

#include <strobe/core/containers/vector.hpp>
#include <strobe/core/graph.hpp>

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace strobe::flow::detail::compiler {

inline LifetimeIR analyze_lifetimes(StorageIR input) {
  const auto &activation = input.input;
  const auto &components = activation.input;
  const auto &node_graph = components.input.graph;
  const auto &values =
      components.input.classified.ordered.dependencies.lowered.resolved.values;

  Vector<ResourceLifetime, allocator_ref> resources;
  Vector<NodeId, allocator_ref> definitions;
  Vector<NodeId, allocator_ref> uses;
  Vector<NodeId, allocator_ref> last_uses;
  Vector<ComponentId, allocator_ref> activation_sources;
  resources.reserve(values.resources.size());

  const auto append_node = [](Vector<NodeId, allocator_ref> &nodes,
                              NodeId node) {
    if (std::find(nodes.begin(), nodes.end(), node) == nodes.end()) {
      nodes.push_back(node);
    }
  };
  const auto append_component =
      [](Vector<ComponentId, allocator_ref> &components,
         ComponentId component) {
        if (std::find(components.begin(), components.end(), component) ==
            components.end()) {
          components.push_back(component);
        }
      };

  const auto reaches = [&](NodeId source, NodeId destination) {
    if (source >= node_graph.node_count() ||
        destination >= node_graph.node_count()) {
      throw std::logic_error("lifetime node is out of range");
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

  for (ResourceId resource = 0; resource < values.resources.size();
       ++resource) {
    Vector<NodeId, allocator_ref> resource_definitions;
    Vector<NodeId, allocator_ref> resource_uses;
    Vector<ComponentId, allocator_ref> resource_sources;
    bool externally_triggered = false;
    bool repeatedly_defined = false;
    bool memoized_reconstruction = false;
    bool retained_by_feedback = false;

    for (const auto &backedge : components.input.classified.ordered.dependencies
                                    .lowered.resolved.backedges) {
      if (backedge.value < values.values.size() &&
          values.values[backedge.value].resource == resource) {
        retained_by_feedback = true;
      }
    }

    for (NodeId node = 0; node < values.nodes.size(); ++node) {
      const auto range = values.nodes[node].accesses;
      if (range.offset + range.count > values.accesses.size()) {
        throw std::logic_error("node accesses are out of range");
      }
      bool accesses_resource = false;
      bool defines_resource = false;
      for (std::uint32_t index = 0; index < range.count; ++index) {
        const auto &access = values.accesses[range.offset + index];
        if (access.resource != resource) {
          continue;
        }
        accesses_resource = true;
        defines_resource |= access.output != null_value_id;
      }
      if (!accesses_resource) {
        continue;
      }

      append_node(resource_uses, node);
      if (defines_resource) {
        append_node(resource_definitions, node);
      }
      if (node >= components.component_of.size()) {
        throw std::logic_error("resource use component is out of range");
      }
      const auto component = components.component_of[node];
      const auto &info = activation.components[component];
      externally_triggered |= info.externally_triggered;
      repeatedly_defined |= defines_resource && !info.runs_once;
      memoized_reconstruction |= defines_resource && !info.runs_once &&
                                 info.memoized &&
                                 values.nodes[node].kind == NodeKind::construct;
      if (info.feedback_sources.offset + info.feedback_sources.count >
          activation.feedback_sources.size()) {
        throw std::logic_error("activation source range is out of bounds");
      }
      for (std::uint32_t index = 0; index < info.feedback_sources.count;
           ++index) {
        append_component(
            resource_sources,
            activation.feedback_sources[info.feedback_sources.offset + index]);
      }
    }

    Vector<NodeId, allocator_ref> resource_last_uses;
    for (const auto candidate : resource_uses) {
      bool precedes_another_use = false;
      for (const auto other : resource_uses) {
        if (candidate != other && reaches(candidate, other)) {
          precedes_another_use = true;
          break;
        }
      }
      if (!precedes_another_use) {
        resource_last_uses.push_back(candidate);
      }
    }
    std::sort(resource_definitions.begin(), resource_definitions.end());
    std::sort(resource_uses.begin(), resource_uses.end());
    std::sort(resource_last_uses.begin(), resource_last_uses.end());
    std::sort(resource_sources.begin(), resource_sources.end());

    const auto definition_offset = definitions.size();
    for (const auto node : resource_definitions) {
      definitions.push_back(node);
    }
    const auto use_offset = uses.size();
    for (const auto node : resource_uses) {
      uses.push_back(node);
    }
    const auto last_use_offset = last_uses.size();
    for (const auto node : resource_last_uses) {
      last_uses.push_back(node);
    }
    const auto source_offset = activation_sources.size();
    for (const auto source : resource_sources) {
      activation_sources.push_back(source);
    }

    const auto storage = input.resources[resource].storage;
    resources.push_back(ResourceLifetime{
        .storage = storage,
        .definitions =
            {
                .offset = static_cast<std::uint32_t>(definition_offset),
                .count =
                    static_cast<std::uint32_t>(resource_definitions.size()),
            },
        .uses =
            {
                .offset = static_cast<std::uint32_t>(use_offset),
                .count = static_cast<std::uint32_t>(resource_uses.size()),
            },
        .last_uses =
            {
                .offset = static_cast<std::uint32_t>(last_use_offset),
                .count = static_cast<std::uint32_t>(resource_last_uses.size()),
            },
        .activation_sources =
            {
                .offset = static_cast<std::uint32_t>(source_offset),
                .count = static_cast<std::uint32_t>(resource_sources.size()),
            },
        .externally_triggered = externally_triggered,
        .may_overlap_generations = (storage == StorageClass::transient ||
                                    storage == StorageClass::persistent) &&
                                   repeatedly_defined,
        .memoized_reconstruction = memoized_reconstruction,
        .preconstructed =
            resource <
                components.input.classified.preconstructed_resources.size() &&
            components.input.classified.preconstructed_resources[resource],
        .retained_by_feedback = retained_by_feedback,
        .destroy_at_shutdown = storage == StorageClass::persistent,
    });
  }

  return LifetimeIR(std::move(input), std::move(resources),
                    std::move(definitions), std::move(uses),
                    std::move(last_uses), std::move(activation_sources));
}

} // namespace strobe::flow::detail::compiler
