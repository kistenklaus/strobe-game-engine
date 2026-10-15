#pragma once

#include "strobe/flow/detail/compiler/classified_ir.hpp"

#include <stdexcept>
#include <utility>

namespace strobe::flow::detail::compiler {

inline ClassifiedIR classify_nodes(OrderedIR ordered) {
  ClassifiedIR result;
  result.ordered = std::move(ordered);
  const auto &dependencies = result.ordered.dependencies;
  const auto &ir = dependencies.lowered.resolved.values;
  result.nodes.resize(ir.nodes.size());
  result.preconstructed_resources.resize(ir.resources.size(), false);

  Vector<bool, allocator_ref> has_predecessor;
  has_predecessor.resize(ir.nodes.size());
  for (const auto &edge : dependencies.edges) {
    if (edge.producer >= ir.nodes.size() || edge.consumer >= ir.nodes.size()) {
      throw std::logic_error("producer-consumer edge is out of range");
    }
    has_predecessor[edge.consumer] = true;
  }
  for (const auto &constraint : result.ordered.resource_order) {
    if (constraint.before >= ir.nodes.size() ||
        constraint.after >= ir.nodes.size()) {
      throw std::logic_error("resource-order constraint is out of range");
    }
    has_predecessor[constraint.after] = true;
  }

  for (NodeId node = 0; node < ir.nodes.size(); ++node) {
    const auto range = ir.nodes[node].accesses;
    if (range.offset + range.count > ir.accesses.size()) {
      throw std::logic_error("node accesses are out of range");
    }

    bool reads_external = false;
    bool writes_external = false;
    for (std::uint32_t index = 0; index < range.count; ++index) {
      const auto &access = ir.accesses[range.offset + index];
      if (access.resource >= ir.resources.size()) {
        throw std::logic_error("access resource is out of range");
      }
      if (ir.resources[access.resource].kind != ResourceKind::external) {
        continue;
      }
      reads_external |= access.mode != AccessMode::write;
      writes_external |= access.mode != AccessMode::read;
    }

    const auto source_operation = ir.nodes[node].source_operation;
    for (const auto &branch : ir.ifs) {
      if (branch.condition >= ir.values.size()) {
        throw std::logic_error("if condition value is out of range");
      }
      const auto condition_is_external =
          ir.values[branch.condition].flags == StaticValueFlags::external;
      reads_external |= condition_is_external &&
                        source_operation != null_source_operation_id &&
                        source_operation >= branch.begin &&
                        source_operation < branch.end;
    }

    const bool root = !has_predecessor[node];
    const bool declared_side_effect =
        (static_cast<std::uint32_t>(ir.nodes[node].semantics) &
         static_cast<std::uint32_t>(OpSemantics::side_effect)) != 0;
    const bool observable_effect = declared_side_effect || writes_external ||
                                   ir.nodes[node].kind == NodeKind::acquire ||
                                   ir.nodes[node].kind == NodeKind::yield;
    result.nodes[node] = NodeClassification{
        .root = root,
        .reads_external = reads_external,
        .writes_external = writes_external,
        .observable_effect = observable_effect,
    };
    if (root) {
      result.roots.push_back(node);
    }
    if (observable_effect) {
      result.observable_effects.push_back(node);
    }
  }

  return result;
}

} // namespace strobe::flow::detail::compiler
