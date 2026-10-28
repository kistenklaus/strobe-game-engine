#pragma once

#include "strobe/flow/detail/job_plan.hpp"
#include "strobe/flow/detail/program_data.hpp"

namespace strobe::flow::detail {

struct CompiledFeedbackSubmissionRegion {
  IndexRange jobs;
  IndexRange terminal_jobs;
  std::uint32_t dependency_capacity = 0;
};

struct FeedbackSubmissionPlan {
  Vector<CompiledFeedbackSubmissionRegion, allocator_ref> regions;
  Vector<JobId, allocator_ref> jobs;
  Vector<JobId, allocator_ref> terminal_jobs;
  Vector<std::uint32_t, allocator_ref> region_of_node;
};

struct ProgramArtifact {
  ProgramData data;
  JobPlan jobs;
  FeedbackSubmissionPlan feedback;
};

namespace compiler {

ProgramArtifact build_program_artifact(ProgramData data);

} // namespace compiler

} // namespace strobe::flow::detail
