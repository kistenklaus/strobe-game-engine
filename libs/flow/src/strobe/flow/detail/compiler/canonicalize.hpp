#pragma once

#include "strobe/flow/detail/compiler/normalized_ir.hpp"
#include "strobe/flow/detail/op_stream.hpp"

#include <limits>
#include <stdexcept>

namespace strobe::flow::detail::compiler {

inline NodeKind node_kind(OperationKind kind) {
  switch (kind) {
  case OperationKind::callback:
    return NodeKind::callback;
  case OperationKind::construct:
    return NodeKind::construct;
  case OperationKind::cutoff:
    return NodeKind::cutoff;
  case OperationKind::discard:
    return NodeKind::discard;
  case OperationKind::acquire:
    return NodeKind::acquire;
  case OperationKind::yield:
    return NodeKind::yield;
  case OperationKind::phi:
    return NodeKind::phi;
  }
  throw std::logic_error("unknown operation kind");
}

inline NormalizedIR canonicalize(const detail::OpStream &stream) {
  if (stream.ops().size() + stream.phis().size() >=
      std::numeric_limits<NodeId>::max()) {
    throw std::length_error("normalized node id space exhausted");
  }
  if (stream.ops().size() >= std::numeric_limits<SourceOperationId>::max()) {
    throw std::length_error("source operation id space exhausted");
  }
  std::size_t access_count = stream.phis().size() * 3;
  if (access_count > std::numeric_limits<std::uint32_t>::max()) {
    throw std::length_error("normalized access id space exhausted");
  }
  for (const auto &operation : stream.ops()) {
    if (operation.accesses.size() >
        std::numeric_limits<std::uint32_t>::max() - access_count) {
      throw std::length_error("normalized access id space exhausted");
    }
    access_count += operation.accesses.size();
  }

  NormalizedIR result;
  result.resources.reserve(stream.resources().size());
  result.nodes.reserve(stream.ops().size() + stream.phis().size());
  Vector<NodeId, allocator_ref> operation_nodes;
  operation_nodes.reserve(stream.ops().size());

  for (const auto &resource : stream.resources()) {
    ResourceKind kind;
    std::uint32_t detail;
    if (resource.construction) {
      kind = ResourceKind::owned;
      detail = static_cast<std::uint32_t>(*resource.construction);
    } else if (resource.external) {
      kind = ResourceKind::external;
      detail = static_cast<std::uint32_t>(*resource.external);
    } else if (resource.phi) {
      kind = ResourceKind::phi;
      detail = static_cast<std::uint32_t>(*resource.phi);
    } else if (resource.cutoff) {
      kind = ResourceKind::cutoff;
      detail = static_cast<std::uint32_t>(*resource.cutoff);
    } else {
      throw std::logic_error("resource has no normalized kind");
    }
    result.resources.push_back(NormalizedResource{
        .kind = kind,
        .type = resource.type,
        .size = resource.size,
        .alignment = resource.alignment,
        .destroy = resource.destroy,
        .detail = detail,
    });
  }

  std::size_t phi_index = 0;
  for (std::size_t position = 0; position <= stream.ops().size(); ++position) {
    while (phi_index < stream.phis().size() &&
           stream.phis()[phi_index].position == position) {
      const auto &phi = stream.phis()[phi_index++];
      const auto node = static_cast<NodeId>(result.nodes.size());
      const auto access_offset = result.accesses.size();
      result.accesses.push_back({phi.initial, AccessMode::read});
      result.accesses.push_back({*phi.feedback, AccessMode::read});
      result.accesses.push_back({phi.output, AccessMode::write});
      result.nodes.push_back(NormalizedNode{
          .kind = NodeKind::phi,
          .source_operation = null_source_operation_id,
          .accesses = {.offset = static_cast<std::uint32_t>(access_offset),
                       .count = 3},
          .argument_groups = {},
          .argument_elements = {},
          .argument_count = 0,
          .semantics = OpSemantics::none,
          .intrinsic = std::nullopt,
      });
      result.phis.push_back(NormalizedPhi{
          .node = node,
          .output = phi.output,
          .initial = phi.initial,
          .feedback = *phi.feedback,
          .condition = phi.condition,
          .declaration_position = static_cast<SourceOperationId>(phi.position),
          .feedback_position =
              static_cast<SourceOperationId>(*phi.feedback_position),
          .max_in_flight = phi.max_in_flight,
      });
    }

    if (position == stream.ops().size()) {
      continue;
    }
    const auto &operation = stream.ops()[position];
    const auto access_offset = result.accesses.size();
    const auto argument_group_offset = result.argument_groups.size();
    const auto argument_element_offset = result.argument_elements.size();
    for (const auto &access : operation.accesses) {
      result.accesses.push_back(NormalizedAccess{
          .resource = access.resource,
          .mode = access.mode,
          .purpose = access.purpose,
      });
    }
    for (const auto group : operation.argument_groups) {
      result.argument_groups.push_back(group);
    }
    for (const auto element : operation.argument_elements) {
      result.argument_elements.push_back(element);
    }
    operation_nodes.push_back(static_cast<NodeId>(result.nodes.size()));
    result.nodes.push_back(NormalizedNode{
        .kind = node_kind(operation.kind),
        .source_operation = static_cast<SourceOperationId>(position),
        .accesses = {.offset = static_cast<std::uint32_t>(access_offset),
                     .count =
                         static_cast<std::uint32_t>(operation.accesses.size())},
        .argument_groups = {
            .offset = static_cast<std::uint32_t>(argument_group_offset),
            .count = static_cast<std::uint32_t>(
                operation.argument_groups.size())},
        .argument_elements = {
            .offset = static_cast<std::uint32_t>(argument_element_offset),
            .count = static_cast<std::uint32_t>(
                operation.argument_elements.size())},
        .argument_count =
            static_cast<std::uint32_t>(operation.argument_types.size()),
        .semantics = operation.metadata.semantics,
        .intrinsic = operation.intrinsic,
    });
  }

  for (std::size_t index = 0; index < stream.cutoffs().size(); ++index) {
    const auto &cutoff = stream.cutoffs()[index];
    result.cutoffs.push_back(NormalizedCutoff{
        .node = operation_nodes[cutoff.operation],
        .input = cutoff.input,
        .output = cutoff.output,
        .source_cutoff = static_cast<std::uint32_t>(index),
    });
  }

  for (std::size_t index = 0; index < stream.externals().size(); ++index) {
    result.externals.push_back(NormalizedExternal{
        .resource = stream.externals()[index].resource,
        .source_external = static_cast<std::uint32_t>(index),
    });
  }
  result.ifs.reserve(stream.ifs().size());
  for (const auto &branch : stream.ifs()) {
    result.ifs.push_back(NormalizedIf{
        .condition = branch.condition,
        .begin = static_cast<SourceOperationId>(branch.begin),
        .alternative = static_cast<SourceOperationId>(branch.alternative),
        .end = static_cast<SourceOperationId>(branch.end),
    });
  }

  return result;
}

} // namespace strobe::flow::detail::compiler
