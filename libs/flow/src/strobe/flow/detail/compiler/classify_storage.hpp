#pragma once

#include "strobe/flow/detail/compiler/storage_ir.hpp"

#include <strobe/core/containers/vector.hpp>

#include <stdexcept>
#include <utility>

namespace strobe::flow::detail::compiler {

inline StorageIR classify_storage(ActivationIR input) {
  const auto &components = input.input;
  const auto &classified = components.input.classified;
  const auto &lowered = classified.ordered.dependencies.lowered;
  const auto &values = lowered.resolved.values;

  Vector<ValueStorageInfo, allocator_ref> value_storage;
  value_storage.resize(values.values.size());

  for (ValueId value = 0; value < values.values.size(); ++value) {
    const auto &static_value = values.values[value];
    if (static_value.resource >= values.resources.size()) {
      throw std::logic_error("static value resource is out of range");
    }
    const auto resource_kind = values.resources[static_value.resource].kind;
    StorageClass storage = StorageClass::transient;
    if (resource_kind == ResourceKind::external) {
      storage = StorageClass::external;
    } else if (resource_kind == ResourceKind::phi ||
               resource_kind == ResourceKind::cutoff) {
      storage = StorageClass::virtual_value;
    }

    bool retained_by_feedback = false;
    for (const auto &backedge : lowered.resolved.backedges) {
      retained_by_feedback |= backedge.value == value;
    }

    bool crosses_activation = false;
    bool retained_by_memoization = false;
    if (static_value.producer != null_node_id) {
      if (static_value.producer >= components.component_of.size()) {
        throw std::logic_error("static value producer is out of range");
      }
      const auto producer_component =
          components.component_of[static_value.producer];
      const bool producer_runs_once =
          input.components[producer_component].runs_once;
      retained_by_memoization = input.components[producer_component].memoized;
      if (producer_runs_once) {
        for (NodeId node = 0; node < values.nodes.size(); ++node) {
          const auto range = values.nodes[node].accesses;
          if (range.offset + range.count > values.accesses.size()) {
            throw std::logic_error("node accesses are out of range");
          }
          bool consumes = false;
          for (std::uint32_t index = 0; index < range.count; ++index) {
            consumes |= values.accesses[range.offset + index].input == value;
          }
          if (consumes &&
              !input.components[components.component_of[node]].runs_once) {
            crosses_activation = true;
            break;
          }
        }
      }
    }

    if (storage == StorageClass::transient &&
        (retained_by_feedback || retained_by_memoization ||
         crosses_activation)) {
      storage = StorageClass::persistent;
    }
    value_storage[value] = ValueStorageInfo{
        .storage = storage,
        .retained_by_feedback = retained_by_feedback,
        .retained_by_memoization = retained_by_memoization,
        .crosses_activation = crosses_activation,
    };
  }

  Vector<ResourceStorageInfo, allocator_ref> resource_storage;
  resource_storage.reserve(values.resources.size());
  Vector<bool, allocator_ref> structured_resources(values.resources.size(),
                                                   false, allocator_ref{});
  const auto mark_control_resources = [&](SourceOperationId begin,
                                          SourceOperationId end,
                                          ValueId condition) {
    if (condition != null_value_id) {
      if (condition >= values.values.size()) {
        throw std::logic_error("structured control condition is invalid");
      }
      structured_resources[values.values[condition].resource] = true;
    }
    for (const auto &node : values.nodes) {
      if (node.source_operation == null_source_operation_id ||
          node.source_operation < begin || node.source_operation >= end) {
        continue;
      }
      if (node.accesses.offset + node.accesses.count > values.accesses.size()) {
        throw std::logic_error("structured control access is invalid");
      }
      for (std::uint32_t index = 0; index < node.accesses.count; ++index) {
        structured_resources[values.accesses[node.accesses.offset + index]
                                 .resource] = true;
      }
    }
  };
  for (const auto &branch : values.ifs) {
    mark_control_resources(branch.begin, branch.end, branch.condition);
  }
  for (const auto &phi : values.phis) {
    if (phi.condition != null_value_id) {
      mark_control_resources(phi.declaration_position, phi.feedback_position,
                             phi.condition);
    }
  }
  for (ResourceId resource = 0; resource < values.resources.size();
       ++resource) {
    const auto &source = values.resources[resource];
    StorageClass storage = StorageClass::transient;
    if (source.kind == ResourceKind::external) {
      storage = StorageClass::external;
    } else if (source.kind == ResourceKind::phi ||
               source.kind == ResourceKind::cutoff) {
      storage = StorageClass::virtual_value;
    } else if (resource < classified.preconstructed_resources.size() &&
               classified.preconstructed_resources[resource]) {
      storage = StorageClass::persistent;
    } else if (structured_resources[resource]) {
      storage = StorageClass::persistent;
    } else {
      for (ValueId value = 0; value < values.values.size(); ++value) {
        if (values.values[value].resource == resource &&
            value_storage[value].storage == StorageClass::persistent) {
          storage = StorageClass::persistent;
          break;
        }
      }
    }
    resource_storage.push_back(ResourceStorageInfo{
        .storage = storage,
        .size = source.size,
        .alignment = source.alignment,
        .destroy = source.destroy,
    });
  }

  return StorageIR(std::move(input), std::move(value_storage),
                   std::move(resource_storage));
}

} // namespace strobe::flow::detail::compiler
