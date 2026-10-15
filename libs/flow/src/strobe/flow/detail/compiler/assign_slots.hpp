#pragma once

#include "strobe/flow/compile_options.hpp"
#include "strobe/flow/detail/compiler/slot_ir.hpp"

#include <strobe/core/containers/vector.hpp>

#include <stdexcept>
#include <utility>

namespace strobe::flow::detail::compiler {

inline SlotIR assign_slots(InterferenceIR input,
                           const CompileOptions &options = {}) {
  if (options.max_slots == 0) {
    throw std::invalid_argument("Flow max_slots must be greater than zero");
  }

  const auto &lifetimes = input.input;
  const auto &components = lifetimes.input.input.input;
  const auto &values =
      components.input.classified.ordered.dependencies.lowered.resolved.values;
  Vector<SourceOperationId, allocator_ref> yields;
  bool has_acquire = false;
  for (const auto &node : values.nodes) {
    if (node.kind == NodeKind::yield || node.kind == NodeKind::acquire) {
      yields.push_back(node.source_operation);
    }
    has_acquire |= node.kind == NodeKind::acquire;
  }

  Vector<bool, allocator_ref> yielding_cycles;
  yielding_cycles.resize(components.components.size());
  for (const auto &phi : values.phis) {
    if (phi.node >= components.component_of.size()) {
      throw std::logic_error("phi component is out of range");
    }
    SourceOperationId declaration = phi.feedback_position;
    for (NodeId node = phi.node + 1; node < values.nodes.size(); ++node) {
      if (values.nodes[node].source_operation != null_source_operation_id) {
        declaration = values.nodes[node].source_operation;
        break;
      }
    }
    for (const auto position : yields) {
      if (position >= declaration && position < phi.feedback_position) {
        yielding_cycles[components.component_of[phi.node]] = true;
        break;
      }
    }
  }

  Vector<bool, allocator_ref> structured_loop_resources(values.resources.size(),
                                                        false, allocator_ref{});
  for (const auto &phi : values.phis) {
    if (phi.condition == null_value_id) {
      continue;
    }
    for (const auto &node : values.nodes) {
      if (node.kind != NodeKind::construct ||
          node.source_operation < phi.declaration_position ||
          node.source_operation >= phi.feedback_position ||
          node.accesses.count == 0) {
        continue;
      }
      const auto resource = values.accesses[node.accesses.offset].resource;
      structured_loop_resources[resource] = true;
    }
  }

  Vector<ResourceSlotInfo, allocator_ref> resources;
  resources.reserve(lifetimes.resources.size());
  for (ResourceId resource = 0; resource < lifetimes.resources.size();
       ++resource) {
    const auto &lifetime = lifetimes.resources[resource];
    if (lifetime.storage == StorageClass::external ||
        lifetime.storage == StorageClass::virtual_value) {
      resources.push_back(ResourceSlotInfo{
          .slot_count = 0,
          .bounded_overlap = false,
      });
      continue;
    }

    bool crosses_yielding_cycle = false;
    const auto sources = lifetime.activation_sources;
    if (sources.offset + sources.count > lifetimes.activation_sources.size()) {
      throw std::logic_error("resource activation sources are out of range");
    }
    for (std::uint32_t index = 0; index < sources.count; ++index) {
      const auto component =
          lifetimes.activation_sources[sources.offset + index];
      if (component >= yielding_cycles.size()) {
        throw std::logic_error("resource activation component is out of range");
      }
      crosses_yielding_cycle |= yielding_cycles[component];
    }
    const bool participates_in_feedback = sources.count != 0;
    if (lifetime.may_overlap_generations && participates_in_feedback &&
        options.max_slots < 2) {
      throw std::invalid_argument(
          "feedback resource requires at least two storage slots");
    }
    const bool uses_activation_bound =
        (has_acquire && lifetime.externally_triggered) ||
        crosses_yielding_cycle;
    const bool replaces_persistent_value =
        lifetime.storage == StorageClass::persistent &&
        lifetime.memoized_reconstruction;
    if (replaces_persistent_value && uses_activation_bound &&
        options.max_slots < 2) {
      throw std::invalid_argument(
          "memoized resource requires at least two storage slots");
    }
    const bool bounded_overlap =
        lifetime.may_overlap_generations &&
        (uses_activation_bound || participates_in_feedback ||
         replaces_persistent_value);
    if (structured_loop_resources[resource] && options.max_slots < 2) {
      throw std::invalid_argument(
          "structured loop resource requires at least two storage slots");
    }
    const auto slot_count = structured_loop_resources[resource] ? 2U
                            : !bounded_overlap                  ? 1U
                            : uses_activation_bound ? options.max_slots
                                                    : 2U;
    resources.push_back(ResourceSlotInfo{
        .slot_count = slot_count,
        .bounded_overlap =
            bounded_overlap || structured_loop_resources[resource],
    });
  }

  return SlotIR(std::move(input), std::move(resources));
}

} // namespace strobe::flow::detail::compiler
