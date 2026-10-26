#pragma once

#include "strobe/flow/detail/compiler/slot_ir.hpp"

#include <strobe/core/containers/vector.hpp>

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <utility>

namespace strobe::flow::detail::compiler {

inline SlotIR assign_slots(InterferenceIR input) {

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
  Vector<std::uint32_t, allocator_ref> component_limits(
      components.components.size(), 0, allocator_ref{});
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
    if (phi.condition == null_value_id) {
      const auto component = components.component_of[phi.node];
      component_limits[component] =
          std::max(component_limits[component], phi.max_in_flight);
    }
  }

  Vector<bool, allocator_ref> structured_loop_resources(values.resources.size(),
                                                        false, allocator_ref{});
  Vector<std::uint32_t, allocator_ref> feedback_resource_limits(
      values.resources.size(), 0, allocator_ref{});
  for (const auto &phi : values.phis) {
    for (const auto &node : values.nodes) {
      if (node.source_operation < phi.declaration_position ||
          node.source_operation >= phi.feedback_position) {
        continue;
      }
      for (std::uint32_t index = 0; index < node.accesses.count; ++index) {
        const auto &access = values.accesses[node.accesses.offset + index];
        if (access.output == null_value_id) {
          continue;
        }
        if (phi.condition == null_value_id) {
          feedback_resource_limits[access.resource] = std::max(
              feedback_resource_limits[access.resource], phi.max_in_flight);
        } else if (node.kind == NodeKind::construct) {
          structured_loop_resources[access.resource] = true;
        }
      }
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
    std::uint32_t feedback_limit = 0;
    Vector<bool, allocator_ref> counted_components(components.components.size(),
                                                   false, allocator_ref{});
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
      if (!counted_components[component]) {
        if (feedback_limit > std::numeric_limits<std::uint32_t>::max() -
                                 component_limits[component]) {
          throw std::length_error("feedback in-flight limit overflow");
        }
        feedback_limit += component_limits[component];
        counted_components[component] = true;
      }
    }
    feedback_limit =
        std::max(feedback_limit, feedback_resource_limits[resource]);
    const bool has_feedback_limit = feedback_limit != 0;
    feedback_limit = std::max(feedback_limit, 2U);
    const bool participates_in_feedback = sources.count != 0;
    if (lifetime.may_overlap_generations && participates_in_feedback &&
        feedback_limit < 2) {
      throw std::invalid_argument(
          "feedback resource requires at least two storage slots");
    }
    const bool uses_activation_bound =
        (has_acquire && lifetime.externally_triggered && has_feedback_limit) ||
        crosses_yielding_cycle;
    const bool replaces_persistent_value =
        lifetime.storage == StorageClass::persistent &&
        lifetime.memoized_reconstruction;
    if (replaces_persistent_value && uses_activation_bound &&
        feedback_limit < 2) {
      throw std::invalid_argument(
          "memoized resource requires at least two storage slots");
    }
    const bool bounded_overlap =
        lifetime.may_overlap_generations &&
        (uses_activation_bound || participates_in_feedback ||
         replaces_persistent_value);
    const auto slot_count = structured_loop_resources[resource] ? 2U
                            : !bounded_overlap                  ? 1U
                            : uses_activation_bound             ? feedback_limit
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
