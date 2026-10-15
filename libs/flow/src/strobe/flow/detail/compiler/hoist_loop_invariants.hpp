#pragma once

#include "strobe/flow/detail/compiler/classified_ir.hpp"

#include <strobe/core/containers/vector.hpp>

#include <cstdint>
#include <stdexcept>
#include <utility>

namespace strobe::flow::detail::compiler {

inline ClassifiedIR hoist_loop_invariants(ClassifiedIR input) {
  const auto &ordered = input.ordered;
  const auto &values = ordered.dependencies.lowered.resolved.values;

  Vector<bool, allocator_ref> candidate_nodes(values.nodes.size(), false,
                                              allocator_ref{});
  Vector<bool, allocator_ref> candidate_values(values.values.size(), false,
                                               allocator_ref{});

  const auto contains_semantics = [](OpSemantics value, OpSemantics flag) {
    return (static_cast<std::uint32_t>(value) &
            static_cast<std::uint32_t>(flag)) != 0;
  };
  const auto inside_structured_control = [&](const ValueNode &node) {
    if (node.source_operation == null_source_operation_id) {
      return true;
    }
    for (const auto &branch : values.ifs) {
      if (node.source_operation >= branch.begin &&
          node.source_operation < branch.end) {
        return true;
      }
    }
    for (const auto &phi : values.phis) {
      if (phi.condition != null_value_id &&
          node.source_operation >= phi.declaration_position &&
          node.source_operation < phi.feedback_position) {
        return true;
      }
    }
    return false;
  };
  const auto predecessors_satisfy = [&](NodeId node, const auto &eligible) {
    for (const auto &edge : ordered.dependencies.edges) {
      if (edge.producer >= eligible.size() ||
          edge.consumer >= eligible.size()) {
        throw std::logic_error("LICM dependency edge is out of range");
      }
      if (edge.consumer == node && !eligible[edge.producer]) {
        return false;
      }
    }
    for (const auto &constraint : ordered.resource_order) {
      if (constraint.before >= eligible.size() ||
          constraint.after >= eligible.size()) {
        throw std::logic_error("LICM resource order is out of range");
      }
      if (constraint.after == node && !eligible[constraint.before]) {
        return false;
      }
    }
    return true;
  };

  bool changed = true;
  while (changed) {
    changed = false;
    for (NodeId node = 0; node < values.nodes.size(); ++node) {
      if (candidate_nodes[node]) {
        continue;
      }
      const auto &candidate = values.nodes[node];
      const bool movable_operation =
          (candidate.kind == NodeKind::construct ||
           candidate.kind == NodeKind::callback) &&
          !contains_semantics(candidate.semantics, OpSemantics::side_effect);
      if (!movable_operation || inside_structured_control(candidate) ||
          candidate.accesses.offset + candidate.accesses.count >
              values.accesses.size() ||
          !predecessors_satisfy(node, candidate_nodes)) {
        continue;
      }

      bool invariant = true;
      bool has_output = false;
      for (std::uint32_t index = 0; index < candidate.accesses.count; ++index) {
        const auto &access = values.accesses[candidate.accesses.offset + index];
        if (access.resource >= values.resources.size()) {
          throw std::logic_error("LICM access resource is out of range");
        }
        if (access.input != null_value_id &&
            (access.input >= candidate_values.size() ||
             !candidate_values[access.input])) {
          invariant = false;
          break;
        }
        if (access.output != null_value_id) {
          has_output = true;
          invariant &=
              values.resources[access.resource].kind == ResourceKind::owned;
        }
      }
      if (!invariant || !has_output) {
        continue;
      }

      candidate_nodes[node] = true;
      for (std::uint32_t index = 0; index < candidate.accesses.count; ++index) {
        const auto output =
            values.accesses[candidate.accesses.offset + index].output;
        if (output != null_value_id) {
          if (output >= candidate_values.size()) {
            throw std::logic_error("LICM output value is out of range");
          }
          candidate_values[output] = true;
        }
      }
      changed = true;
    }
  }

  Vector<bool, allocator_ref> invariant_nodes = candidate_nodes;
  for (NodeId node = 0; node < values.nodes.size(); ++node) {
    if (!candidate_nodes[node] ||
        values.nodes[node].kind != NodeKind::construct) {
      continue;
    }
    const auto accesses = values.nodes[node].accesses;
    if (accesses.count == 0) {
      continue;
    }
    const auto resource = values.accesses[accesses.offset].resource;
    bool found_discard = false;
    for (NodeId later = node + 1; later < values.nodes.size(); ++later) {
      const auto later_accesses = values.nodes[later].accesses;
      bool accesses_resource = false;
      for (std::uint32_t index = 0; index < later_accesses.count; ++index) {
        accesses_resource |=
            values.accesses[later_accesses.offset + index].resource == resource;
      }
      if (!accesses_resource) {
        continue;
      }
      found_discard = values.nodes[later].kind == NodeKind::discard;
      break;
    }
    if (found_discard) {
      input.preconstructed_resources[resource] = true;
    }
  }
  changed = true;
  while (changed) {
    changed = false;
    for (NodeId node = 0; node < values.nodes.size(); ++node) {
      if (!invariant_nodes[node]) {
        continue;
      }
      const auto &candidate = values.nodes[node];
      bool invariant = predecessors_satisfy(node, invariant_nodes);
      for (std::uint32_t index = 0;
           invariant && index < candidate.accesses.count; ++index) {
        const auto &access = values.accesses[candidate.accesses.offset + index];
        if (access.output == null_value_id) {
          continue;
        }
        if (input.preconstructed_resources[access.resource] &&
            candidate.kind == NodeKind::construct) {
          continue;
        }
        for (NodeId definition = 0; definition < values.nodes.size();
             ++definition) {
          const auto range = values.nodes[definition].accesses;
          for (std::uint32_t definition_index = 0;
               definition_index < range.count; ++definition_index) {
            const auto &other =
                values.accesses[range.offset + definition_index];
            if (other.resource == access.resource &&
                other.output != null_value_id && !invariant_nodes[definition]) {
              invariant = false;
              break;
            }
          }
          if (!invariant) {
            break;
          }
        }
      }
      if (!invariant) {
        invariant_nodes[node] = false;
        changed = true;
      }
    }
  }

  for (NodeId node = 0; node < invariant_nodes.size(); ++node) {
    input.nodes[node].loop_invariant = invariant_nodes[node];
  }
  return input;
}

} // namespace strobe::flow::detail::compiler
