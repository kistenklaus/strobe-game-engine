#include <strobe/flow/detail/program_artifact.hpp>

#include "strobe/flow/detail/compiler/plan_physical_storage.hpp"

#include <algorithm>
#include <limits>
#include <optional>
#include <stdexcept>

namespace strobe::flow::detail::compiler {
namespace {

void finalize_activation_layout(ProgramData &data) {
  if (data.activation_layout.size != 0 || data.nodes.empty()) {
    return;
  }
  auto &layout = data.activation_layout;
  layout.capacity = 1;
  std::size_t offset = 0;
  const auto append = [&](std::size_t count, std::size_t size,
                          std::size_t alignment) {
    offset = (offset + alignment - 1) & ~(alignment - 1);
    const auto result = offset;
    offset += count * size;
    layout.alignment = std::max(layout.alignment, alignment);
    return result;
  };
  layout.values_offset =
      append(data.values.size(), sizeof(void *), alignof(void *));
  layout.arguments_offset = append(data.node_storage.accesses.size(),
                                   sizeof(void *), alignof(void *));
  layout.generations_offset =
      append(data.resources.size(), sizeof(std::optional<ResourceGeneration>),
             alignof(std::optional<ResourceGeneration>));
  layout.previous_generations_offset =
      append(data.resources.size(), sizeof(std::optional<ResourceGeneration>),
             alignof(std::optional<ResourceGeneration>));
  layout.changed_values_offset =
      append(data.values.size(), sizeof(std::uint8_t), alignof(std::uint8_t));
  offset = std::max(offset, std::size_t{1});
  layout.stride = (offset + layout.alignment - 1) & ~(layout.alignment - 1);
  layout.size = layout.stride;
}

FeedbackSubmissionPlan build_feedback_submission_plan(const ProgramData &data,
                                                      const JobPlan &jobs) {
  FeedbackSubmissionPlan plan;
  plan.region_of_node.resize(data.nodes.size(),
                             std::numeric_limits<std::uint32_t>::max());

  for (std::uint32_t region = 0; region < data.feedback_regions.size();
       ++region) {
    const auto &compiled_region = data.feedback_regions[region];
    if (compiled_region.nodes.offset + compiled_region.nodes.count >
        data.feedback_region_nodes.size()) {
      throw std::logic_error("feedback region nodes are out of range");
    }
    for (std::uint32_t index = 0; index < compiled_region.nodes.count;
         ++index) {
      const auto node =
          data.feedback_region_nodes[compiled_region.nodes.offset + index];
      if (node >= data.nodes.size()) {
        throw std::logic_error("feedback region node is out of range");
      }
      if (plan.region_of_node[node] !=
          std::numeric_limits<std::uint32_t>::max()) {
        throw std::logic_error("node belongs to multiple feedback regions");
      }
      plan.region_of_node[node] = region;
    }
  }

  for (std::uint32_t region = 0; region < data.feedback_regions.size();
       ++region) {
    const auto &compiled_region = data.feedback_regions[region];
    const auto job_offset = plan.jobs.size();
    std::size_t dependency_capacity =
        compiled_region.nodes.count + data.resources.size() + 2;
    for (std::uint32_t index = 0; index < compiled_region.nodes.count;
         ++index) {
      const auto node =
          data.feedback_region_nodes[compiled_region.nodes.offset + index];
      if (node >= jobs.job_of_node.size()) {
        throw std::logic_error("feedback node has no planned job");
      }
      const auto job = jobs.job_of_node[node];
      if (job >= jobs.jobs.size()) {
        throw std::logic_error("feedback planned job is out of range");
      }
      const auto &planned = jobs.jobs[job];
      const auto planned_nodes = jobs.nodes_of(planned);
      if (planned_nodes.empty()) {
        throw std::logic_error("feedback planned job is malformed");
      }
      if (planned_nodes.front() != node) {
        continue;
      }
      for (const auto planned_node : planned_nodes) {
        if (planned_node >= plan.region_of_node.size() ||
            plan.region_of_node[planned_node] != region) {
          throw std::logic_error(
              "planned job crosses a feedback region boundary");
        }
      }
      plan.jobs.push_back(job);
      std::size_t count = jobs.predecessors_of(planned).size() + 2;
      for (const auto planned_node : planned_nodes) {
        count += data.nodes[planned_node].temporal_predecessors.count + 1;
      }
      dependency_capacity = std::max(dependency_capacity, count);
    }

    const auto terminal_offset = plan.terminal_jobs.size();
    for (std::uint32_t index = 0; index < compiled_region.nodes.count;
         ++index) {
      const auto node =
          data.feedback_region_nodes[compiled_region.nodes.offset + index];
      bool has_successor = false;
      const auto &compiled = data.nodes[node];
      for (std::uint32_t successor_index = 0;
           successor_index < compiled.successors.count; ++successor_index) {
        const auto successor =
            data.node_storage
                .successors[compiled.successors.offset + successor_index];
        has_successor |= successor < plan.region_of_node.size() &&
                         plan.region_of_node[successor] == region;
      }
      if (!has_successor) {
        const auto job = jobs.job_of_node[node];
        if (std::find(plan.terminal_jobs.begin() + terminal_offset,
                      plan.terminal_jobs.end(),
                      job) == plan.terminal_jobs.end()) {
          plan.terminal_jobs.push_back(job);
        }
      }
    }
    plan.regions.push_back(CompiledFeedbackSubmissionRegion{
        .jobs =
            {
                .offset = static_cast<std::uint32_t>(job_offset),
                .count =
                    static_cast<std::uint32_t>(plan.jobs.size() - job_offset),
            },
        .terminal_jobs =
            {
                .offset = static_cast<std::uint32_t>(terminal_offset),
                .count = static_cast<std::uint32_t>(plan.terminal_jobs.size() -
                                                    terminal_offset),
            },
        .dependency_capacity = static_cast<std::uint32_t>(dependency_capacity),
    });
  }
  return plan;
}

} // namespace

ProgramArtifact build_program_artifact(ProgramData data) {
  finalize_activation_layout(data);
  plan_physical_storage(data);
  auto jobs = build_job_plan(data);
  auto feedback = build_feedback_submission_plan(data, jobs);
  return ProgramArtifact{
      .data = std::move(data),
      .jobs = std::move(jobs),
      .feedback = std::move(feedback),
  };
}

} // namespace strobe::flow::detail::compiler
