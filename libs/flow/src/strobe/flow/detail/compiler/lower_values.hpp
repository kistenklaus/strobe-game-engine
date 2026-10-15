#pragma once

#include "strobe/flow/detail/compiler/value_ir.hpp"

#include <limits>
#include <optional>
#include <stdexcept>

namespace strobe::flow::detail::compiler {

inline ValueIR lower_values(NormalizedIR normalized) {
  ValueIR result;
  result.resources = std::move(normalized.resources);
  result.cutoffs = std::move(normalized.cutoffs);
  result.externals = std::move(normalized.externals);
  result.nodes.reserve(normalized.nodes.size());
  result.accesses.reserve(normalized.accesses.size());

  Vector<std::optional<ValueId>, allocator_ref> current;
  Vector<std::uint32_t, allocator_ref> versions;
  current.resize(result.resources.size());
  versions.resize(result.resources.size());

  const auto define = [&](ResourceId resource, NodeId producer,
                          StaticValueFlags flags = StaticValueFlags::none) {
    if (result.values.size() >= std::numeric_limits<ValueId>::max()) {
      throw std::length_error("static value id space exhausted");
    }
    const auto value = static_cast<ValueId>(result.values.size());
    result.values.push_back(StaticValue{
        .resource = resource,
        .producer = producer,
        .version = versions[resource]++,
        .flags = flags,
    });
    current[resource] = value;
    return value;
  };

  for (const auto &external : result.externals) {
    define(external.resource, null_node_id, StaticValueFlags::external);
  }

  result.ifs.reserve(normalized.ifs.size());
  for (const auto &branch : normalized.ifs) {
    result.ifs.push_back(ValueIf{
        .condition_resource = branch.condition,
        .condition = null_value_id,
        .begin = branch.begin,
        .alternative = branch.alternative,
        .end = branch.end,
    });
  }

  const auto resolve_if_conditions = [&](SourceOperationId position) {
    for (auto &branch : result.ifs) {
      if (branch.begin != position || branch.condition != null_value_id) {
        continue;
      }
      if (!current[branch.condition_resource]) {
        throw std::logic_error("if condition has no static value");
      }
      branch.condition = *current[branch.condition_resource];
    }
  };

  std::size_t phi_index = 0;
  const auto resolve_feedback = [&](SourceOperationId position) {
    for (auto &phi : result.phis) {
      if (phi.feedback != null_value_id || phi.feedback_position != position) {
        continue;
      }
      const auto resource = phi.feedback_resource;
      if (!current[resource]) {
        throw std::logic_error("phi feedback has no static value");
      }
      phi.feedback = *current[resource];
      result.accesses[result.nodes[phi.node].accesses.offset + 1].input =
          phi.feedback;
      if (phi.condition_resource) {
        if (!current[*phi.condition_resource]) {
          throw std::logic_error("phi feedback condition has no static value");
        }
        phi.condition = *current[*phi.condition_resource];
      }
    }
  };

  for (NodeId node = 0; node < normalized.nodes.size(); ++node) {
    const auto &source_node = normalized.nodes[node];
    if (source_node.kind != NodeKind::phi) {
      resolve_if_conditions(source_node.source_operation);
      resolve_feedback(source_node.source_operation);
    }

    const auto output_offset = result.accesses.size();
    result.nodes.push_back(ValueNode{
        .kind = source_node.kind,
        .source_operation = source_node.source_operation,
        .accesses = {.offset = static_cast<std::uint32_t>(output_offset),
                     .count = source_node.accesses.count},
        .semantics = source_node.semantics,
    });

    if (source_node.kind == NodeKind::phi) {
      const auto &source_phi = normalized.phis[phi_index++];
      const auto initial = normalized.accesses[source_node.accesses.offset];
      if (!current[initial.resource]) {
        throw std::logic_error("phi initial input has no static value");
      }
      result.accesses.push_back({initial.resource, AccessMode::read,
                                 *current[initial.resource], null_value_id});
      result.accesses.push_back({source_phi.feedback, AccessMode::read,
                                 null_value_id, null_value_id});
      const auto output = define(source_phi.output, node);
      result.accesses.push_back(
          {source_phi.output, AccessMode::write, null_value_id, output});
      result.phis.push_back(ValuePhi{
          .node = node,
          .feedback_resource = source_phi.feedback,
          .output = output,
          .initial = result.accesses[output_offset].input,
          .feedback = null_value_id,
          .condition_resource = source_phi.condition,
          .condition = null_value_id,
          .declaration_position = source_phi.declaration_position,
          .feedback_position = source_phi.feedback_position,
      });
      continue;
    }

    for (std::uint32_t index = 0; index < source_node.accesses.count; ++index) {
      const auto &access =
          normalized.accesses[source_node.accesses.offset + index];
      if (source_node.kind == NodeKind::discard) {
        result.accesses.push_back(
            {access.resource, access.mode, null_value_id, null_value_id});
        current[access.resource] = std::nullopt;
        continue;
      }

      const bool defines_without_input =
          (source_node.kind == NodeKind::construct && index == 0) ||
          result.resources[access.resource].kind == ResourceKind::cutoff;
      ValueId input = null_value_id;
      ValueId output = null_value_id;
      if (access.mode != AccessMode::write || !defines_without_input) {
        if (current[access.resource]) {
          input = *current[access.resource];
        } else if (access.mode != AccessMode::write) {
          throw std::logic_error("read access has no static value");
        }
      }
      if (access.mode != AccessMode::read) {
        output = define(access.resource, node);
      }
      result.accesses.push_back({access.resource, access.mode, input, output});
    }
  }

  SourceOperationId completed_operations = 0;
  for (const auto &node : normalized.nodes) {
    if (node.source_operation != null_source_operation_id) {
      completed_operations = node.source_operation + 1;
    }
  }
  resolve_feedback(completed_operations);
  resolve_if_conditions(completed_operations);
  for (const auto &phi : result.phis) {
    if (phi.feedback == null_value_id) {
      throw std::logic_error("phi feedback was not resolved");
    }
    if (phi.condition_resource && phi.condition == null_value_id) {
      throw std::logic_error("phi feedback condition was not resolved");
    }
  }
  for (const auto &branch : result.ifs) {
    if (branch.condition == null_value_id) {
      throw std::logic_error("if condition was not resolved");
    }
  }

  return result;
}

} // namespace strobe::flow::detail::compiler
