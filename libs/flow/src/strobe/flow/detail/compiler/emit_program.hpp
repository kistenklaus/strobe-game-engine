#pragma once

#include "strobe/flow/detail/compiler/activation_layout_ir.hpp"
#include "strobe/flow/detail/op_stream.hpp"
#include "strobe/flow/program.hpp"

#include <strobe/core/containers/vector.hpp>
#include <strobe/core/graph.hpp>

#include <algorithm>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <utility>

namespace strobe::flow::detail::compiler {

inline OperationKind program_kind(NodeKind kind) {
  switch (kind) {
  case NodeKind::callback:
    return OperationKind::callback;
  case NodeKind::construct:
    return OperationKind::construct;
  case NodeKind::cutoff:
    return OperationKind::cutoff;
  case NodeKind::discard:
    return OperationKind::discard;
  case NodeKind::acquire:
    return OperationKind::acquire;
  case NodeKind::yield:
    return OperationKind::yield;
  case NodeKind::phi:
    return OperationKind::phi;
  }
  throw std::logic_error("unknown compiler node kind");
}

inline ProgramData emit_program(ActivationLayoutIR activation_layout,
                                detail::OpStream &stream,
                                CompileContext &context) {
  auto &schedule = activation_layout.input;
  const auto &layout = schedule.input;
  const auto &slots = layout.input;
  const auto &interference = slots.input;
  const auto &lifetimes = interference.input;
  const auto &storage = lifetimes.input;
  const auto &activation = storage.input;
  const auto &components = activation.input;
  const auto &graph_ir = components.input;
  const auto &graph = graph_ir.graph;
  const auto &values =
      graph_ir.classified.ordered.dependencies.lowered.resolved.values;

  using NodeList = Vector<NodeId, allocator_ref>;
  Vector<NodeList, allocator_ref> predecessors(graph.node_count(),
                                               allocator_ref{});
  Vector<NodeList, allocator_ref> successors(graph.node_count(),
                                             allocator_ref{});
  Vector<NodeList, allocator_ref> temporal_predecessors(graph.node_count(),
                                                        allocator_ref{});
  Vector<NodeList, allocator_ref> temporal_successors(graph.node_count(),
                                                      allocator_ref{});
  const auto append_unique = [](NodeList &list, NodeId node) {
    if (std::find(list.begin(), list.end(), node) == list.end()) {
      list.push_back(node);
    }
  };
  for (std::uint32_t index = 0; index < graph.edge_count(); ++index) {
    const auto edge = graph::EdgeId{index};
    const auto source = graph.source(edge).value;
    const auto destination = graph.destination(edge).value;
    if (graph.get(edge).kind == DependencyKind::temporal) {
      append_unique(temporal_successors[source], destination);
      append_unique(temporal_predecessors[destination], source);
    } else {
      append_unique(successors[source], destination);
      append_unique(predecessors[destination], source);
    }
  }

  struct FeedbackRegionKey {
    SourceOperationId declaration_position;
    SourceOperationId feedback_position;
  };
  Vector<FeedbackRegionKey, allocator_ref> feedback_region_keys;
  for (const auto &phi : values.phis) {
    if (phi.condition != null_value_id) {
      continue;
    }
    const auto existing = std::find_if(
        feedback_region_keys.begin(), feedback_region_keys.end(),
        [&](const FeedbackRegionKey &region) {
          return region.declaration_position == phi.declaration_position &&
                 region.feedback_position == phi.feedback_position;
        });
    if (existing == feedback_region_keys.end()) {
      feedback_region_keys.push_back({
          .declaration_position = phi.declaration_position,
          .feedback_position = phi.feedback_position,
      });
    }
  }

  constexpr auto no_feedback_region = std::numeric_limits<std::uint32_t>::max();
  Vector<std::uint32_t, allocator_ref> feedback_region_of(
      graph.node_count(), no_feedback_region, allocator_ref{});
  Vector<std::uint32_t, allocator_ref> feedback_region_node_counts(
      feedback_region_keys.size(), 0, allocator_ref{});
  for (NodeId node = 0; node < values.nodes.size(); ++node) {
    const auto &source = values.nodes[node];
    for (std::uint32_t region = 0; region < feedback_region_keys.size();
         ++region) {
      const auto &key = feedback_region_keys[region];
      bool is_region_phi = false;
      for (const auto &phi : values.phis) {
        is_region_phi |= phi.node == node &&
                         phi.declaration_position >= key.declaration_position &&
                         phi.feedback_position <= key.feedback_position;
      }
      const bool inside = source.source_operation != null_source_operation_id &&
                          source.source_operation >= key.declaration_position &&
                          source.source_operation < key.feedback_position;
      const bool included =
          (inside || is_region_phi) &&
          !(activation.components[components.component_of[node]].initialization &&
            !is_region_phi && source.kind != NodeKind::acquire);
      if (!included) {
        continue;
      }
      if (feedback_region_of[node] != no_feedback_region &&
          feedback_region_of[node] != region) {
        throw std::logic_error("feedback regions overlap");
      }
      feedback_region_of[node] = region;
      ++feedback_region_node_counts[region];
    }
  }

  Vector<std::uint32_t, allocator_ref> indegree(graph.node_count(), 0,
                                                allocator_ref{});
  NodeList ready;
  NodeList order;
  order.reserve(graph.node_count());
  for (NodeId node = 0; node < graph.node_count(); ++node) {
    indegree[node] = static_cast<std::uint32_t>(predecessors[node].size());
    if (indegree[node] == 0) {
      ready.push_back(node);
    }
  }
  auto select_ready = [&](std::uint32_t region) {
    const auto selected = std::min_element(
        ready.begin(), ready.end(), [&](NodeId left, NodeId right) {
          if (region == no_feedback_region) {
            const bool left_outside =
                feedback_region_of[left] == no_feedback_region;
            const bool right_outside =
                feedback_region_of[right] == no_feedback_region;
            if (left_outside != right_outside) {
              return left_outside;
            }
          } else {
            const bool left_inside = feedback_region_of[left] == region;
            const bool right_inside = feedback_region_of[right] == region;
            if (left_inside != right_inside) {
              return left_inside;
            }
          }
          return left < right;
        });
    if (selected == ready.end() ||
        (region != no_feedback_region &&
         feedback_region_of[*selected] != region) ||
        (region == no_feedback_region &&
         feedback_region_of[*selected] != no_feedback_region)) {
      return ready.end();
    }
    return selected;
  };
  std::uint32_t active_feedback_region = no_feedback_region;
  while (!ready.empty()) {
    auto next = ready.end();
    if (active_feedback_region != no_feedback_region) {
      next = select_ready(active_feedback_region);
      if (next == ready.end()) {
        if (feedback_region_node_counts[active_feedback_region] != 0) {
          throw std::logic_error(
              "feedback region cannot be emitted as a contiguous range");
        }
        active_feedback_region = no_feedback_region;
      }
    }
    if (next == ready.end()) {
      next = select_ready(no_feedback_region);
    }
    if (next == ready.end()) {
      next = std::min_element(ready.begin(), ready.end());
      active_feedback_region = feedback_region_of[*next];
    }
    const auto node = *next;
    ready.erase(next);
    order.push_back(node);
    if (feedback_region_of[node] != no_feedback_region) {
      auto &remaining = feedback_region_node_counts[feedback_region_of[node]];
      if (remaining == 0) {
        throw std::logic_error("feedback region node count underflow");
      }
      --remaining;
    }
    for (const auto destination : successors[node]) {
      if (--indegree[destination] == 0) {
        ready.push_back(destination);
      }
    }
  }
  if (order.size() != graph.node_count()) {
    throw std::logic_error("immediate runtime dependency graph is cyclic");
  }

  NodeList runtime_of(graph.node_count(), null_node_id, allocator_ref{});
  for (NodeId runtime = 0; runtime < order.size(); ++runtime) {
    runtime_of[order[runtime]] = runtime;
  }

  ProgramData data;
  data.source_operation_count = stream.ops().size();
  data.source_resource_count = stream.resources().size();
  data.source_external_count = stream.externals().size();
  data.storage_size = layout.size;
  data.storage_alignment = layout.alignment;
  data.activation_layout = ActivationLayout{
      .capacity = activation_layout.capacity,
      .stride = activation_layout.stride,
      .values_offset = activation_layout.values_offset,
      .arguments_offset = activation_layout.arguments_offset,
      .generations_offset = activation_layout.generations_offset,
      .previous_generations_offset =
          activation_layout.previous_generations_offset,
      .changed_values_offset = activation_layout.changed_values_offset,
      .size = activation_layout.size,
      .alignment = activation_layout.alignment,
  };
  data.nodes.reserve(order.size());
  data.callbacks.reserve(stream.ops().size());

  data.resources.reserve(values.resources.size());
  for (const auto &resource : values.resources) {
    data.resources.push_back(CompiledResource{
        .type = resource.type,
        .type_name = String<allocator_ref>(resource.type_name, allocator_ref{}),
        .label = String<allocator_ref>(resource.label, allocator_ref{}),
    });
  }

  for (const auto &value : values.values) {
    data.values.push_back(CompiledValue{
        .resource = value.resource,
        .version = value.version,
        .external = value.flags == StaticValueFlags::external,
    });
  }
  for (const auto &external : stream.externals()) {
    data.externals.push_back(CompiledExternal{
        .resource = external.resource,
        .name = String<allocator_ref>(external.name.view(), allocator_ref{}),
    });
  }
  for (std::size_t index = 0; index < stream.projections().size(); ++index) {
    const auto &projection = stream.projections()[index];
    data.projections.push_back(CompiledProjection{
        .resource = projection.resource,
        .owner = projection.owner,
        .kind = projection.kind,
        .name = String<allocator_ref>(projection.name.view(), allocator_ref{}),
        .resolve = stream.release_projection(index),
        .external = projection.external,
    });
  }
  for (const auto &phi : values.phis) {
    data.phis.push_back(CompiledPhi{
        .node = runtime_of[phi.node],
        .initial = phi.initial,
        .feedback = phi.feedback,
        .output = phi.output,
    });
  }
  for (const auto &source : values.phis) {
    if (source.condition != null_value_id) {
      continue;
    }
    const auto existing = std::find_if(
        data.feedback_regions.begin(), data.feedback_regions.end(),
        [&](const CompiledFeedbackRegion &region) {
          return region.declaration_position == source.declaration_position &&
                 region.feedback_position == source.feedback_position;
        });
    if (existing == data.feedback_regions.end()) {
      data.feedback_regions.push_back(CompiledFeedbackRegion{
          .declaration_position = source.declaration_position,
          .feedback_position = source.feedback_position,
          .phis = {},
          .nodes = {},
          .acquires = {},
          .token_requirements = {},
          .max_in_flight = source.max_in_flight,
      });
    } else if (existing->max_in_flight != source.max_in_flight) {
      throw std::logic_error("feedback relations in one region have "
                             "conflicting max_in_flight");
    }
  }
  for (auto &region : data.feedback_regions) {
    region.phis.offset =
        static_cast<std::uint32_t>(data.feedback_region_phis.size());
    for (std::uint32_t phi = 0; phi < values.phis.size(); ++phi) {
      const auto &source = values.phis[phi];
      if (region.declaration_position == source.declaration_position &&
          region.feedback_position == source.feedback_position) {
        data.feedback_region_phis.push_back(phi);
        ++region.phis.count;
      }
    }
  }
  for (const auto &acquire : stream.acquires()) {
    std::uint32_t semaphore = 0;
    for (; semaphore < data.semaphore_names.size(); ++semaphore) {
      if (data.semaphore_names[semaphore].view() == acquire.name.view()) {
        break;
      }
    }
    if (semaphore == data.semaphore_names.size()) {
      data.semaphore_names.emplace_back(acquire.name.view(), allocator_ref{});
    }
    NodeId source_node = null_node_id;
    for (NodeId node = 0; node < values.nodes.size(); ++node) {
      if (values.nodes[node].source_operation == acquire.operation) {
        source_node = node;
        break;
      }
    }
    if (source_node == null_node_id) {
      throw std::logic_error("acquire operation has no compiler node");
    }
    data.acquires.push_back(CompiledAcquire{
        .node = runtime_of[source_node],
        .semaphore = semaphore,
    });
  }
  const auto &lowered_cutoffs =
      graph_ir.classified.ordered.dependencies.lowered.cutoffs;
  for (const auto &cutoff : lowered_cutoffs) {
    const auto &source = stream.cutoffs()[cutoff.source_cutoff];
    data.cutoffs.push_back(CompiledCutoff{
        .node = runtime_of[cutoff.node],
        .input = cutoff.input,
        .output = cutoff.output,
        .equal = stream.release_cutoff_equal(cutoff.source_cutoff),
        .copy_construct = stream.release_cutoff_copy(cutoff.source_cutoff),
        .destroy = source.destroy,
        .size = source.size,
        .alignment = source.alignment,
    });
  }

  const auto append_remapped = [&](Vector<NodeId, allocator_ref> &flat,
                                   const NodeList &nodes) {
    const auto offset = flat.size();
    NodeList remapped;
    remapped.reserve(nodes.size());
    for (const auto node : nodes) {
      remapped.push_back(runtime_of[node]);
    }
    std::sort(remapped.begin(), remapped.end());
    for (const auto node : remapped) {
      flat.push_back(node);
    }
    return IndexRange{.offset = static_cast<std::uint32_t>(offset),
                      .count = static_cast<std::uint32_t>(remapped.size())};
  };

  for (NodeId runtime = 0; runtime < order.size(); ++runtime) {
    const auto source = order[runtime];
    const auto &node = values.nodes[source];
    const auto input_offset = data.node_storage.inputs.size();
    const auto output_offset = data.node_storage.outputs.size();
    const auto access_offset = data.node_storage.accesses.size();
    for (std::uint32_t index = 0; index < node.accesses.count; ++index) {
      const auto &access = values.accesses[node.accesses.offset + index];
      data.node_storage.accesses.push_back(CompiledAccess{
          .resource = access.resource,
          .mode = access.mode,
          .purpose = access.purpose,
          .input = access.input,
          .output = access.output,
      });
      if (access.input != null_value_id) {
        data.node_storage.inputs.push_back(access.input);
      }
      if (access.output != null_value_id) {
        data.node_storage.outputs.push_back(access.output);
      }
    }
    const auto immediate_predecessors =
        append_remapped(data.node_storage.predecessors, predecessors[source]);
    const auto immediate_successors =
        append_remapped(data.node_storage.successors, successors[source]);
    const auto prior_activation = append_remapped(
        data.node_storage.temporal_predecessors, temporal_predecessors[source]);
    const auto next_activation = append_remapped(
        data.node_storage.temporal_successors, temporal_successors[source]);
    CallbackId callback = null_callback_id;
    if (node.resolved_operation != null_resolved_operation_id) {
      auto resolved =
          context.release_resolved_operation(node.resolved_operation);
      callback = static_cast<CallbackId>(data.callbacks.size());
      CompiledCallback compiled_callback;
      compiled_callback.metadata = std::move(resolved.metadata);
      compiled_callback.external = std::move(resolved.external);
      compiled_callback.argument_types = std::move(resolved.argument_types);
      compiled_callback.argument_accesses =
          std::move(resolved.argument_accesses);
      const auto argument_group_offset = data.argument_groups.size();
      for (const auto group : resolved.argument_groups) {
        data.argument_groups.push_back(group);
      }
      compiled_callback.argument_groups = {
          .offset = static_cast<std::uint32_t>(argument_group_offset),
          .count = static_cast<std::uint32_t>(resolved.argument_groups.size())};
      const auto argument_element_offset = data.argument_elements.size();
      for (const auto element : resolved.argument_elements) {
        data.argument_elements.push_back(element);
      }
      compiled_callback.argument_elements = {
          .offset = static_cast<std::uint32_t>(argument_element_offset),
          .count =
              static_cast<std::uint32_t>(resolved.argument_elements.size())};
      const auto immediate_offset = data.immediates.size();
      for (const auto &immediate : resolved.immediates) {
        const auto aligned =
            (data.immediate_data.size() + immediate.alignment - 1) &
            ~(immediate.alignment - 1);
        data.immediate_data.resize(aligned + immediate.size);
        if (immediate.size != 0) {
          std::memcpy(data.immediate_data.data() + aligned,
                      resolved.immediate_data.data() + immediate.offset,
                      immediate.size);
        }
        data.immediates.push_back({immediate.type, aligned, immediate.size,
                                   immediate.alignment, immediate.element_size,
                                   immediate.element_count});
        data.immediate_alignment =
            std::max(data.immediate_alignment, immediate.alignment);
      }
      compiled_callback.immediates = {
          .offset = static_cast<std::uint32_t>(immediate_offset),
          .count = static_cast<std::uint32_t>(resolved.immediates.size())};
      compiled_callback.runtime = std::move(resolved.runtime);
      data.callbacks.push_back(std::move(compiled_callback));
    } else if (node.source_operation != null_source_operation_id) {
      const auto &recorded = stream.ops()[node.source_operation];
      if (!recorded.intrinsic) {
        callback = static_cast<CallbackId>(data.callbacks.size());
        CompiledCallback compiled_callback;
        compiled_callback.metadata = recorded.metadata;
        compiled_callback.external = recorded.external;
        compiled_callback.argument_types = recorded.argument_types;
        compiled_callback.argument_accesses = recorded.argument_accesses;
        const auto argument_group_offset = data.argument_groups.size();
        for (const auto group : recorded.argument_groups) {
          data.argument_groups.push_back(group);
        }
        compiled_callback.argument_groups = {
            .offset = static_cast<std::uint32_t>(argument_group_offset),
            .count =
                static_cast<std::uint32_t>(recorded.argument_groups.size())};
        const auto argument_element_offset = data.argument_elements.size();
        for (const auto element : recorded.argument_elements) {
          data.argument_elements.push_back(element);
        }
        compiled_callback.argument_elements = {
            .offset = static_cast<std::uint32_t>(argument_element_offset),
            .count =
                static_cast<std::uint32_t>(recorded.argument_elements.size())};
        auto runtime = stream.release_callback(node.source_operation);
        if (runtime) {
          compiled_callback.runtime = [runtime = std::move(runtime)](
                                          Lifetime lifetime, span<void *const>,
                                          span<const std::uint32_t>,
                                          span<const ArgumentGroup>,
                                          span<const std::uint32_t>,
                                          span<void *const> arguments) mutable {
            runtime(lifetime, arguments);
          };
        }
        data.callbacks.push_back(std::move(compiled_callback));
      }
    }
    data.nodes.push_back(CompiledNode{
        .source = source,
        .kind = program_kind(node.kind),
        .callback = callback,
        .inputs = {.offset = static_cast<std::uint32_t>(input_offset),
                   .count = static_cast<std::uint32_t>(
                       data.node_storage.inputs.size() - input_offset)},
        .outputs = {.offset = static_cast<std::uint32_t>(output_offset),
                    .count = static_cast<std::uint32_t>(
                        data.node_storage.outputs.size() - output_offset)},
        .accesses = {.offset = static_cast<std::uint32_t>(access_offset),
                     .count = node.accesses.count},
        .argument_count = node.argument_count,
        .predecessors = immediate_predecessors,
        .successors = immediate_successors,
        .temporal_predecessors = prior_activation,
        .temporal_successors = next_activation,
        .initialization =
            activation.components[components.component_of[source]].initialization &&
            node.kind != NodeKind::phi && node.kind != NodeKind::acquire,
        .memoized =
            activation.components[components.component_of[source]].memoized &&
            !graph.get(graph::NodeId{source}).classification.observable_effect,
    });
    if (immediate_predecessors.count == 0) {
      data.roots.push_back(runtime);
    }
    if (node.kind == NodeKind::yield) {
      data.yields.push_back(runtime);
    }
  }

  for (auto &region : data.feedback_regions) {
    region.nodes.offset =
        static_cast<std::uint32_t>(data.feedback_region_nodes.size());
    for (NodeId source = 0; source < values.nodes.size(); ++source) {
      const auto &node = values.nodes[source];
      bool is_region_phi = false;
      for (std::uint32_t index = 0; index < region.phis.count; ++index) {
        const auto phi = data.feedback_region_phis[region.phis.offset + index];
        is_region_phi |= values.phis[phi].node == source;
      }
      if (!is_region_phi) {
        for (const auto &phi : values.phis) {
          is_region_phi |=
              phi.node == source &&
              phi.declaration_position >= region.declaration_position &&
              phi.feedback_position <= region.feedback_position;
        }
      }
      const bool inside =
          node.source_operation != null_source_operation_id &&
          node.source_operation >= region.declaration_position &&
          node.source_operation < region.feedback_position;
      if ((!inside && !is_region_phi) ||
          (activation.components[components.component_of[source]].initialization &&
           !is_region_phi && node.kind != NodeKind::acquire)) {
        continue;
      }
      data.feedback_region_nodes.push_back(runtime_of[source]);
      ++region.nodes.count;
    }
    std::sort(data.feedback_region_nodes.begin() + region.nodes.offset,
              data.feedback_region_nodes.end());

    region.acquires.offset =
        static_cast<std::uint32_t>(data.feedback_region_acquires.size());
    for (std::uint32_t acquire = 0; acquire < data.acquires.size(); ++acquire) {
      const auto node = data.acquires[acquire].node;
      const auto begin =
          data.feedback_region_nodes.begin() + region.nodes.offset;
      const auto end = begin + region.nodes.count;
      if (std::find(begin, end, node) != end) {
        data.feedback_region_acquires.push_back(acquire);
        ++region.acquires.count;
      }
    }
    if (region.acquires.count == 0) {
      throw std::logic_error("feedback region has no compiled acquire");
    }

    region.token_requirements.offset =
        static_cast<std::uint32_t>(data.feedback_token_requirements.size());
    for (std::uint32_t index = 0; index < region.acquires.count; ++index) {
      const auto acquire =
          data.feedback_region_acquires[region.acquires.offset + index];
      if (acquire >= data.acquires.size()) {
        throw std::logic_error("feedback region acquire is out of range");
      }
      const auto semaphore = data.acquires[acquire].semaphore;
      const auto existing = std::find_if(
          data.feedback_token_requirements.begin() +
              region.token_requirements.offset,
          data.feedback_token_requirements.end(),
          [semaphore](const CompiledTokenRequirement &requirement) {
            return requirement.semaphore == semaphore;
          });
      if (existing == data.feedback_token_requirements.end()) {
        data.feedback_token_requirements.push_back({semaphore, 1});
        ++region.token_requirements.count;
      } else {
        ++existing->count;
      }
    }
  }

  struct ControlBuild {
    ControlKind kind;
    SourceOperationId begin;
    SourceOperationId alternative;
    SourceOperationId end;
    ValueId initial_condition;
    ValueId repeat_condition;
    ControlId parent = null_control_id;
  };
  Vector<ControlBuild, allocator_ref> control_builds;
  for (const auto &phi : values.phis) {
    if (phi.condition == null_value_id) {
      continue;
    }
    control_builds.push_back(ControlBuild{
        .kind = ControlKind::loop,
        .begin = phi.declaration_position,
        .alternative = phi.feedback_position,
        .end = phi.feedback_position,
        .initial_condition = phi.initial,
        .repeat_condition = phi.condition,
    });
  }
  for (const auto &branch : values.ifs) {
    control_builds.push_back(ControlBuild{
        .kind = ControlKind::branch,
        .begin = branch.begin,
        .alternative = branch.alternative,
        .end = branch.end,
        .initial_condition = branch.condition,
        .repeat_condition = null_value_id,
    });
  }
  std::sort(control_builds.begin(), control_builds.end(),
            [](const ControlBuild &left, const ControlBuild &right) {
              if (left.begin != right.begin) {
                return left.begin < right.begin;
              }
              return left.end > right.end;
            });
  for (ControlId left = 0; left < control_builds.size(); ++left) {
    const auto &control = control_builds[left];
    if (control.begin >= control.end || control.alternative < control.begin ||
        control.alternative > control.end) {
      throw std::logic_error("structured control interval is invalid");
    }
    for (ControlId right = left + 1; right < control_builds.size(); ++right) {
      const auto &other = control_builds[right];
      if (control.begin == other.begin || control.end == other.end) {
        throw std::logic_error(
            "structured control regions must have distinct boundaries");
      }
      const bool crosses =
          (control.begin < other.begin && other.begin < control.end &&
           control.end < other.end) ||
          (other.begin < control.begin && control.begin < other.end &&
           other.end < control.end);
      if (crosses) {
        throw std::logic_error("structured control regions cannot overlap");
      }
    }
  }
  for (ControlId control = 0; control < control_builds.size(); ++control) {
    for (ControlId candidate = 0; candidate < control_builds.size();
         ++candidate) {
      if (candidate == control ||
          control_builds[candidate].begin >= control_builds[control].begin ||
          control_builds[candidate].end <= control_builds[control].end) {
        continue;
      }
      if (control_builds[control].parent == null_control_id ||
          control_builds[candidate].begin >
              control_builds[control_builds[control].parent].begin) {
        control_builds[control].parent = candidate;
      }
    }
  }

  Vector<ControlId, allocator_ref> innermost(values.nodes.size(),
                                             null_control_id, allocator_ref{});
  const auto source_position = [&](NodeId node) {
    if (values.nodes[node].kind != NodeKind::phi) {
      return values.nodes[node].source_operation;
    }
    for (const auto &phi : values.phis) {
      if (phi.node == node) {
        return phi.declaration_position;
      }
    }
    throw std::logic_error("phi node has no source position");
  };
  for (NodeId source = 0; source < values.nodes.size(); ++source) {
    const auto position = source_position(source);
    for (ControlId control = 0; control < control_builds.size(); ++control) {
      if (position < control_builds[control].begin ||
          position >= control_builds[control].end) {
        continue;
      }
      if (innermost[source] == null_control_id ||
          control_builds[control].begin >=
              control_builds[innermost[source]].begin) {
        innermost[source] = control;
      }
    }
  }

  data.control_of_node.resize(data.nodes.size(), null_control_id);
  const auto append_steps = [&](ControlId control, SourceOperationId begin,
                                SourceOperationId end) {
    const auto offset = data.control_steps.size();
    for (SourceOperationId position = begin; position < end;) {
      ControlId child = null_control_id;
      for (ControlId candidate = 0; candidate < control_builds.size();
           ++candidate) {
        if (control_builds[candidate].parent == control &&
            control_builds[candidate].begin == position) {
          child = candidate;
          break;
        }
      }
      if (child != null_control_id) {
        data.control_steps.push_back(CompiledControlStep{
            .kind = ControlStepKind::control,
            .index = child,
        });
        position = control_builds[child].end;
        continue;
      }
      for (const auto &phi : values.phis) {
        if (phi.declaration_position == position &&
            innermost[phi.node] == control) {
          data.control_steps.push_back(CompiledControlStep{
              .kind = ControlStepKind::node,
              .index = runtime_of[phi.node],
          });
        }
      }
      for (NodeId source = 0; source < values.nodes.size(); ++source) {
        if (values.nodes[source].kind != NodeKind::phi &&
            values.nodes[source].source_operation == position &&
            innermost[source] == control) {
          data.control_steps.push_back(CompiledControlStep{
              .kind = ControlStepKind::node,
              .index = runtime_of[source],
          });
        }
      }
      ++position;
    }
    return IndexRange{
        .offset = static_cast<std::uint32_t>(offset),
        .count = static_cast<std::uint32_t>(data.control_steps.size() - offset),
    };
  };
  const auto append_written_resources = [&](ControlId control,
                                            SourceOperationId begin,
                                            SourceOperationId end) {
    const auto offset = data.control_written_resources.size();
    for (NodeId source = 0; source < values.nodes.size(); ++source) {
      const auto position = source_position(source);
      if (position < begin || position >= end || innermost[source] != control) {
        continue;
      }
      const auto accesses = values.nodes[source].accesses;
      for (std::uint32_t index = 0; index < accesses.count; ++index) {
        const auto &access = values.accesses[accesses.offset + index];
        if (access.output == null_value_id ||
            std::find(data.control_written_resources.begin() + offset,
                      data.control_written_resources.end(), access.resource) !=
                data.control_written_resources.end()) {
          continue;
        }
        data.control_written_resources.push_back(access.resource);
      }
    }
    return IndexRange{
        .offset = static_cast<std::uint32_t>(offset),
        .count = static_cast<std::uint32_t>(
            data.control_written_resources.size() - offset),
    };
  };

  for (ControlId control = 0; control < control_builds.size(); ++control) {
    const auto &source_control = control_builds[control];
    const auto primary_end = source_control.kind == ControlKind::branch
                                 ? source_control.alternative
                                 : source_control.end;
    const auto primary_steps =
        append_steps(control, source_control.begin, primary_end);
    IndexRange secondary_steps{};
    if (source_control.kind == ControlKind::branch) {
      secondary_steps =
          append_steps(control, source_control.alternative, source_control.end);
    }

    const auto node_offset = data.control_nodes.size();
    for (NodeId runtime = 0; runtime < order.size(); ++runtime) {
      const auto source = order[runtime];
      const auto position = source_position(source);
      if (position >= source_control.begin && position < source_control.end) {
        data.control_nodes.push_back(runtime);
        auto outermost = control;
        while (control_builds[outermost].parent != null_control_id) {
          outermost = control_builds[outermost].parent;
        }
        data.control_of_node[runtime] = outermost;
      }
    }

    const auto resource_offset = data.control_resources.size();
    if (source_control.kind == ControlKind::loop) {
      for (NodeId source = 0; source < values.nodes.size(); ++source) {
        if (innermost[source] != control ||
            values.nodes[source].kind != NodeKind::construct ||
            values.nodes[source].accesses.count == 0) {
          continue;
        }
        const auto resource =
            values.accesses[values.nodes[source].accesses.offset].resource;
        if (std::find(data.control_resources.begin() + resource_offset,
                      data.control_resources.end(),
                      resource) == data.control_resources.end()) {
          data.control_resources.push_back(resource);
        }
      }
    }
    const auto primary_written =
        append_written_resources(control, source_control.begin, primary_end);
    IndexRange secondary_written{};
    if (source_control.kind == ControlKind::branch) {
      secondary_written = append_written_resources(
          control, source_control.alternative, source_control.end);
    }
    data.controls.push_back(CompiledControl{
        .kind = source_control.kind,
        .parent = source_control.parent,
        .initial_condition = source_control.initial_condition,
        .repeat_condition = source_control.repeat_condition,
        .initial_condition_node =
            values.values[source_control.initial_condition].producer ==
                    null_node_id
                ? null_node_id
                : runtime_of[values.values[source_control.initial_condition]
                                 .producer],
        .primary_steps = primary_steps,
        .secondary_steps = secondary_steps,
        .nodes =
            {
                .offset = static_cast<std::uint32_t>(node_offset),
                .count = static_cast<std::uint32_t>(data.control_nodes.size() -
                                                    node_offset),
            },
        .resources =
            {
                .offset = static_cast<std::uint32_t>(resource_offset),
                .count = static_cast<std::uint32_t>(
                    data.control_resources.size() - resource_offset),
            },
        .primary_written_resources = primary_written,
        .secondary_written_resources = secondary_written,
    });
  }

  if (layout.resources.size() != data.resources.size() ||
      schedule.resources.size() != data.resources.size()) {
    throw std::logic_error("compiled resource tables have different sizes");
  }
  for (ResourceId resource = 0; resource < layout.resources.size();
       ++resource) {
    const auto &slot = layout.resources[resource];
    data.resources[resource].slots = ResourceSlotPlan{
        .offset = slot.offset == no_storage_offset ? null_storage_offset
                                                   : slot.offset,
        .slot_stride = slot.slot_stride,
        .slot_count = slot.slot_count,
        .bounded_overlap = slot.bounded_overlap,
    };
  }
  for (ResourceId resource = 0; resource < schedule.resources.size();
       ++resource) {
    const auto &lifetime = schedule.resources[resource];
    const auto &analyzed = lifetimes.resources[resource];
    auto interval = Lifetime{};
    if (analyzed.storage == StorageClass::persistent) {
      interval = Lifetime{
          .begin = 0,
          .end = std::numeric_limits<std::uint32_t>::max(),
      };
    } else if (analyzed.definitions.count != 0) {
      interval.begin = std::numeric_limits<std::uint32_t>::max();
      for (std::uint32_t index = 0; index < analyzed.definitions.count;
           ++index) {
        interval.begin = std::min(
            interval.begin,
            runtime_of[lifetimes
                           .definitions[analyzed.definitions.offset + index]]);
      }
      interval.end = interval.begin + 1;
      for (std::uint32_t index = 0; index < analyzed.uses.count; ++index) {
        const auto use =
            runtime_of[lifetimes.uses[analyzed.uses.offset + index]];
        if (use == std::numeric_limits<std::uint32_t>::max()) {
          throw std::length_error("Flow lifetime position overflow");
        }
        interval.end = std::max(interval.end, use + 1);
      }
    }
    const auto construction_offset = data.constructions.size();
    for (std::uint32_t index = 0; index < lifetime.constructions.count;
         ++index) {
      data.constructions.push_back(
          runtime_of[schedule.constructions[lifetime.constructions.offset +
                                            index]]);
    }
    const auto release_offset = data.release_after.size();
    for (std::uint32_t index = 0; index < lifetime.release_after.count;
         ++index) {
      data.release_after.push_back(
          runtime_of[schedule.release_after[lifetime.release_after.offset +
                                            index]]);
    }
    std::sort(data.release_after.begin() + release_offset,
              data.release_after.end());
    data.resources[resource].lifetime = ResourceLifetimePlan{
        .interval = interval,
        .constructions =
            {
                .offset = static_cast<std::uint32_t>(construction_offset),
                .count = lifetime.constructions.count,
            },
        .release_after =
            {
                .offset = static_cast<std::uint32_t>(release_offset),
                .count = lifetime.release_after.count,
            },
        .repeating = lifetime.repeating,
        .preconstructed = lifetime.preconstructed,
        .retained_by_feedback = lifetime.retained_by_feedback,
        .destroy_at_shutdown = lifetime.destroy_at_shutdown,
        .destroy = lifetime.destroy,
    };
  }
  return data;
}

} // namespace strobe::flow::detail::compiler
