#pragma once

#include "strobe/flow/detail/compiler/activation_layout_ir.hpp"
#include "strobe/flow/detail/compiler/dependency_ir.hpp"
#include "strobe/flow/detail/compiler/graph_ir.hpp"
#include "strobe/flow/detail/compiler/lifetime_ir.hpp"
#include "strobe/flow/detail/compiler/normalized_ir.hpp"
#include "strobe/flow/detail/compiler/ordered_ir.hpp"
#include "strobe/flow/detail/compiler/storage_ir.hpp"
#include "strobe/flow/detail/compiler/storage_layout_ir.hpp"
#include "strobe/flow/detail/compiler/value_ir.hpp"
#include "strobe/flow/program.hpp"

#include <cstddef>
#include <stdexcept>

namespace strobe::flow::detail::compiler {

inline bool valid_range(IndexRange range, std::size_t size) noexcept {
  return range.offset <= size && range.count <= size - range.offset;
}

inline bool valid_alignment(std::size_t alignment) noexcept {
  return alignment != 0 && (alignment & (alignment - 1)) == 0;
}

inline void validate(const NormalizedIR &ir) {
  for (const auto &resource : ir.resources) {
    if (resource.type.value == 0 || !valid_alignment(resource.alignment)) {
      throw std::logic_error("invalid normalized resource description");
    }
  }
  for (const auto &node : ir.nodes) {
    if (!valid_range(node.accesses, ir.accesses.size())) {
      throw std::logic_error("normalized node access range is invalid");
    }
  }
  for (const auto &access : ir.accesses) {
    if (access.resource >= ir.resources.size()) {
      throw std::logic_error("normalized access resource is out of range");
    }
  }
  for (const auto &phi : ir.phis) {
    if (phi.node >= ir.nodes.size() || phi.output >= ir.resources.size() ||
        phi.initial >= ir.resources.size() ||
        phi.feedback >= ir.resources.size() ||
        (phi.condition && *phi.condition >= ir.resources.size()) ||
        phi.declaration_position > phi.feedback_position) {
      throw std::logic_error("normalized phi is out of range");
    }
    if (phi.condition && ir.resources[*phi.condition].type != type_id<bool>()) {
      throw std::logic_error("normalized phi condition is not boolean");
    }
  }
  for (const auto &cutoff : ir.cutoffs) {
    if (cutoff.node >= ir.nodes.size() || cutoff.input >= ir.resources.size() ||
        cutoff.output >= ir.resources.size()) {
      throw std::logic_error("normalized cutoff is out of range");
    }
  }
  for (const auto &external : ir.externals) {
    if (external.resource >= ir.resources.size()) {
      throw std::logic_error("normalized external is out of range");
    }
  }
  for (const auto &branch : ir.ifs) {
    if (branch.condition >= ir.resources.size() ||
        ir.resources[branch.condition].type != type_id<bool>() ||
        branch.begin > branch.alternative || branch.alternative > branch.end) {
      throw std::logic_error("normalized if is invalid");
    }
  }
}

inline void validate(const ValueIR &ir) {
  for (const auto &resource : ir.resources) {
    if (resource.type.value == 0 || !valid_alignment(resource.alignment)) {
      throw std::logic_error("invalid value IR resource description");
    }
  }
  for (const auto &node : ir.nodes) {
    if (!valid_range(node.accesses, ir.accesses.size())) {
      throw std::logic_error("value node access range is invalid");
    }
  }
  for (const auto &access : ir.accesses) {
    if (access.resource >= ir.resources.size() ||
        (access.input != null_value_id && access.input >= ir.values.size()) ||
        (access.output != null_value_id && access.output >= ir.values.size())) {
      throw std::logic_error("value access is out of range");
    }
  }
  for (const auto &value : ir.values) {
    if (value.resource >= ir.resources.size() ||
        (value.producer != null_node_id && value.producer >= ir.nodes.size())) {
      throw std::logic_error("static value is out of range");
    }
  }
  for (const auto &phi : ir.phis) {
    if (phi.node >= ir.nodes.size() || phi.output >= ir.values.size() ||
        phi.initial >= ir.values.size() || phi.feedback >= ir.values.size() ||
        phi.feedback_resource >= ir.resources.size() ||
        (phi.condition_resource &&
         *phi.condition_resource >= ir.resources.size()) ||
        (phi.condition != null_value_id && phi.condition >= ir.values.size()) ||
        phi.declaration_position > phi.feedback_position) {
      throw std::logic_error("value phi is out of range");
    }
    if ((phi.condition_resource.has_value()) !=
        (phi.condition != null_value_id)) {
      throw std::logic_error("value phi condition is incomplete");
    }
    if (phi.condition_resource &&
        ir.resources[*phi.condition_resource].type != type_id<bool>()) {
      throw std::logic_error("value phi condition is not boolean");
    }
  }
  for (const auto &branch : ir.ifs) {
    if (branch.condition_resource >= ir.resources.size() ||
        branch.condition >= ir.values.size() ||
        ir.resources[branch.condition_resource].type != type_id<bool>() ||
        branch.begin > branch.alternative || branch.alternative > branch.end) {
      throw std::logic_error("value if is invalid");
    }
  }
}

inline void validate(const DependencyIR &ir) {
  validate(ir.lowered.resolved.values);
  const auto &values = ir.lowered.resolved.values;
  for (const auto &edge : ir.edges) {
    if (edge.producer >= values.nodes.size() ||
        edge.consumer >= values.nodes.size() ||
        edge.value >= values.values.size()) {
      throw std::logic_error("producer-consumer edge is out of range");
    }
  }
}

inline void validate(const OrderedIR &ir) {
  validate(ir.dependencies);
  const auto &values = ir.dependencies.lowered.resolved.values;
  for (const auto &constraint : ir.resource_order) {
    if (constraint.before >= values.nodes.size() ||
        constraint.after >= values.nodes.size() ||
        (constraint.resource != null_resource_id &&
         constraint.resource >= values.resources.size())) {
      throw std::logic_error("resource order constraint is out of range");
    }
  }
}

inline void validate(const GraphIR &ir) {
  validate(ir.classified.ordered);
  const auto &values =
      ir.classified.ordered.dependencies.lowered.resolved.values;
  if (ir.graph.node_count() != values.nodes.size()) {
    throw std::logic_error("dependency graph node count does not match IR");
  }
  for (std::uint32_t node = 0; node < ir.graph.node_count(); ++node) {
    if (ir.graph.get(graph::NodeId{node}).source >= values.nodes.size()) {
      throw std::logic_error("dependency graph node source is out of range");
    }
  }
  for (std::uint32_t edge = 0; edge < ir.graph.edge_count(); ++edge) {
    const auto &payload = ir.graph.get(graph::EdgeId{edge});
    if ((payload.value != null_value_id &&
         payload.value >= values.values.size()) ||
        (payload.resource != null_resource_id &&
         payload.resource >= values.resources.size())) {
      throw std::logic_error("dependency graph edge payload is out of range");
    }
  }
}

inline void validate(const StorageIR &ir) {
  const auto &values = ir.input.input.input.classified.ordered.dependencies
                           .lowered.resolved.values;
  if (ir.values.size() != values.values.size() ||
      ir.resources.size() != values.resources.size()) {
    throw std::logic_error("storage classification size does not match IR");
  }
  for (const auto &resource : ir.resources) {
    if (!valid_alignment(resource.alignment)) {
      throw std::logic_error("storage resource alignment is invalid");
    }
  }
}

inline void validate(const LifetimeIR &ir) {
  validate(ir.input);
  if (ir.resources.size() != ir.input.resources.size()) {
    throw std::logic_error("lifetime resource count does not match storage");
  }
  const auto node_count = ir.input.input.input.input.graph.node_count();
  for (const auto &resource : ir.resources) {
    if (!valid_range(resource.definitions, ir.definitions.size()) ||
        !valid_range(resource.uses, ir.uses.size()) ||
        !valid_range(resource.last_uses, ir.last_uses.size()) ||
        !valid_range(resource.activation_sources,
                     ir.activation_sources.size())) {
      throw std::logic_error("resource lifetime range is invalid");
    }
  }
  for (const auto node : ir.definitions) {
    if (node >= node_count) {
      throw std::logic_error("lifetime definition is out of range");
    }
  }
  for (const auto node : ir.uses) {
    if (node >= node_count) {
      throw std::logic_error("lifetime use is out of range");
    }
  }
}

inline void validate(const StorageLayoutIR &ir) {
  const auto &storage = ir.input.input.input.input;
  if (ir.resources.size() != storage.resources.size() ||
      !valid_alignment(ir.alignment)) {
    throw std::logic_error("storage layout does not match resources");
  }
  for (std::size_t index = 0; index < ir.resources.size(); ++index) {
    const auto &layout = ir.resources[index];
    if (layout.offset == no_storage_offset) {
      continue;
    }
    if (layout.slot_count == 0 ||
        layout.offset % storage.resources[index].alignment != 0 ||
        layout.offset > ir.size ||
        layout.slot_stride > ir.size - layout.offset) {
      throw std::logic_error("resource storage layout is invalid");
    }
  }
}

inline void validate(const ActivationLayoutIR &ir) {
  if (ir.capacity == 0 || !valid_alignment(ir.alignment) ||
      (ir.capacity != 0 && ir.stride > ir.size / ir.capacity)) {
    throw std::logic_error("activation layout is invalid");
  }
  const auto offsets_valid = ir.values_offset <= ir.stride &&
                             ir.arguments_offset <= ir.stride &&
                             ir.generations_offset <= ir.stride &&
                             ir.previous_generations_offset <= ir.stride &&
                             ir.changed_values_offset <= ir.stride;
  if (!offsets_valid) {
    throw std::logic_error("activation layout offset is out of range");
  }
}

inline void validate(const ProgramData &data) {
  if (data.resource_types.size() != data.source_resource_count ||
      data.resource_slots.size() != data.source_resource_count ||
      data.resource_lifetimes.size() != data.source_resource_count ||
      !valid_alignment(data.storage_alignment)) {
    throw std::logic_error("compiled resource tables do not match source");
  }
  for (const auto &node : data.nodes) {
    if (!valid_range(node.inputs, data.inputs.size()) ||
        !valid_range(node.outputs, data.outputs.size()) ||
        !valid_range(node.accesses, data.accesses.size()) ||
        !valid_range(node.predecessors, data.predecessors.size()) ||
        !valid_range(node.successors, data.successors.size()) ||
        !valid_range(node.temporal_predecessors,
                     data.temporal_predecessors.size()) ||
        !valid_range(node.temporal_successors,
                     data.temporal_successors.size()) ||
        (node.callback != null_callback_id &&
         node.callback >= data.callbacks.size())) {
      throw std::logic_error("compiled node is invalid");
    }
  }
  for (const auto &access : data.accesses) {
    if (access.resource >= data.source_resource_count ||
        (access.input != null_value_id && access.input >= data.values.size()) ||
        (access.output != null_value_id &&
         access.output >= data.values.size())) {
      throw std::logic_error("compiled access is out of range");
    }
  }
  for (const auto &callback : data.callbacks) {
    if (callback.argument_types.size() != callback.argument_accesses.size()) {
      throw std::logic_error("compiled callback argument tables do not match");
    }
  }
  if (!data.control_of_node.empty() &&
      data.control_of_node.size() != data.nodes.size()) {
    throw std::logic_error("compiled control node map is invalid");
  }
  for (ControlId control = 0; control < data.controls.size(); ++control) {
    const auto &compiled = data.controls[control];
    if ((compiled.parent != null_control_id &&
         compiled.parent >= data.controls.size()) ||
        compiled.parent == control ||
        compiled.initial_condition >= data.values.size() ||
        (compiled.initial_condition_node != null_node_id &&
         compiled.initial_condition_node >= data.nodes.size()) ||
        (compiled.kind == ControlKind::loop &&
         compiled.repeat_condition >= data.values.size()) ||
        !valid_range(compiled.primary_steps, data.control_steps.size()) ||
        !valid_range(compiled.secondary_steps, data.control_steps.size()) ||
        !valid_range(compiled.nodes, data.control_nodes.size()) ||
        !valid_range(compiled.resources, data.control_resources.size()) ||
        !valid_range(compiled.primary_written_resources,
                     data.control_written_resources.size()) ||
        !valid_range(compiled.secondary_written_resources,
                     data.control_written_resources.size())) {
      throw std::logic_error("compiled control is invalid");
    }
    const auto condition_resource =
        data.values[compiled.initial_condition].resource;
    if (condition_resource >= data.resource_types.size() ||
        data.resource_types[condition_resource] != type_id<bool>()) {
      throw std::logic_error("compiled control condition is invalid");
    }
  }
  for (const auto &step : data.control_steps) {
    if ((step.kind == ControlStepKind::node &&
         step.index >= data.nodes.size()) ||
        (step.kind == ControlStepKind::control &&
         step.index >= data.controls.size())) {
      throw std::logic_error("compiled control step is invalid");
    }
  }
  for (const auto node : data.control_nodes) {
    if (node >= data.nodes.size()) {
      throw std::logic_error("compiled control node is out of range");
    }
  }
  for (const auto resource : data.control_resources) {
    if (resource >= data.source_resource_count) {
      throw std::logic_error("compiled control resource is out of range");
    }
  }
  for (const auto resource : data.control_written_resources) {
    if (resource >= data.source_resource_count) {
      throw std::logic_error("compiled control write is out of range");
    }
  }
  for (const auto control : data.control_of_node) {
    if (control != null_control_id && control >= data.controls.size()) {
      throw std::logic_error("compiled control mapping is invalid");
    }
  }
}

} // namespace strobe::flow::detail::compiler
