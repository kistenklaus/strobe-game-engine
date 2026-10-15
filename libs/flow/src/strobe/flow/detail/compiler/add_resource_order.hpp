#pragma once

#include "strobe/flow/detail/compiler/ordered_ir.hpp"

#include <optional>
#include <stdexcept>
#include <utility>

namespace strobe::flow::detail::compiler {

inline OrderedIR add_resource_order(DependencyIR dependencies) {
  OrderedIR result;
  result.dependencies = std::move(dependencies);
  const auto &resolved = result.dependencies.lowered.resolved;
  const auto &ir = resolved.values;

  struct ResourceState {
    std::optional<NodeId> writer;
    Vector<NodeId, allocator_ref> readers;
  };
  Vector<ResourceState, allocator_ref> states;
  states.resize(ir.resources.size());

  const auto is_backedge = [&](NodeId node, ValueId value) {
    if (value == null_value_id) {
      return false;
    }
    for (const auto &backedge : resolved.backedges) {
      if (backedge.target == node && backedge.value == value) {
        return true;
      }
    }
    return false;
  };
  const auto has_direct_edge = [&](NodeId before, NodeId after) {
    for (const auto &edge : result.dependencies.edges) {
      if (edge.producer == before && edge.consumer == after) {
        return true;
      }
    }
    return false;
  };
  const auto add_constraint = [&](NodeId before, NodeId after,
                                  ResourceId resource) {
    if (before == after || has_direct_edge(before, after)) {
      return;
    }
    for (const auto &constraint : result.resource_order) {
      if (constraint.before == before && constraint.after == after) {
        return;
      }
    }
    result.resource_order.push_back(ResourceOrderConstraint{
        .before = before,
        .after = after,
        .resource = resource,
    });
  };

  for (NodeId node = 0; node < ir.nodes.size(); ++node) {
    const auto range = ir.nodes[node].accesses;
    if (range.offset + range.count > ir.accesses.size()) {
      throw std::logic_error("node accesses are out of range");
    }
    for (std::uint32_t index = 0; index < range.count; ++index) {
      const auto &access = ir.accesses[range.offset + index];
      if (access.resource >= states.size()) {
        throw std::logic_error("access resource is out of range");
      }
      if (is_backedge(node, access.input)) {
        continue;
      }

      auto &state = states[access.resource];
      if (access.mode == AccessMode::read) {
        if (state.writer) {
          add_constraint(*state.writer, node, access.resource);
        }
        state.readers.push_back(node);
        continue;
      }

      if (state.writer && state.readers.empty()) {
        add_constraint(*state.writer, node, access.resource);
      }
      for (const auto reader : state.readers) {
        add_constraint(reader, node, access.resource);
      }
      state.readers.clear();
      state.writer = node;
    }
  }

  return result;
}

} // namespace strobe::flow::detail::compiler
