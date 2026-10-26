#pragma once

#include "strobe/flow/allocator.hpp"
#include "strobe/flow/detail/program_data.hpp"

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
};

struct JobPlan {
  Vector<CompiledJob, allocator_ref> jobs;
  Vector<NodeId, allocator_ref> nodes;
  Vector<JobId, allocator_ref> predecessors;
  Vector<JobId, allocator_ref> successors;
  Vector<JobId, allocator_ref> job_of_node;
};

JobPlan build_job_plan(const ProgramData &data);

String<allocator_ref> to_pretty(const JobPlan &plan, const ProgramData &data);

} // namespace strobe::flow::detail
