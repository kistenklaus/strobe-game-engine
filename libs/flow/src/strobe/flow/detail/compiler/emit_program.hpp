#pragma once

#include "strobe/flow/detail/compiler/activation_layout_ir.hpp"
#include "strobe/flow/detail/op_stream.hpp"
#include "strobe/flow/program.hpp"

#include <strobe/core/containers/vector.hpp>
#include <strobe/core/graph.hpp>

#include <algorithm>
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
                                detail::OpStream &stream) {
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
  while (!ready.empty()) {
    const auto next = std::min_element(ready.begin(), ready.end());
    const auto node = *next;
    ready.erase(next);
    order.push_back(node);
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

  data.resource_types.reserve(values.resources.size());
  for (const auto &resource : values.resources) {
    data.resource_types.push_back(resource.type);
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
  for (const auto &phi : values.phis) {
    data.phis.push_back(CompiledPhi{
        .node = runtime_of[phi.node],
        .initial = phi.initial,
        .feedback = phi.feedback,
        .output = phi.output,
    });
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
    const auto input_offset = data.inputs.size();
    const auto output_offset = data.outputs.size();
    const auto access_offset = data.accesses.size();
    for (std::uint32_t index = 0; index < node.accesses.count; ++index) {
      const auto &access = values.accesses[node.accesses.offset + index];
      data.accesses.push_back(CompiledAccess{
          .resource = access.resource,
          .mode = access.mode,
          .input = access.input,
          .output = access.output,
      });
      if (access.input != null_value_id) {
        data.inputs.push_back(access.input);
      }
      if (access.output != null_value_id) {
        data.outputs.push_back(access.output);
      }
    }
    const auto immediate_predecessors =
        append_remapped(data.predecessors, predecessors[source]);
    const auto immediate_successors =
        append_remapped(data.successors, successors[source]);
    const auto prior_activation = append_remapped(
        data.temporal_predecessors, temporal_predecessors[source]);
    const auto next_activation =
        append_remapped(data.temporal_successors, temporal_successors[source]);
    CallbackId callback = null_callback_id;
    if (node.source_operation != null_source_operation_id) {
      callback = static_cast<CallbackId>(data.callbacks.size());
      const auto &recorded = stream.ops()[node.source_operation];
      CompiledCallback compiled_callback;
      compiled_callback.metadata = recorded.metadata;
      compiled_callback.external = recorded.external;
      compiled_callback.argument_types = recorded.argument_types;
      compiled_callback.argument_accesses = recorded.argument_accesses;
      compiled_callback.runtime =
          stream.release_callback(node.source_operation);
      data.callbacks.push_back(std::move(compiled_callback));
    }
    data.nodes.push_back(CompiledNode{
        .source = source,
        .kind = program_kind(node.kind),
        .callback = callback,
        .inputs = {.offset = static_cast<std::uint32_t>(input_offset),
                   .count = static_cast<std::uint32_t>(data.inputs.size() -
                                                       input_offset)},
        .outputs = {.offset = static_cast<std::uint32_t>(output_offset),
                    .count = static_cast<std::uint32_t>(data.outputs.size() -
                                                        output_offset)},
        .accesses = {.offset = static_cast<std::uint32_t>(access_offset),
                     .count = node.accesses.count},
        .predecessors = immediate_predecessors,
        .successors = immediate_successors,
        .temporal_predecessors = prior_activation,
        .temporal_successors = next_activation,
        .runs_once =
            activation.components[components.component_of[source]].runs_once,
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

  data.resource_slots.reserve(layout.resources.size());
  for (const auto &slot : layout.resources) {
    data.resource_slots.push_back(ResourceSlotPlan{
        .offset = slot.offset == no_storage_offset ? null_storage_offset
                                                   : slot.offset,
        .slot_stride = slot.slot_stride,
        .slot_count = slot.slot_count,
        .bounded_overlap = slot.bounded_overlap,
    });
  }
  data.resource_lifetimes.reserve(schedule.resources.size());
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
    data.resource_lifetimes.push_back(ResourceLifetimePlan{
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
    });
  }
  return data;
}

} // namespace strobe::flow::detail::compiler
