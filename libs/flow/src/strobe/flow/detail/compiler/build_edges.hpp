#pragma once

#include "strobe/flow/detail/compiler/dependency_ir.hpp"

#include <stdexcept>
#include <utility>

namespace strobe::flow::detail::compiler {

inline DependencyIR build_edges(CutoffIR lowered) {
  DependencyIR result;
  result.lowered = std::move(lowered);
  const auto &resolved = result.lowered.resolved;
  const auto &ir = resolved.values;

  const auto is_backedge = [&](NodeId consumer, ValueId value) {
    for (const auto &backedge : resolved.backedges) {
      if (backedge.target == consumer && backedge.value == value) {
        return true;
      }
    }
    return false;
  };

  for (NodeId consumer = 0; consumer < ir.nodes.size(); ++consumer) {
    const auto range = ir.nodes[consumer].accesses;
    if (range.offset + range.count > ir.accesses.size()) {
      throw std::logic_error("node accesses are out of range");
    }
    for (std::uint32_t index = 0; index < range.count; ++index) {
      const auto &access = ir.accesses[range.offset + index];
      if (access.input == null_value_id) {
        continue;
      }
      if (access.input >= ir.values.size()) {
        throw std::logic_error("input static value is out of range");
      }
      if (is_backedge(consumer, access.input)) {
        continue;
      }

      const auto producer = ir.values[access.input].producer;
      if (producer == null_node_id) {
        continue;
      }
      if (producer >= ir.nodes.size()) {
        throw std::logic_error("static value producer is out of range");
      }
      result.edges.push_back(ProducerConsumerEdge{
          .producer = producer,
          .consumer = consumer,
          .value = access.input,
      });
    }
  }

  for (const auto &branch : ir.ifs) {
    if (branch.condition >= ir.values.size()) {
      throw std::logic_error("if condition is out of range");
    }
    const auto producer = ir.values[branch.condition].producer;
    if (producer == null_node_id) {
      continue;
    }
    for (NodeId consumer = 0; consumer < ir.nodes.size(); ++consumer) {
      const auto position = ir.nodes[consumer].source_operation;
      if (position < branch.begin || position >= branch.end ||
          consumer == producer) {
        continue;
      }
      result.edges.push_back(ProducerConsumerEdge{
          .producer = producer,
          .consumer = consumer,
          .value = branch.condition,
      });
    }
  }

  return result;
}

} // namespace strobe::flow::detail::compiler
