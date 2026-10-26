#include <strobe/flow/detail/program_data.hpp>

#include <fmt/format.h>

#include <algorithm>
#include <iterator>
#include <limits>
#include <string>
#include <string_view>

namespace strobe::flow::detail {
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
  std::size_t first_access = node.accesses.offset;
  const auto access_end = first_access + node.accesses.count;

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
    if (first_access < data.accesses.size() && first_access < access_end) {
      fmt::format_to(out, "discard %{}", data.accesses[first_access].resource);
    } else {
      fmt::format_to(out, "discard <invalid>");
    }
    return;
  }
  if (node.kind == OperationKind::construct) {
    if (first_access < data.accesses.size() && first_access < access_end) {
      append_value(out, data, data.accesses[first_access].output);
      fmt::format_to(out, " = construct");
      ++first_access;
    } else {
      fmt::format_to(out, "construct <invalid>");
    }
  } else {
    fmt::format_to(out, "{}", node.kind);
  }

  for (std::size_t index = first_access; index < access_end; ++index) {
    fmt::format_to(out, "{}", index == first_access ? " " : ", ");
    if (index < data.accesses.size()) {
      append_access(prefix, data, data.accesses[index]);
    } else {
      fmt::format_to(out, "<invalid-access>");
    }
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
      if (external.resource < data.resource_types.size()) {
        fmt::format_to(out(), " : {}", data.resource_types[external.resource]);
      }
      fmt::format_to(out(), "\n");
    }
  }

  fmt::format_to(out(), "\nresources:\n");
  for (ResourceId resource = 0; resource < data.resource_types.size();
       ++resource) {
    fmt::format_to(out(), "  %{} : {}", resource,
                   data.resource_types[resource]);
    if (resource < data.resource_slots.size()) {
      const auto &slot = data.resource_slots[resource];
      if (slot.offset == null_storage_offset) {
        fmt::format_to(out(), " storage=none");
      } else {
        fmt::format_to(out(), " storage=[offset={}, stride={}, slots={}]",
                       slot.offset, slot.slot_stride, slot.slot_count);
        if (slot.bounded_overlap) {
          fmt::format_to(out(), " bounded-overlap");
        }
      }
    }
    if (resource < data.resource_lifetimes.size()) {
      const auto &lifetime = data.resource_lifetimes[resource];
      fmt::format_to(out(), " lifetime=[{},{})", lifetime.interval.begin,
                     lifetime.interval.end);
      if (lifetime.repeating) {
        fmt::format_to(out(), " repeating");
      }
      if (lifetime.preconstructed) {
        fmt::format_to(out(), " preconstructed");
      }
      if (lifetime.retained_by_feedback) {
        fmt::format_to(out(), " retained-by-feedback");
      }
      if (lifetime.destroy_at_shutdown) {
        fmt::format_to(out(), " destroy-at-shutdown");
      }
      if (lifetime.constructions.count != 0) {
        fmt::format_to(out(), " construct=");
        append_nodes(out(), data.constructions, lifetime.constructions);
      }
      if (lifetime.release_after.count != 0) {
        fmt::format_to(out(), " release-after=");
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
    if (node.runs_once || node.memoized) {
      fmt::format_to(out(), " [");
      if (node.runs_once) {
        fmt::format_to(out(), "once");
      }
      if (node.runs_once && node.memoized) {
        fmt::format_to(out(), ", ");
      }
      if (node.memoized) {
        fmt::format_to(out(), "memoized");
      }
      fmt::format_to(out(), "]");
    }
    fmt::format_to(out(), " deps=");
    append_nodes(out(), data.predecessors, node.predecessors);
    if (node.temporal_predecessors.count != 0) {
      fmt::format_to(out(), " temporal=");
      append_nodes(out(), data.temporal_predecessors,
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
      append_nodes(out(), data.feedback_region_nodes, region.nodes);
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
