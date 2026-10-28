#include <strobe/flow/detail/program_data.hpp>

#include <fmt/format.h>

#include <algorithm>
#include <cassert>
#include <iterator>
#include <limits>
#include <string>
#include <string_view>

namespace strobe::flow::detail {
namespace {

template <typename T>
span<T> slice(Vector<T, allocator_ref> &storage, IndexRange range) noexcept {
  assert(range.offset <= storage.size() &&
         range.count <= storage.size() - range.offset);
  return span<T>(storage.data(), storage.size())
      .subspan(range.offset, range.count);
}

template <typename T>
span<const T> slice(const Vector<T, allocator_ref> &storage,
                    IndexRange range) noexcept {
  assert(range.offset <= storage.size() &&
         range.count <= storage.size() - range.offset);
  return span<const T>(storage.data(), storage.size())
      .subspan(range.offset, range.count);
}

} // namespace

#define STROBE_PROGRAM_NODE_VIEW(Name, Storage, Member, Type)                  \
  span<Type> ProgramData::Name(CompiledNode &node) noexcept {                  \
    return slice(Storage, node.Member);                                        \
  }                                                                            \
  span<const Type> ProgramData::Name(const CompiledNode &node)                 \
      const noexcept {                                                         \
    return slice(Storage, node.Member);                                        \
  }

STROBE_PROGRAM_NODE_VIEW(accesses_of, node_storage.accesses, accesses,
                         CompiledAccess)
STROBE_PROGRAM_NODE_VIEW(inputs_of, node_storage.inputs, inputs, ValueId)
STROBE_PROGRAM_NODE_VIEW(outputs_of, node_storage.outputs, outputs, ValueId)
STROBE_PROGRAM_NODE_VIEW(predecessors_of, node_storage.predecessors,
                         predecessors, NodeId)
STROBE_PROGRAM_NODE_VIEW(successors_of, node_storage.successors, successors,
                         NodeId)
STROBE_PROGRAM_NODE_VIEW(temporal_predecessors_of,
                         node_storage.temporal_predecessors,
                         temporal_predecessors, NodeId)
STROBE_PROGRAM_NODE_VIEW(temporal_successors_of,
                         node_storage.temporal_successors, temporal_successors,
                         NodeId)

#undef STROBE_PROGRAM_NODE_VIEW

span<CompiledAccess> ProgramData::accesses_of(NodeId node) noexcept {
  assert(node < nodes.size());
  return accesses_of(nodes[node]);
}

span<const CompiledAccess>
ProgramData::accesses_of(NodeId node) const noexcept {
  assert(node < nodes.size());
  return accesses_of(nodes[node]);
}

#define STROBE_PROGRAM_VIEW(Name, Descriptor, Storage, Member, Type)           \
  span<Type> ProgramData::Name(Descriptor &descriptor) noexcept {              \
    return slice(Storage, descriptor.Member);                                  \
  }                                                                            \
  span<const Type> ProgramData::Name(const Descriptor &descriptor)             \
      const noexcept {                                                         \
    return slice(Storage, descriptor.Member);                                  \
  }

STROBE_PROGRAM_VIEW(phis_of, CompiledFeedbackRegion, feedback_region_phis, phis,
                    std::uint32_t)
STROBE_PROGRAM_VIEW(nodes_of, CompiledFeedbackRegion, feedback_region_nodes,
                    nodes, NodeId)
STROBE_PROGRAM_VIEW(acquires_of, CompiledFeedbackRegion,
                    feedback_region_acquires, acquires, std::uint32_t)
STROBE_PROGRAM_VIEW(token_requirements_of, CompiledFeedbackRegion,
                    feedback_token_requirements, token_requirements,
                    CompiledTokenRequirement)
STROBE_PROGRAM_VIEW(primary_steps_of, CompiledControl, control_steps,
                    primary_steps, CompiledControlStep)
STROBE_PROGRAM_VIEW(secondary_steps_of, CompiledControl, control_steps,
                    secondary_steps, CompiledControlStep)
STROBE_PROGRAM_VIEW(nodes_of, CompiledControl, control_nodes, nodes, NodeId)
STROBE_PROGRAM_VIEW(resources_of, CompiledControl, control_resources, resources,
                    ResourceId)
STROBE_PROGRAM_VIEW(primary_written_resources_of, CompiledControl,
                    control_written_resources, primary_written_resources,
                    ResourceId)
STROBE_PROGRAM_VIEW(secondary_written_resources_of, CompiledControl,
                    control_written_resources, secondary_written_resources,
                    ResourceId)
STROBE_PROGRAM_VIEW(argument_groups_of, CompiledCallback, argument_groups,
                    argument_groups, ArgumentGroup)
STROBE_PROGRAM_VIEW(argument_elements_of, CompiledCallback, argument_elements,
                    argument_elements, std::uint32_t)
STROBE_PROGRAM_VIEW(immediates_of, CompiledCallback, immediates, immediates,
                    CompiledImmediate)

#undef STROBE_PROGRAM_VIEW

span<NodeId>
ProgramData::constructions_of(CompiledResource &resource) noexcept {
  return slice(constructions, resource.lifetime.constructions);
}

span<const NodeId>
ProgramData::constructions_of(const CompiledResource &resource) const noexcept {
  return slice(constructions, resource.lifetime.constructions);
}

span<NodeId>
ProgramData::release_after_of(CompiledResource &resource) noexcept {
  return slice(release_after, resource.lifetime.release_after);
}

span<const NodeId>
ProgramData::release_after_of(const CompiledResource &resource) const noexcept {
  return slice(release_after, resource.lifetime.release_after);
}

namespace {

void append_value(auto out, const ProgramData &data, ValueId value) {
  if (value == null_value_id) {
    fmt::format_to(out, "_");
    return;
  }
  if (value >= data.values.size()) {
    fmt::format_to(out, "<invalid-value:{}>", value);
    return;
  }
  const auto &compiled = data.values[value];
  fmt::format_to(out, "%{}.{}", compiled.resource, compiled.version);
}

template <typename T, typename Format>
void append_range(auto out, const Vector<T, allocator_ref> &values,
                  IndexRange range, Format format) {
  fmt::format_to(out, "{{");
  for (std::uint32_t index = 0; index < range.count; ++index) {
    if (index != 0) {
      fmt::format_to(out, ",");
    }
    const auto flat = static_cast<std::size_t>(range.offset) + index;
    if (flat >= values.size()) {
      fmt::format_to(out, "<invalid>");
      continue;
    }
    format(out, values[flat]);
  }
  fmt::format_to(out, "}}");
}

void append_nodes(auto out, const Vector<NodeId, allocator_ref> &nodes,
                  IndexRange range) {
  append_range(out, nodes, range, [](auto target, NodeId node) {
    fmt::format_to(target, "@{}", node);
  });
}

void append_resources(auto out,
                      const Vector<ResourceId, allocator_ref> &resources,
                      IndexRange range) {
  append_range(out, resources, range, [](auto target, ResourceId resource) {
    fmt::format_to(target, "%{}", resource);
  });
}

void append_access(std::string &prefix, const ProgramData &data,
                   const CompiledAccess &access) {
  auto out = std::back_inserter(prefix);
  if (access.purpose == AccessPurpose::serialization) {
    fmt::format_to(out, "s:%{}", access.resource);
    return;
  }
  if (access.mode == AccessMode::read || access.mode == AccessMode::consume) {
    fmt::format_to(out, "{}:", access.mode);
    append_value(out, data, access.input);
    return;
  }
  fmt::format_to(out, "{}:", access.mode);
  append_value(out, data, access.input);
  fmt::format_to(out, "->");
  append_value(out, data, access.output);
}

const CompiledPhi *phi_for(const ProgramData &data, NodeId node) {
  const auto result =
      std::find_if(data.phis.begin(), data.phis.end(),
                   [&](const CompiledPhi &phi) { return phi.node == node; });
  return result == data.phis.end() ? nullptr : &*result;
}

const CompiledAcquire *acquire_for(const ProgramData &data, NodeId node) {
  const auto result = std::find_if(
      data.acquires.begin(), data.acquires.end(),
      [&](const CompiledAcquire &acquire) { return acquire.node == node; });
  return result == data.acquires.end() ? nullptr : &*result;
}

const CompiledCutoff *cutoff_for(const ProgramData &data, NodeId node) {
  const auto result = std::find_if(
      data.cutoffs.begin(), data.cutoffs.end(),
      [&](const CompiledCutoff &cutoff) { return cutoff.node == node; });
  return result == data.cutoffs.end() ? nullptr : &*result;
}

void append_operation(std::string &prefix, const ProgramData &data,
                      NodeId node_id, const CompiledNode &node) {
  auto out = std::back_inserter(prefix);
  const auto accesses = data.accesses_of(node);
  std::size_t first_access = 0;

  if (node.kind == OperationKind::phi) {
    if (const auto *phi = phi_for(data, node_id)) {
      append_value(out, data, phi->output);
      fmt::format_to(out, " = phi ");
      append_value(out, data, phi->initial);
      fmt::format_to(out, " feedback ");
      append_value(out, data, phi->feedback);
    } else {
      fmt::format_to(out, "phi <invalid>");
    }
    return;
  }
  if (node.kind == OperationKind::acquire) {
    const auto *acquire = acquire_for(data, node_id);
    if (acquire != nullptr &&
        acquire->semaphore < data.semaphore_names.size()) {
      fmt::format_to(out, "acquire \"{}\"",
                     data.semaphore_names[acquire->semaphore]);
    } else {
      fmt::format_to(out, "acquire <invalid>");
    }
    return;
  }
  if (node.kind == OperationKind::cutoff) {
    if (const auto *cutoff = cutoff_for(data, node_id)) {
      append_value(out, data, cutoff->output);
      fmt::format_to(out, " = cutoff ");
      append_value(out, data, cutoff->input);
    } else {
      fmt::format_to(out, "cutoff <invalid>");
    }
    return;
  }
  if (node.kind == OperationKind::discard) {
    if (!accesses.empty()) {
      fmt::format_to(out, "discard %{}", accesses.front().resource);
    } else {
      fmt::format_to(out, "discard <invalid>");
    }
    return;
  }
  if (node.kind == OperationKind::construct) {
    if (!accesses.empty()) {
      append_value(out, data, accesses.front().output);
      fmt::format_to(out, " = construct");
      ++first_access;
    } else {
      fmt::format_to(out, "construct <invalid>");
    }
  } else {
    fmt::format_to(out, "{}", node.kind);
  }

  for (std::size_t index = first_access; index < accesses.size(); ++index) {
    fmt::format_to(out, "{}", index == first_access ? " " : ", ");
    append_access(prefix, data, accesses[index]);
  }
}

void append_callback(auto out, const ProgramData &data,
                     const CompiledNode &node) {
  if (node.callback == null_callback_id) {
    return;
  }
  if (node.callback >= data.callbacks.size()) {
    fmt::format_to(out, " (<invalid-callback:{}>)", node.callback);
    return;
  }
  const auto &callback = data.callbacks[node.callback];
  if (callback.external) {
    fmt::format_to(out, " ({}::{})", callback.external->library_id,
                   callback.external->symbol);
  } else {
    fmt::format_to(out, " (<lambda>)");
  }
  if (!callback.metadata.title.empty() ||
      callback.metadata.semantics != OpSemantics::none) {
    fmt::format_to(out, " {}", callback.metadata);
  }
}

std::string_view control_name(ControlKind kind) {
  switch (kind) {
  case ControlKind::loop:
    return "while";
  case ControlKind::branch:
    return "if";
  }
  return "unknown";
}

void append_control_steps(auto out, const ProgramData &data, IndexRange range) {
  append_range(out, data.control_steps, range,
               [](auto target, const CompiledControlStep &step) {
                 if (step.kind == ControlStepKind::node) {
                   fmt::format_to(target, "@{}", step.index);
                 } else {
                   fmt::format_to(target, "control#{}", step.index);
                 }
               });
}

} // namespace

String<allocator_ref> to_pretty(const ProgramData &data) {
  String<allocator_ref> result(allocator_ref{});
  const auto out = [&] { return std::back_inserter(result); };

  fmt::format_to(out(), "storage: size={}, alignment={}\n", data.storage_size,
                 data.storage_alignment);
  fmt::format_to(out(),
                 "activation: capacity={}, stride={}, size={}, alignment={}\n",
                 data.activation_layout.capacity, data.activation_layout.stride,
                 data.activation_layout.size, data.activation_layout.alignment);

  if (!data.externals.empty()) {
    fmt::format_to(out(), "\nexternals:\n");
    for (const auto &external : data.externals) {
      fmt::format_to(out(), "  %{} = external \"{}\"", external.resource,
                     external.name);
      if (external.resource < data.resources.size()) {
        fmt::format_to(out(), " : {}",
                       data.resources[external.resource].type_name);
      }
      fmt::format_to(out(), "\n");
    }
  }

  fmt::format_to(out(), "\nresources:\n");
  for (ResourceId resource = 0; resource < data.resources.size(); ++resource) {
    fmt::format_to(out(), "  %{}", resource);
    if (resource < data.resources.size() &&
        !data.resources[resource].label.empty()) {
      fmt::format_to(out(), " \"{}\"", data.resources[resource].label);
    }
    if (resource < data.resources.size()) {
      fmt::format_to(out(), " : {}", data.resources[resource].type_name);
    } else {
      fmt::format_to(out(), " : {}", data.resources[resource].type);
    }
    if (resource < data.resources.size()) {
      const auto &slot = data.resources[resource].slots;
      if (slot.offset != null_storage_offset) {
        fmt::format_to(out(), " storage=[offset={}, stride={}, slots={}]",
                       slot.offset, slot.slot_stride, slot.slot_count);
      }
    }
    if (resource < data.resources.size()) {
      const auto &lifetime = data.resources[resource].lifetime;
      fmt::format_to(out(), " lifetime=[{},", lifetime.interval.begin);
      if (lifetime.interval.end == std::numeric_limits<std::uint32_t>::max()) {
        fmt::format_to(out(), "inf)");
      } else {
        fmt::format_to(out(), "{})", lifetime.interval.end);
      }
      if (lifetime.repeating) {
        fmt::format_to(out(), " repeat");
      }
      if (lifetime.preconstructed) {
        fmt::format_to(out(), " preconstructed");
      }
      if (lifetime.retained_by_feedback) {
        fmt::format_to(out(), " temporal");
      }
      if (lifetime.constructions.count != 0) {
        fmt::format_to(out(), " modified=");
        append_nodes(out(), data.constructions, lifetime.constructions);
      }
      if (!lifetime.destroy_at_shutdown && lifetime.release_after.count != 0) {
        fmt::format_to(out(), " last-use=");
        append_nodes(out(), data.release_after, lifetime.release_after);
      }
    }
    fmt::format_to(out(), "\n");
  }

  fmt::format_to(out(), "\nnodes:\n");
  for (NodeId node_id = 0; node_id < data.nodes.size(); ++node_id) {
    const auto &node = data.nodes[node_id];
    std::string prefix = fmt::format("@{} ", node_id);
    append_operation(prefix, data, node_id, node);
    fmt::format_to(out(), "  {:<50}", prefix);
    append_callback(out(), data, node);
    if (node.initialization || node.memoized) {
      fmt::format_to(out(), " [");
      if (node.initialization) {
        fmt::format_to(out(), "init");
      }
      if (node.initialization && node.memoized) {
        fmt::format_to(out(), ", ");
      }
      if (node.memoized) {
        fmt::format_to(out(), "memoized");
      }
      fmt::format_to(out(), "]");
    }
    fmt::format_to(out(), " deps=");
    append_nodes(out(), data.node_storage.predecessors, node.predecessors);
    if (node.temporal_predecessors.count != 0) {
      fmt::format_to(out(), " temporal=");
      append_nodes(out(), data.node_storage.temporal_predecessors,
                   node.temporal_predecessors);
    }
    fmt::format_to(out(), "\n");
  }

  if (!data.feedback_regions.empty()) {
    fmt::format_to(out(), "\nfeedback regions:\n");
    for (std::uint32_t region_id = 0; region_id < data.feedback_regions.size();
         ++region_id) {
      const auto &region = data.feedback_regions[region_id];
      fmt::format_to(out(), "  loop#{} source=[{},{}) max_in_flight={} nodes=",
                     region_id, region.declaration_position,
                     region.feedback_position, region.max_in_flight);
      bool contiguous = region.nodes.count != 0;
      NodeId first = 0;
      for (std::uint32_t index = 0; index < region.nodes.count; ++index) {
        const auto flat = static_cast<std::size_t>(region.nodes.offset) + index;
        if (flat >= data.feedback_region_nodes.size()) {
          contiguous = false;
          break;
        }
        const auto node = data.feedback_region_nodes[flat];
        if (index == 0) {
          first = node;
        } else if (node != first + index) {
          contiguous = false;
          break;
        }
      }
      if (contiguous) {
        fmt::format_to(out(), "[@{},@{})", first, first + region.nodes.count);
      } else {
        append_nodes(out(), data.feedback_region_nodes, region.nodes);
      }
      fmt::format_to(out(), " phis=");
      append_range(out(), data.feedback_region_phis, region.phis,
                   [](auto target, std::uint32_t phi) {
                     fmt::format_to(target, "phi#{}", phi);
                   });
      fmt::format_to(out(), " acquires={{");
      for (std::uint32_t index = 0; index < region.acquires.count; ++index) {
        if (index != 0) {
          fmt::format_to(out(), ",");
        }
        const auto flat =
            static_cast<std::size_t>(region.acquires.offset) + index;
        if (flat >= data.feedback_region_acquires.size() ||
            data.feedback_region_acquires[flat] >= data.acquires.size()) {
          fmt::format_to(out(), "<invalid>");
          continue;
        }
        const auto &acquire =
            data.acquires[data.feedback_region_acquires[flat]];
        if (acquire.semaphore < data.semaphore_names.size()) {
          fmt::format_to(out(), "\"{}\"",
                         data.semaphore_names[acquire.semaphore]);
        } else {
          fmt::format_to(out(), "<invalid>");
        }
      }
      fmt::format_to(out(), "}}\n");
    }
  }

  if (!data.controls.empty()) {
    fmt::format_to(out(), "\ncontrols:\n");
    for (ControlId control_id = 0; control_id < data.controls.size();
         ++control_id) {
      const auto &control = data.controls[control_id];
      fmt::format_to(out(), "  control#{} {} parent=", control_id,
                     control_name(control.kind));
      if (control.parent == null_control_id) {
        fmt::format_to(out(), "none");
      } else {
        fmt::format_to(out(), "control#{}", control.parent);
      }
      fmt::format_to(out(), " condition=");
      append_value(out(), data, control.initial_condition);
      if (control.repeat_condition != null_value_id) {
        fmt::format_to(out(), " repeat=");
        append_value(out(), data, control.repeat_condition);
      }
      fmt::format_to(out(), " primary=");
      append_control_steps(out(), data, control.primary_steps);
      if (control.kind == ControlKind::branch) {
        fmt::format_to(out(), " secondary=");
        append_control_steps(out(), data, control.secondary_steps);
      }
      fmt::format_to(out(), " resources=");
      append_resources(out(), data.control_resources, control.resources);
      fmt::format_to(out(), " primary-writes=");
      append_resources(out(), data.control_written_resources,
                       control.primary_written_resources);
      if (control.kind == ControlKind::branch) {
        fmt::format_to(out(), " secondary-writes=");
        append_resources(out(), data.control_written_resources,
                         control.secondary_written_resources);
      }
      fmt::format_to(out(), "\n");
    }
  }

  if (!result.empty() && result.back() == '\n') {
    result.pop_back();
  }
  return result;
}

} // namespace strobe::flow::detail
