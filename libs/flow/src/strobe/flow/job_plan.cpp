#include <strobe/flow/detail/job_plan.hpp>

#include <fmt/format.h>

#include <algorithm>
#include <cassert>
#include <stdexcept>
#include <string>

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

#define STROBE_JOB_PLAN_VIEW(Name, Storage, Member, Type)                      \
  span<Type> JobPlan::Name(CompiledJob &job) noexcept {                        \
    return slice(storage.Storage, job.Member);                                 \
  }                                                                            \
  span<const Type> JobPlan::Name(const CompiledJob &job) const noexcept {      \
    return slice(storage.Storage, job.Member);                                 \
  }                                                                            \
  span<Type> JobPlan::Name(JobId job) noexcept {                               \
    assert(job < jobs.size());                                                 \
    return Name(jobs[job]);                                                    \
  }                                                                            \
  span<const Type> JobPlan::Name(JobId job) const noexcept {                   \
    assert(job < jobs.size());                                                 \
    return Name(jobs[job]);                                                    \
  }

STROBE_JOB_PLAN_VIEW(nodes_of, nodes, nodes, NodeId)
STROBE_JOB_PLAN_VIEW(predecessors_of, predecessors, predecessors, JobId)
STROBE_JOB_PLAN_VIEW(successors_of, successors, successors, JobId)
STROBE_JOB_PLAN_VIEW(resource_frontiers_of, resource_frontiers,
                     resource_frontiers, ResourceId)
STROBE_JOB_PLAN_VIEW(released_resources_of, released_resources,
                     released_resources, ResourceId)

#undef STROBE_JOB_PLAN_VIEW

namespace {

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
  for (const auto predecessor : data.predecessors_of(compiled)) {
    if (is_ancestor(data, ancestor, predecessor)) {
      return true;
    }
  }
  return false;
}

bool can_append(const ProgramData &data, span<const std::uint32_t> regions,
                span<const bool> release_boundary, span<const bool> members,
                NodeId first, NodeId previous, NodeId node) {
  if (is_boundary(data.nodes[previous]) || is_boundary(data.nodes[node]) ||
      release_boundary[previous] ||
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
  for (const auto predecessor : data.predecessors_of(compiled)) {
    depends_on_previous |= predecessor == previous;
    if (predecessor < members.size() && members[predecessor]) {
      continue;
    }
    if (!is_ancestor(data, predecessor, first)) {
      return false;
    }
  }
  return depends_on_previous;
}

template <typename Id>
using Adjacency = Vector<Vector<Id, allocator_ref>, allocator_ref>;

template <typename Id>
Adjacency<Id> transitive_reduction(const Adjacency<Id> &outgoing) {
  Adjacency<Id> result(outgoing.size(), allocator_ref{});
  for (Id source = 0; source < outgoing.size(); ++source) {
    const auto &successors = outgoing[source];
    for (std::size_t candidate_index = 0; candidate_index < successors.size();
         ++candidate_index) {
      const auto candidate = successors[candidate_index];
      Vector<bool, allocator_ref> visited(outgoing.size(), false,
                                          allocator_ref{});
      Vector<Id, allocator_ref> pending(allocator_ref{});
      for (std::size_t index = 0; index < successors.size(); ++index) {
        if (index != candidate_index) {
          pending.push_back(successors[index]);
        }
      }
      bool redundant = false;
      while (!pending.empty()) {
        const auto node = pending.back();
        pending.pop_back();
        if (node >= outgoing.size()) {
          throw std::logic_error("dependency successor is out of range");
        }
        if (node == candidate) {
          redundant = true;
          break;
        }
        if (visited[node]) {
          continue;
        }
        visited[node] = true;
        for (const auto successor : outgoing[node]) {
          pending.push_back(successor);
        }
      }
      if (!redundant) {
        result[source].push_back(candidate);
      }
    }
  }
  return result;
}

template <typename Id>
Adjacency<Id> invert_adjacency(const Adjacency<Id> &outgoing) {
  Adjacency<Id> incoming(outgoing.size(), allocator_ref{});
  for (Id source = 0; source < outgoing.size(); ++source) {
    for (const auto destination : outgoing[source]) {
      if (destination >= incoming.size()) {
        throw std::logic_error("dependency successor is out of range");
      }
      incoming[destination].push_back(source);
    }
  }
  return incoming;
}

template <typename T>
void append_unique(Vector<T, allocator_ref> &values, T value) {
  if (std::find(values.begin(), values.end(), value) == values.end()) {
    values.push_back(value);
  }
}

} // namespace

