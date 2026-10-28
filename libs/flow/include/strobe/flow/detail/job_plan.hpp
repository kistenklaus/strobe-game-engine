#pragma once

#include "strobe/flow/allocator.hpp"
#include "strobe/flow/detail/program_data.hpp"

#include <strobe/core/containers/span.hpp>
#include <strobe/core/containers/string.hpp>
#include <strobe/core/containers/vector.hpp>

#include <cstdint>
#include <limits>

namespace strobe::flow::detail {

using JobId = std::uint32_t;

inline constexpr JobId null_job_id = std::numeric_limits<JobId>::max();

struct CompiledJob {
  IndexRange nodes;
  IndexRange predecessors;
  IndexRange successors;
  IndexRange resource_frontiers;
  IndexRange released_resources;
};

struct CompiledJobStorage {
  Vector<NodeId, allocator_ref> nodes;
  Vector<JobId, allocator_ref> predecessors;
  Vector<JobId, allocator_ref> successors;
  Vector<ResourceId, allocator_ref> resource_frontiers;
  Vector<ResourceId, allocator_ref> released_resources;
};

struct JobPlan {
  Vector<CompiledJob, allocator_ref> jobs;
  CompiledJobStorage storage;
  Vector<JobId, allocator_ref> job_of_node;

  span<NodeId> nodes_of(CompiledJob &job) noexcept;
  span<const NodeId> nodes_of(const CompiledJob &job) const noexcept;
  span<NodeId> nodes_of(JobId job) noexcept;
  span<const NodeId> nodes_of(JobId job) const noexcept;

  span<JobId> predecessors_of(CompiledJob &job) noexcept;
  span<const JobId> predecessors_of(const CompiledJob &job) const noexcept;
  span<JobId> predecessors_of(JobId job) noexcept;
  span<const JobId> predecessors_of(JobId job) const noexcept;

  span<JobId> successors_of(CompiledJob &job) noexcept;
  span<const JobId> successors_of(const CompiledJob &job) const noexcept;
  span<JobId> successors_of(JobId job) noexcept;
  span<const JobId> successors_of(JobId job) const noexcept;

  span<ResourceId> resource_frontiers_of(CompiledJob &job) noexcept;
  span<const ResourceId>
  resource_frontiers_of(const CompiledJob &job) const noexcept;
  span<ResourceId> resource_frontiers_of(JobId job) noexcept;
  span<const ResourceId> resource_frontiers_of(JobId job) const noexcept;

  span<ResourceId> released_resources_of(CompiledJob &job) noexcept;
  span<const ResourceId>
  released_resources_of(const CompiledJob &job) const noexcept;
  span<ResourceId> released_resources_of(JobId job) noexcept;
  span<const ResourceId> released_resources_of(JobId job) const noexcept;
};

namespace compiler {

JobPlan build_job_plan(const ProgramData &data);

} // namespace compiler

String<allocator_ref> to_pretty(const JobPlan &plan, const ProgramData &data);

} // namespace strobe::flow::detail
