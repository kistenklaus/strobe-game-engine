#include <strobe/flow/detail/job_plan.hpp>

#include <fmt/format.h>

#include <algorithm>
#include <stdexcept>
#include <string>

namespace strobe::flow::detail {
namespace {

constexpr std::uint32_t max_nodes_per_job = 64;

bool is_boundary(const CompiledNode &node) {
  return node.kind == OperationKind::acquire ||
         node.kind == OperationKind::yield || node.kind == OperationKind::phi;
}

bool is_ancestor(const ProgramData &data, NodeId ancestor, NodeId node) {
  if (ancestor == node) {
    return true;
  }
  if (ancestor > node) {
    return false;
  }
  const auto &compiled = data.nodes[node];
  for (std::uint32_t index = 0; index < compiled.predecessors.count; ++index) {
    const auto predecessor =
        data.predecessors[compiled.predecessors.offset + index];
    if (is_ancestor(data, ancestor, predecessor)) {
      return true;
    }
  }
  return false;
}

bool can_append(const ProgramData &data, span<const std::uint32_t> regions,
                span<const bool> release_boundary, NodeId first,
                NodeId previous, NodeId node, std::uint32_t group_size) {
  if (group_size >= max_nodes_per_job || is_boundary(data.nodes[previous]) ||
      is_boundary(data.nodes[node]) || release_boundary[previous] ||
      data.nodes[previous].temporal_predecessors.count != 0 ||
      data.nodes[previous].temporal_successors.count != 0 ||
      data.nodes[node].temporal_predecessors.count != 0 ||
      data.nodes[node].temporal_successors.count != 0) {
    return false;
  }
  const auto previous_control = previous < data.control_of_node.size()
                                    ? data.control_of_node[previous]
                                    : null_control_id;
  const auto node_control = node < data.control_of_node.size()
                                ? data.control_of_node[node]
                                : null_control_id;
  if (previous_control != null_control_id || node_control != null_control_id ||
      regions[previous] != regions[node]) {
    return false;
  }

  bool depends_on_previous = false;
  const auto &compiled = data.nodes[node];
  for (std::uint32_t index = 0; index < compiled.predecessors.count; ++index) {
    const auto predecessor =
        data.predecessors[compiled.predecessors.offset + index];
    depends_on_previous |= predecessor == previous;
    if (predecessor >= first && predecessor <= previous) {
      continue;
    }
    if (!is_ancestor(data, predecessor, first)) {
      return false;
    }
  }
  return depends_on_previous;
}

template <typename T>
void append_unique(Vector<T, allocator_ref> &values, T value) {
  if (std::find(values.begin(), values.end(), value) == values.end()) {
    values.push_back(value);
  }
}

} // namespace

JobPlan build_job_plan(const ProgramData &data) {
  JobPlan plan;
  plan.job_of_node.resize(data.nodes.size(), null_job_id);

  for (NodeId node = 0; node < data.nodes.size(); ++node) {
    const auto &compiled = data.nodes[node];
    if (compiled.predecessors.offset + compiled.predecessors.count >
        data.predecessors.size()) {
      throw std::logic_error("compiled node predecessors are out of range");
    }
    for (std::uint32_t index = 0; index < compiled.predecessors.count;
         ++index) {
      const auto predecessor =
          data.predecessors[compiled.predecessors.offset + index];
      if (predecessor >= node) {
        throw std::logic_error(
            "compiled nodes are not stored in dependency order");
      }
    }
  }

  Vector<std::uint32_t, allocator_ref> region_of(
      data.nodes.size(), std::numeric_limits<std::uint32_t>::max(),
      allocator_ref{});
  for (std::uint32_t region = 0; region < data.feedback_regions.size();
       ++region) {
    const auto range = data.feedback_regions[region].nodes;
    if (range.offset + range.count > data.feedback_region_nodes.size()) {
      throw std::logic_error("feedback region nodes are out of range");
    }
    for (std::uint32_t index = 0; index < range.count; ++index) {
      const auto node = data.feedback_region_nodes[range.offset + index];
      if (node >= data.nodes.size() ||
          region_of[node] != std::numeric_limits<std::uint32_t>::max()) {
        throw std::logic_error("feedback region node membership is invalid");
      }
      region_of[node] = region;
    }
  }
  Vector<bool, allocator_ref> release_boundary(data.nodes.size(), false,
                                               allocator_ref{});
  for (const auto &lifetime : data.resource_lifetimes) {
    const bool release_preconstructed =
        lifetime.preconstructed && !lifetime.retained_by_feedback;
    if ((lifetime.destroy_at_shutdown && !release_preconstructed) ||
        lifetime.release_after.count == 0 ||
        lifetime.release_after.offset + lifetime.release_after.count >
            data.release_after.size()) {
      continue;
    }
    NodeId last = 0;
    for (std::uint32_t index = 0; index < lifetime.release_after.count;
         ++index) {
      last = std::max(
          last, data.release_after[lifetime.release_after.offset + index]);
    }
    if (last >= data.nodes.size()) {
      throw std::logic_error("resource release frontier is out of range");
    }
    release_boundary[last] = true;
  }

  for (NodeId first = 0; first < data.nodes.size();) {
    const auto job = static_cast<JobId>(plan.jobs.size());
    const auto node_offset = static_cast<std::uint32_t>(plan.nodes.size());
    plan.nodes.push_back(first);
    plan.job_of_node[first] = job;
    auto previous = first;
    auto next = static_cast<NodeId>(first + 1);
    while (next < data.nodes.size() &&
           can_append(data, region_of, release_boundary, first, previous, next,
                      static_cast<std::uint32_t>(plan.nodes.size()) -
                          node_offset)) {
      plan.nodes.push_back(next);
      plan.job_of_node[next] = job;
      previous = next;
      ++next;
    }
    plan.jobs.push_back(CompiledJob{
        .nodes = {.offset = node_offset,
                  .count = static_cast<std::uint32_t>(plan.nodes.size()) -
                           node_offset},
        .predecessors = {},
        .successors = {},
    });
    first = next;
  }

  Vector<Vector<JobId, allocator_ref>, allocator_ref> predecessors(
      plan.jobs.size(), allocator_ref{});
  Vector<Vector<JobId, allocator_ref>, allocator_ref> successors(
      plan.jobs.size(), allocator_ref{});
  for (NodeId node = 0; node < data.nodes.size(); ++node) {
    const auto destination = plan.job_of_node[node];
    const auto &compiled = data.nodes[node];
    for (std::uint32_t index = 0; index < compiled.predecessors.count;
         ++index) {
      const auto predecessor =
          data.predecessors[compiled.predecessors.offset + index];
      if (predecessor >= data.nodes.size()) {
        throw std::logic_error("compiled node predecessor is out of range");
      }
      const auto source = plan.job_of_node[predecessor];
      if (source == destination) {
        continue;
      }
      append_unique(predecessors[destination], source);
      append_unique(successors[source], destination);
    }
  }
  for (JobId job = 0; job < plan.jobs.size(); ++job) {
    auto &compiled = plan.jobs[job];
    compiled.predecessors.offset =
        static_cast<std::uint32_t>(plan.predecessors.size());
    compiled.predecessors.count =
        static_cast<std::uint32_t>(predecessors[job].size());
    for (const auto predecessor : predecessors[job]) {
      plan.predecessors.push_back(predecessor);
    }
    compiled.successors.offset =
        static_cast<std::uint32_t>(plan.successors.size());
    compiled.successors.count =
        static_cast<std::uint32_t>(successors[job].size());
    for (const auto successor : successors[job]) {
      plan.successors.push_back(successor);
    }
  }
  return plan;
}

String<allocator_ref> to_pretty(const JobPlan &plan, const ProgramData &data) {
  std::string result;
  for (JobId job = 0; job < plan.jobs.size(); ++job) {
    const auto &compiled = plan.jobs[job];
    fmt::format_to(std::back_inserter(result), "job %{} deps={{", job);
    for (std::uint32_t index = 0; index < compiled.predecessors.count;
         ++index) {
      if (index != 0) {
        result += ',';
      }
      fmt::format_to(std::back_inserter(result), "%{}",
                     plan.predecessors[compiled.predecessors.offset + index]);
    }
    result += "} {\n";
    for (std::uint32_t index = 0; index < compiled.nodes.count; ++index) {
      const auto node = plan.nodes[compiled.nodes.offset + index];
      fmt::format_to(std::back_inserter(result), "  @{}", node);
      const auto callback = data.nodes[node].callback;
      if (callback != null_callback_id && callback < data.callbacks.size() &&
          !data.callbacks[callback].metadata.title.empty()) {
        fmt::format_to(std::back_inserter(result), " {}",
                       data.callbacks[callback].metadata.title.view());
      }
      result += '\n';
    }
    result += '}';
    if (job + 1 != plan.jobs.size()) {
      result += '\n';
    }
  }
  return String<allocator_ref>(result, allocator_ref{});
}

} // namespace strobe::flow::detail
