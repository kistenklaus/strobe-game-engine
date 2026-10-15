#pragma once

#include "strobe/flow/detail/compiler/activation_ir.hpp"

#include <strobe/core/containers/vector.hpp>
#include <strobe/core/graph.hpp>

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace strobe::flow::detail::compiler {

inline ActivationIR analyze_activation(ComponentIR input) {
  struct WorkingInfo {
    Vector<ComponentId, allocator_ref> feedback_sources;
    bool externally_triggered = false;
    bool contains_cutoff = false;
    bool memoized = false;
    bool contains_yield = false;
    bool reaches_yield = false;

    WorkingInfo() : feedback_sources(allocator_ref{}) {}
  };

  const auto component_count = input.components.size();
  Vector<WorkingInfo, allocator_ref> working;
  working.resize(component_count);
  const auto &node_graph = input.input.graph;

  for (ComponentId component = 0; component < component_count; ++component) {
    auto &info = working[component];
    if (input.components[component].cyclic) {
      info.feedback_sources.push_back(component);
    }
    const auto members = input.components[component].nodes;
    if (members.offset + members.count > input.component_nodes.size()) {
      throw std::logic_error("component node range is out of bounds");
    }
    for (std::uint32_t index = 0; index < members.count; ++index) {
      const auto node = input.component_nodes[members.offset + index];
      if (node >= node_graph.node_count()) {
        throw std::logic_error("component node is out of range");
      }
      const auto &payload = node_graph.get(graph::NodeId{node});
      info.externally_triggered |= payload.classification.reads_external ||
                                   payload.kind == NodeKind::acquire;
      info.contains_cutoff |= payload.kind == NodeKind::cutoff;
      info.contains_yield |=
          payload.kind == NodeKind::yield || payload.kind == NodeKind::acquire;
    }
    info.memoized = info.contains_cutoff;
    info.reaches_yield = info.contains_yield;
  }

  const auto append_unique =
      [](Vector<ComponentId, allocator_ref> &target,
         const Vector<ComponentId, allocator_ref> &source) {
        for (const auto value : source) {
          if (std::find(target.begin(), target.end(), value) == target.end()) {
            target.push_back(value);
          }
        }
      };

  for (const auto component : input.topological_order) {
    if (component >= component_count) {
      throw std::logic_error("component topological order is invalid");
    }
    const auto source = graph::NodeId{component};
    for (const auto edge : input.condensation.outgoing(source)) {
      const auto destination = input.condensation.destination(edge).value;
      append_unique(working[destination].feedback_sources,
                    working[component].feedback_sources);
      working[destination].externally_triggered |=
          working[component].externally_triggered;
      working[destination].memoized |=
          working[component].memoized || working[component].contains_cutoff;
    }
  }

  for (auto iterator = input.topological_order.rbegin();
       iterator != input.topological_order.rend(); ++iterator) {
    const auto component = *iterator;
    for (const auto edge :
         input.condensation.outgoing(graph::NodeId{component})) {
      const auto destination = input.condensation.destination(edge).value;
      working[component].reaches_yield |= working[destination].reaches_yield;
    }
  }

  Vector<ActivationInfo, allocator_ref> components;
  Vector<ComponentId, allocator_ref> feedback_sources;
  components.reserve(component_count);
  for (ComponentId component = 0; component < component_count; ++component) {
    auto &info = working[component];
    std::sort(info.feedback_sources.begin(), info.feedback_sources.end());
    const auto offset = feedback_sources.size();
    for (const auto source : info.feedback_sources) {
      feedback_sources.push_back(source);
    }
    components.push_back(ActivationInfo{
        .feedback_sources =
            {
                .offset = static_cast<std::uint32_t>(offset),
                .count =
                    static_cast<std::uint32_t>(info.feedback_sources.size()),
            },
        .runs_once =
            info.feedback_sources.empty() && !info.externally_triggered,
        .externally_triggered = info.externally_triggered,
        .memoized = info.memoized,
        .contains_yield = info.contains_yield,
        .reaches_yield = info.reaches_yield,
    });
  }

  return ActivationIR(std::move(input), std::move(components),
                      std::move(feedback_sources));
}

} // namespace strobe::flow::detail::compiler