namespace compiler {

JobPlan build_job_plan(const ProgramData &data) {
  JobPlan plan;
  plan.job_of_node.resize(data.nodes.size(), null_job_id);

  for (NodeId node = 0; node < data.nodes.size(); ++node) {
    const auto &compiled = data.nodes[node];
    if (compiled.predecessors.offset + compiled.predecessors.count >
        data.node_storage.predecessors.size()) {
      throw std::logic_error("compiled node predecessors are out of range");
    }
    for (const auto predecessor : data.predecessors_of(compiled)) {
      if (predecessor >= node) {
        throw std::logic_error(
            "compiled nodes are not stored in dependency order");
      }
    }
  }

  Adjacency<NodeId> node_successors(data.nodes.size(), allocator_ref{});
  for (NodeId node = 0; node < data.nodes.size(); ++node) {
    const auto &compiled = data.nodes[node];
    for (const auto predecessor : data.predecessors_of(compiled)) {
      append_unique(node_successors[predecessor], node);
    }
  }
  const auto reduced_node_successors = transitive_reduction(node_successors);
  const auto reduced_node_predecessors =
      invert_adjacency(reduced_node_successors);

  Vector<std::uint32_t, allocator_ref> region_of(
      data.nodes.size(), std::numeric_limits<std::uint32_t>::max(),
      allocator_ref{});
  constexpr auto initialization_region =
      std::numeric_limits<std::uint32_t>::max() - 1;
  for (NodeId node = 0; node < data.nodes.size(); ++node) {
    if (data.nodes[node].initialization) {
      region_of[node] = initialization_region;
    }
  }
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
  for (const auto &resource : data.resources) {
    const auto &lifetime = resource.lifetime;
    const bool release_preconstructed =
        lifetime.preconstructed && !lifetime.retained_by_feedback;
    if (lifetime.destroy == nullptr ||
        (lifetime.destroy_at_shutdown && !release_preconstructed) ||
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

  Vector<bool, allocator_ref> assigned(data.nodes.size(), false,
                                       allocator_ref{});
  for (NodeId first = 0; first < data.nodes.size(); ++first) {
    if (assigned[first]) {
      continue;
    }
    const auto job = static_cast<JobId>(plan.jobs.size());
    const auto node_offset =
        static_cast<std::uint32_t>(plan.storage.nodes.size());
    plan.storage.nodes.push_back(first);
    plan.job_of_node[first] = job;
    assigned[first] = true;
    auto previous = first;
    while (reduced_node_successors[previous].size() == 1) {
      const auto next = reduced_node_successors[previous][0];
      if (next <= previous || assigned[next] ||
          reduced_node_predecessors[next].size() != 1 ||
          !can_append(data, region_of, release_boundary, assigned, first,
                      previous, next)) {
        break;
      }
      plan.storage.nodes.push_back(next);
      plan.job_of_node[next] = job;
      assigned[next] = true;
      previous = next;
    }
    plan.jobs.push_back(CompiledJob{
        .nodes = {.offset = node_offset,
                  .count =
                      static_cast<std::uint32_t>(plan.storage.nodes.size()) -
                      node_offset},
        .predecessors = {},
        .successors = {},
        .resource_frontiers = {},
        .released_resources = {},
    });
  }

  Adjacency<JobId> job_successors(plan.jobs.size(), allocator_ref{});
  for (NodeId node = 0; node < data.nodes.size(); ++node) {
    const auto destination = plan.job_of_node[node];
    const auto &compiled = data.nodes[node];
    for (const auto predecessor : data.predecessors_of(compiled)) {
      if (predecessor >= data.nodes.size()) {
        throw std::logic_error("compiled node predecessor is out of range");
      }
      const auto source = plan.job_of_node[predecessor];
      if (source == destination) {
        continue;
      }
      append_unique(job_successors[source], destination);
    }
  }
  const auto reduced_job_successors = transitive_reduction(job_successors);
  const auto reduced_job_predecessors =
      invert_adjacency(reduced_job_successors);
  for (JobId job = 0; job < plan.jobs.size(); ++job) {
    auto &compiled = plan.jobs[job];
    compiled.predecessors.offset =
        static_cast<std::uint32_t>(plan.storage.predecessors.size());
    compiled.predecessors.count =
        static_cast<std::uint32_t>(reduced_job_predecessors[job].size());
    for (const auto predecessor : reduced_job_predecessors[job]) {
      plan.storage.predecessors.push_back(predecessor);
    }
    compiled.successors.offset =
        static_cast<std::uint32_t>(plan.storage.successors.size());
    compiled.successors.count =
        static_cast<std::uint32_t>(reduced_job_successors[job].size());
    for (const auto successor : reduced_job_successors[job]) {
      plan.storage.successors.push_back(successor);
    }
  }

  Adjacency<ResourceId> frontier_resources(plan.jobs.size(), allocator_ref{});
  Adjacency<ResourceId> released_resources(plan.jobs.size(), allocator_ref{});
  for (ResourceId resource = 0; resource < data.resources.size(); ++resource) {
    const auto &lifetime = data.resources[resource].lifetime;
    if (lifetime.release_after.count == 0) {
      continue;
    }
    if (lifetime.release_after.offset + lifetime.release_after.count >
        data.release_after.size()) {
      throw std::logic_error("resource release frontier is out of range");
    }
    NodeId last_frontier = 0;
    for (const auto frontier :
         data.release_after_of(data.resources[resource])) {
      if (frontier >= plan.job_of_node.size()) {
        throw std::logic_error("resource release frontier is out of range");
      }
      const auto job = plan.job_of_node[frontier];
      if (job >= plan.jobs.size()) {
        throw std::logic_error("resource release frontier has no job");
      }
      append_unique(frontier_resources[job], resource);
      last_frontier = std::max(last_frontier, frontier);
    }
    const bool release_preconstructed =
        lifetime.preconstructed && !lifetime.retained_by_feedback;
    if (lifetime.destroy != nullptr &&
        (!lifetime.destroy_at_shutdown || release_preconstructed)) {
      released_resources[plan.job_of_node[last_frontier]].push_back(resource);
    }
  }
  for (JobId job = 0; job < plan.jobs.size(); ++job) {
    auto &compiled = plan.jobs[job];
    compiled.resource_frontiers.offset =
        static_cast<std::uint32_t>(plan.storage.resource_frontiers.size());
    for (const auto resource : frontier_resources[job]) {
      plan.storage.resource_frontiers.push_back(resource);
    }
    compiled.resource_frontiers.count =
        static_cast<std::uint32_t>(plan.storage.resource_frontiers.size()) -
        compiled.resource_frontiers.offset;
    compiled.released_resources.offset =
        static_cast<std::uint32_t>(plan.storage.released_resources.size());
    for (const auto resource : released_resources[job]) {
      plan.storage.released_resources.push_back(resource);
    }
    compiled.released_resources.count =
        static_cast<std::uint32_t>(plan.storage.released_resources.size()) -
        compiled.released_resources.offset;
  }
  return plan;
}

} // namespace compiler

String<allocator_ref> to_pretty(const JobPlan &plan, const ProgramData &data) {
  std::string result;
  for (JobId job = 0; job < plan.jobs.size(); ++job) {
    const auto &compiled = plan.jobs[job];
    fmt::format_to(std::back_inserter(result), "job %{} deps={{", job);
    const auto predecessors = plan.predecessors_of(compiled);
    for (std::size_t index = 0; index < predecessors.size(); ++index) {
      if (index != 0) {
        result += ',';
      }
      fmt::format_to(std::back_inserter(result), "%{}", predecessors[index]);
    }
    result += "} {\n";
    for (const auto node : plan.nodes_of(compiled)) {
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
