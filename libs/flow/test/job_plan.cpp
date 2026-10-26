#include <strobe/flow/detail/job_plan.hpp>

#include <gtest/gtest.h>

namespace {

namespace flow = strobe::flow;

void append_node(
    flow::detail::ProgramData &data,
    std::initializer_list<flow::detail::NodeId> predecessors,
    flow::detail::OperationKind kind = flow::detail::OperationKind::callback) {
  const auto offset = static_cast<std::uint32_t>(data.predecessors.size());
  for (const auto predecessor : predecessors) {
    data.predecessors.push_back(predecessor);
  }
  data.nodes.push_back(flow::detail::CompiledNode{
      .kind = kind,
      .inputs = {},
      .outputs = {},
      .accesses = {},
      .predecessors =
          {
              .offset = offset,
              .count = static_cast<std::uint32_t>(predecessors.size()),
          },
      .successors = {},
      .temporal_predecessors = {},
      .temporal_successors = {},
  });
}

TEST(FlowJobPlan, CoalescesAContiguousDependencyChain) {
  flow::detail::ProgramData data;
  append_node(data, {});
  append_node(data, {0});
  append_node(data, {1});

  const auto plan = flow::detail::build_job_plan(data);

  ASSERT_EQ(plan.jobs.size(), 1);
  EXPECT_EQ(plan.jobs[0].nodes.count, 3);
  EXPECT_EQ(plan.job_of_node[0], 0);
  EXPECT_EQ(plan.job_of_node[1], 0);
  EXPECT_EQ(plan.job_of_node[2], 0);
}

TEST(FlowJobPlan, PreservesParallelismAtAJoin) {
  flow::detail::ProgramData data;
  append_node(data, {});
  append_node(data, {0});
  append_node(data, {});
  append_node(data, {1, 2});

  const auto plan = flow::detail::build_job_plan(data);

  ASSERT_EQ(plan.jobs.size(), 3);
  EXPECT_EQ(plan.job_of_node[0], plan.job_of_node[1]);
  EXPECT_NE(plan.job_of_node[1], plan.job_of_node[2]);
  EXPECT_NE(plan.job_of_node[2], plan.job_of_node[3]);
}

TEST(FlowJobPlan, DoesNotCrossAcquireBoundaries) {
  flow::detail::ProgramData data;
  append_node(data, {});
  append_node(data, {0}, flow::detail::OperationKind::acquire);
  append_node(data, {1});

  const auto plan = flow::detail::build_job_plan(data);

  ASSERT_EQ(plan.jobs.size(), 3);
  EXPECT_NE(plan.job_of_node[0], plan.job_of_node[1]);
  EXPECT_NE(plan.job_of_node[1], plan.job_of_node[2]);
}

TEST(FlowJobPlan, DoesNotCrossResourceReleaseFrontiers) {
  flow::detail::ProgramData data;
  append_node(data, {});
  append_node(data, {0});
  data.release_after.push_back(0);
  data.resource_lifetimes.push_back(flow::detail::ResourceLifetimePlan{
      .interval = {},
      .constructions = {},
      .release_after = {.offset = 0, .count = 1},
      .repeating = false,
      .preconstructed = false,
      .retained_by_feedback = false,
      .destroy_at_shutdown = false,
      .destroy = nullptr,
  });

  const auto plan = flow::detail::build_job_plan(data);

  ASSERT_EQ(plan.jobs.size(), 2);
  EXPECT_NE(plan.job_of_node[0], plan.job_of_node[1]);
}

TEST(FlowJobPlan, PrettyPrintsJobsWithOriginalNodeIds) {
  flow::detail::ProgramData data;
  append_node(data, {});
  append_node(data, {0});

  const auto plan = flow::detail::build_job_plan(data);
  const auto pretty = flow::detail::to_pretty(plan, data);

  EXPECT_EQ(pretty.view(), "job %0 deps={} {\n  @0\n  @1\n}");
}

} // namespace
