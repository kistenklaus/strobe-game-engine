#include <strobe/flow/detail/job_plan.hpp>

#include <gtest/gtest.h>

namespace {

namespace flow = strobe::flow;

void destroy_resource(void *) noexcept {}

void append_node(
    flow::detail::ProgramData &data,
    std::initializer_list<flow::detail::NodeId> predecessors,
    flow::detail::OperationKind kind = flow::detail::OperationKind::callback) {
  const auto offset =
      static_cast<std::uint32_t>(data.node_storage.predecessors.size());
  for (const auto predecessor : predecessors) {
    data.node_storage.predecessors.push_back(predecessor);
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

  const auto plan = flow::detail::compiler::build_job_plan(data);

  ASSERT_EQ(plan.jobs.size(), 1);
  EXPECT_EQ(plan.nodes_of(0).size(), 3);
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

  const auto plan = flow::detail::compiler::build_job_plan(data);

  ASSERT_EQ(plan.jobs.size(), 3);
  EXPECT_EQ(plan.job_of_node[0], plan.job_of_node[1]);
  EXPECT_NE(plan.job_of_node[1], plan.job_of_node[2]);
  EXPECT_NE(plan.job_of_node[2], plan.job_of_node[3]);
}

TEST(FlowJobPlan, CoalescesTheTailOfAForkedChain) {
  flow::detail::ProgramData data;
  append_node(data, {});     // A
  append_node(data, {0});    // B
  append_node(data, {0, 1}); // C
  append_node(data, {0});    // D

  const auto plan = flow::detail::compiler::build_job_plan(data);

  ASSERT_EQ(plan.jobs.size(), 3);
  EXPECT_NE(plan.job_of_node[0], plan.job_of_node[1]);
  EXPECT_EQ(plan.job_of_node[1], plan.job_of_node[2]);
  EXPECT_NE(plan.job_of_node[1], plan.job_of_node[3]);
}

TEST(FlowJobPlan, RemovesTransitiveJobDependencies) {
  flow::detail::ProgramData data;
  append_node(data, {});
  append_node(data, {0}, flow::detail::OperationKind::acquire);
  append_node(data, {0, 1});

  const auto plan = flow::detail::compiler::build_job_plan(data);

  ASSERT_EQ(plan.jobs.size(), 3);
  const auto final_job = plan.job_of_node[2];
  const auto &compiled = plan.jobs[final_job];
  ASSERT_EQ(compiled.predecessors.count, 1);
  ASSERT_EQ(plan.predecessors_of(compiled).size(), 1);
  EXPECT_EQ(plan.predecessors_of(compiled).front(), plan.job_of_node[1]);
}

TEST(FlowJobPlan, DoesNotCrossAcquireBoundaries) {
  flow::detail::ProgramData data;
  append_node(data, {});
  append_node(data, {0}, flow::detail::OperationKind::acquire);
  append_node(data, {1});

  const auto plan = flow::detail::compiler::build_job_plan(data);

  ASSERT_EQ(plan.jobs.size(), 3);
  EXPECT_NE(plan.job_of_node[0], plan.job_of_node[1]);
  EXPECT_NE(plan.job_of_node[1], plan.job_of_node[2]);
}

TEST(FlowJobPlan, DoesNotCrossResourceReleaseFrontiers) {
  flow::detail::ProgramData data;
  append_node(data, {});
  append_node(data, {0});
  data.release_after.push_back(0);
  data.resources.push_back(flow::detail::CompiledResource{
      .lifetime =
          {
              .interval = {},
              .constructions = {},
              .release_after = {.offset = 0, .count = 1},
              .destroy = destroy_resource,
          },
  });

  const auto plan = flow::detail::compiler::build_job_plan(data);

  ASSERT_EQ(plan.jobs.size(), 2);
  EXPECT_NE(plan.job_of_node[0], plan.job_of_node[1]);
}

TEST(FlowJobPlan, PrecomputesResourceFrontierJobs) {
  flow::detail::ProgramData data;
  append_node(data, {});
  append_node(data, {});
  data.release_after.push_back(0);
  data.release_after.push_back(1);
  data.resources.push_back(flow::detail::CompiledResource{
      .lifetime =
          {
              .interval = {},
              .constructions = {},
              .release_after = {.offset = 0, .count = 2},
              .destroy = destroy_resource,
          },
  });

  const auto plan = flow::detail::compiler::build_job_plan(data);
  const auto first_job = plan.job_of_node[0];
  const auto second_job = plan.job_of_node[1];

  ASSERT_NE(first_job, second_job);
  ASSERT_EQ(plan.resource_frontiers_of(first_job).size(), 1);
  EXPECT_EQ(plan.resource_frontiers_of(first_job).front(), 0);
  EXPECT_TRUE(plan.released_resources_of(first_job).empty());
  ASSERT_EQ(plan.resource_frontiers_of(second_job).size(), 1);
  EXPECT_EQ(plan.resource_frontiers_of(second_job).front(), 0);
  ASSERT_EQ(plan.released_resources_of(second_job).size(), 1);
  EXPECT_EQ(plan.released_resources_of(second_job).front(), 0);
}

TEST(FlowJobPlan, PrettyPrintsJobsWithOriginalNodeIds) {
  flow::detail::ProgramData data;
  append_node(data, {});
  append_node(data, {0});

  const auto plan = flow::detail::compiler::build_job_plan(data);
  const auto pretty = flow::detail::to_pretty(plan, data);

  EXPECT_EQ(pretty.view(), "job %0 deps={} {\n  @0\n  @1\n}");
}

} // namespace
