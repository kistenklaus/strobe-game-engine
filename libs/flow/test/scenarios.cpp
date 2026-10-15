#include <strobe/flow/flow.hpp>

#include <strobe/core/sync/job_scheduler.hpp>

#include <gtest/gtest.h>

#include <atomic>
#include <cstdint>
#include <memory>

namespace flow = strobe::flow;

namespace {

struct Extent {
  std::uint32_t width;
  std::uint32_t height;

  bool operator==(const Extent &) const = default;
};

struct Mesh {
  std::uint32_t triangles;
};

struct Blas {
  std::uint32_t triangles = 0;
  bool built = false;
};

struct GBuffer {
  Extent extent;
};

struct TrackedResource {
  std::atomic<std::uint32_t> *destructions;
  std::uint32_t value;

  TrackedResource(std::atomic<std::uint32_t> *destruction_counter,
                  std::uint32_t resource_value)
      : destructions(destruction_counter), value(resource_value) {}

  TrackedResource(const TrackedResource &) = delete;
  TrackedResource &operator=(const TrackedResource &) = delete;

  ~TrackedResource() { destructions->fetch_add(1, std::memory_order_relaxed); }
};

struct ReusableValue {
  std::atomic<std::uint32_t> *destructions;
  std::uint32_t value;

  ReusableValue(std::atomic<std::uint32_t> *destruction_counter,
                std::uint32_t initial_value)
      : destructions(destruction_counter), value(initial_value) {}

  ReusableValue(const ReusableValue &) = delete;
  ReusableValue &operator=(const ReusableValue &) = delete;

  ~ReusableValue() { destructions->fetch_add(1, std::memory_order_relaxed); }
};

void submit_and_wait(flow::Program &program,
                     strobe::sync::JobScheduler &scheduler) {
  const auto completion = program.submit(&scheduler);
  try {
    scheduler.wait(completion);
  } catch (...) {
    program.rethrow_execution_error();
    throw;
  }
  program.rethrow_execution_error();
}

TEST(FlowScenario, RunsPersistentResizeAndPerFrameRenderingTogether) {
  Extent swapchain_extent{1280, 720};
  std::atomic<std::uint32_t> mesh_constructions = 0;
  std::atomic<std::uint32_t> blas_allocations = 0;
  std::atomic<std::uint32_t> blas_builds = 0;
  std::atomic<std::uint32_t> gbuffer_constructions = 0;
  std::atomic<std::uint32_t> frame_seed_constructions = 0;
  std::atomic<std::uint32_t> renders = 0;
  std::atomic<std::uint32_t> presents = 0;
  std::atomic<std::uint32_t> presented_sum = 0;

  flow::begin(flow::CompileOptions{.max_slots = 3});
  flow::acquire("frame");

  auto mesh = flow::resource<Mesh>();
  flow::construct(mesh, [&](Mesh *output) {
    mesh_constructions.fetch_add(1, std::memory_order_relaxed);
    std::construct_at(output, Mesh{.triangles = 1024});
  });
  auto blas = flow::resource<Blas>();
  flow::construct(blas, [&](Blas *output) {
    blas_allocations.fetch_add(1, std::memory_order_relaxed);
    std::construct_at(output);
  });
  flow::op(flow::read(mesh), flow::write(blas),
           [&](const Mesh &source, Blas &destination) {
             destination.triangles = source.triangles;
             destination.built = true;
             blas_builds.fetch_add(1, std::memory_order_relaxed);
           });

  auto external_extent = flow::external<Extent>("swapchain-extent");
  auto stable_extent = flow::cutoff(external_extent);
  auto gbuffer = flow::resource<GBuffer>();
  flow::construct(gbuffer, flow::read(stable_extent),
                  [&](GBuffer *output, const Extent &extent) {
                    gbuffer_constructions.fetch_add(1,
                                                    std::memory_order_relaxed);
                    std::construct_at(output, GBuffer{.extent = extent});
                  });

  auto initial_frame = flow::resource<std::uint32_t>();
  flow::construct(initial_frame, [&](std::uint32_t *output) {
    frame_seed_constructions.fetch_add(1, std::memory_order_relaxed);
    std::construct_at(output, 0);
  });
  auto previous_frame = flow::phi(initial_frame);
  auto current_frame = flow::resource<std::uint32_t>();
  flow::construct(current_frame, 0);
  flow::op(flow::read(previous_frame), flow::read(blas), flow::read(gbuffer),
           flow::write(current_frame),
           [&](const std::uint32_t &previous, const Blas &acceleration,
               const GBuffer &targets, std::uint32_t &current) {
             EXPECT_TRUE(acceleration.built);
             EXPECT_EQ(acceleration.triangles, 1024);
             EXPECT_EQ(targets.extent, swapchain_extent);
             current = previous + 1;
             renders.fetch_add(1, std::memory_order_relaxed);
           });
  flow::op(flow::OpMetadata("Present", "", flow::OpSemantics::side_effect),
           flow::read(current_frame), [&](const std::uint32_t &frame) {
             presents.fetch_add(1, std::memory_order_relaxed);
             presented_sum.fetch_add(frame, std::memory_order_relaxed);
           });
  flow::feedback(previous_frame, current_frame);
  auto program = flow::end();
  program.bind_external("swapchain-extent", &swapchain_extent);

  strobe::sync::allocator allocator;
  strobe::sync::JobScheduler scheduler(&allocator, 4);

  submit_and_wait(program, scheduler);
  EXPECT_EQ(mesh_constructions.load(std::memory_order_relaxed), 1);
  EXPECT_EQ(blas_allocations.load(std::memory_order_relaxed), 1);
  EXPECT_EQ(blas_builds.load(std::memory_order_relaxed), 1);
  EXPECT_EQ(frame_seed_constructions.load(std::memory_order_relaxed), 1);
  EXPECT_EQ(renders.load(std::memory_order_relaxed), 0);

  program.release("frame", 3);
  submit_and_wait(program, scheduler);
  EXPECT_EQ(gbuffer_constructions.load(std::memory_order_relaxed), 1);
  EXPECT_EQ(renders.load(std::memory_order_relaxed), 3);
  EXPECT_EQ(presents.load(std::memory_order_relaxed), 3);

  swapchain_extent = Extent{1920, 1080};
  program.release("frame", 2);
  submit_and_wait(program, scheduler);
  scheduler.stop();

  EXPECT_EQ(mesh_constructions.load(std::memory_order_relaxed), 1);
  EXPECT_EQ(blas_allocations.load(std::memory_order_relaxed), 1);
  EXPECT_EQ(blas_builds.load(std::memory_order_relaxed), 1);
  EXPECT_EQ(gbuffer_constructions.load(std::memory_order_relaxed), 2);
  EXPECT_EQ(frame_seed_constructions.load(std::memory_order_relaxed), 1);
  EXPECT_EQ(renders.load(std::memory_order_relaxed), 5);
  EXPECT_EQ(presents.load(std::memory_order_relaxed), 5);
  EXPECT_EQ(presented_sum.load(std::memory_order_relaxed), 15);
}

TEST(FlowScenario, PreservesFeedbackAndMemoizationAcrossYieldedFrames) {
  std::uint32_t asset_revision = 2;
  std::atomic<std::uint32_t> seed_constructions = 0;
  std::atomic<std::uint32_t> preparations = 0;
  std::atomic<std::uint32_t> before_yield = 0;
  std::atomic<std::uint32_t> after_yield = 0;
  std::atomic<std::uint32_t> completed_sum = 0;

  flow::begin(flow::CompileOptions{.max_slots = 3});
  flow::acquire("frame");
  auto external_revision = flow::external<std::uint32_t>("asset-revision");
  auto stable_revision = flow::cutoff(external_revision);
  auto prepared_asset = flow::resource<std::uint32_t>();
  flow::construct(prepared_asset, flow::read(stable_revision),
                  [&](std::uint32_t *output, const std::uint32_t &revision) {
                    preparations.fetch_add(1, std::memory_order_relaxed);
                    std::construct_at(output, revision);
                  });

  auto initial = flow::resource<std::uint32_t>();
  flow::construct(initial, [&](std::uint32_t *output) {
    seed_constructions.fetch_add(1, std::memory_order_relaxed);
    std::construct_at(output, 0);
  });
  auto previous = flow::phi(initial);
  auto current = flow::resource<std::uint32_t>();
  flow::construct(current, 0);
  flow::op(flow::read(previous), flow::read(prepared_asset),
           flow::write(current),
           [](const std::uint32_t &prior, const std::uint32_t &revision,
              std::uint32_t &output) { output = prior + revision; });
  flow::op(flow::OpMetadata("Before yield", "", flow::OpSemantics::side_effect),
           flow::read(current), [&](const std::uint32_t &) {
             before_yield.fetch_add(1, std::memory_order_relaxed);
           });
  flow::yield();
  flow::op(
      flow::OpMetadata("Complete frame", "", flow::OpSemantics::side_effect),
      flow::read(current), [&](const std::uint32_t &value) {
        after_yield.fetch_add(1, std::memory_order_relaxed);
        completed_sum.fetch_add(value, std::memory_order_relaxed);
      });
  flow::feedback(previous, current);
  auto program = flow::end();
  program.bind_external("asset-revision", &asset_revision);

  strobe::sync::allocator allocator;
  strobe::sync::JobScheduler scheduler(&allocator, 3);
  const auto submit = [&] { submit_and_wait(program, scheduler); };

  submit();
  EXPECT_EQ(seed_constructions.load(std::memory_order_relaxed), 1);
  EXPECT_EQ(before_yield.load(std::memory_order_relaxed), 0);

  program.release("frame", 3);
  submit();
  EXPECT_EQ(before_yield.load(std::memory_order_relaxed), 1);
  EXPECT_EQ(after_yield.load(std::memory_order_relaxed), 0);
  submit();
  submit();
  submit();
  EXPECT_EQ(preparations.load(std::memory_order_relaxed), 1);
  EXPECT_EQ(before_yield.load(std::memory_order_relaxed), 3);
  EXPECT_EQ(after_yield.load(std::memory_order_relaxed), 3);

  asset_revision = 3;
  program.release("frame", 2);
  submit();
  submit();
  submit();
  scheduler.stop();

  EXPECT_EQ(seed_constructions.load(std::memory_order_relaxed), 1);
  EXPECT_EQ(preparations.load(std::memory_order_relaxed), 2);
  EXPECT_EQ(before_yield.load(std::memory_order_relaxed), 5);
  EXPECT_EQ(after_yield.load(std::memory_order_relaxed), 5);
  EXPECT_EQ(completed_sum.load(std::memory_order_relaxed), 33);
}

TEST(FlowScenario, JoinsIndependentFrameBranchesBeforePresentation) {
  std::atomic<std::uint32_t> left_jobs = 0;
  std::atomic<std::uint32_t> right_jobs = 0;
  std::atomic<std::uint32_t> composites = 0;
  std::atomic<std::uint32_t> presented_sum = 0;

  flow::begin(flow::CompileOptions{.max_slots = 4});
  flow::acquire("frame");
  auto initial = flow::resource<std::uint32_t>();
  flow::construct(initial, 0);
  auto previous = flow::phi(initial);

  auto frame = flow::resource<std::uint32_t>();
  flow::construct(frame, flow::read(previous),
                  [](std::uint32_t *output, const std::uint32_t &prior) {
                    std::construct_at(output, prior + 1);
                  });
  auto left = flow::resource<std::uint32_t>();
  flow::construct(left, 0);
  flow::op(flow::read(frame), flow::write(left),
           [&](const std::uint32_t &number, std::uint32_t &output) {
             output = number * 2;
             left_jobs.fetch_add(1, std::memory_order_relaxed);
           });
  auto right = flow::resource<std::uint32_t>();
  flow::construct(right, 0);
  flow::op(flow::read(frame), flow::write(right),
           [&](const std::uint32_t &number, std::uint32_t &output) {
             output = number * 3;
             right_jobs.fetch_add(1, std::memory_order_relaxed);
           });
  auto composite = flow::resource<std::uint32_t>();
  flow::construct(composite, 0);
  flow::op(flow::read(left), flow::read(right), flow::write(composite),
           [&](const std::uint32_t &left_value,
               const std::uint32_t &right_value, std::uint32_t &output) {
             output = left_value + right_value;
             composites.fetch_add(1, std::memory_order_relaxed);
           });
  flow::op(flow::OpMetadata("Present", "", flow::OpSemantics::side_effect),
           flow::read(composite), [&](const std::uint32_t &value) {
             presented_sum.fetch_add(value, std::memory_order_relaxed);
           });
  flow::feedback(previous, frame);
  auto program = flow::end();
  program.release("frame", 4);

  strobe::sync::allocator allocator;
  strobe::sync::JobScheduler scheduler(&allocator, 4);
  submit_and_wait(program, scheduler);
  scheduler.stop();

  EXPECT_EQ(left_jobs.load(std::memory_order_relaxed), 4);
  EXPECT_EQ(right_jobs.load(std::memory_order_relaxed), 4);
  EXPECT_EQ(composites.load(std::memory_order_relaxed), 4);
  EXPECT_EQ(presented_sum.load(std::memory_order_relaxed), 50);
}

TEST(FlowScenario, RunsStructuredControlInsideAFrameStateMachine) {
  std::atomic<std::uint32_t> loop_iterations = 0;
  std::atomic<std::uint32_t> even_branches = 0;
  std::atomic<std::uint32_t> odd_branches = 0;
  std::atomic<std::uint32_t> completed_frames = 0;

  flow::begin(flow::CompileOptions{.max_slots = 2});
  flow::acquire("frame");
  auto initial_frame = flow::resource<std::uint32_t>();
  flow::construct(initial_frame, 0);
  auto previous_frame = flow::phi(initial_frame);

  auto frame = flow::resource<std::uint32_t>();
  flow::construct(frame, flow::read(previous_frame),
                  [](std::uint32_t *output, const std::uint32_t &prior) {
                    std::construct_at(output, prior + 1);
                  });
  auto counter = flow::resource<std::uint32_t>();
  flow::construct(counter, 0);
  auto repeat = flow::resource<bool>();
  flow::construct(repeat, true);
  auto even = flow::resource<bool>();
  flow::construct(even, false);
  flow::op(flow::write(counter), flow::write(repeat),
           [](std::uint32_t &value, bool &condition) {
             value = 0;
             condition = true;
           });
  flow::while_(repeat, [&] {
    flow::op(flow::read_write(counter), flow::write(even),
             [&](std::uint32_t &value, bool &condition) {
               ++value;
               condition = value % 2 == 0;
               loop_iterations.fetch_add(1, std::memory_order_relaxed);
             });
    flow::if_(
        even,
        [&] {
          flow::op(
              flow::OpMetadata("Even", "", flow::OpSemantics::side_effect),
              [&] { even_branches.fetch_add(1, std::memory_order_relaxed); });
        },
        [&] {
          flow::op(
              flow::OpMetadata("Odd", "", flow::OpSemantics::side_effect),
              [&] { odd_branches.fetch_add(1, std::memory_order_relaxed); });
        });
    flow::op(flow::read(counter), flow::write(repeat),
             [](const std::uint32_t &value, bool &condition) {
               condition = value < 3;
             });
  });
  flow::op(flow::OpMetadata("Complete", "", flow::OpSemantics::side_effect),
           flow::read(frame), [&](const std::uint32_t &) {
             completed_frames.fetch_add(1, std::memory_order_relaxed);
           });
  flow::feedback(previous_frame, frame);
  auto program = flow::end();
  program.release("frame", 2);

  strobe::sync::allocator allocator;
  strobe::sync::JobScheduler scheduler(&allocator, 2);
  submit_and_wait(program, scheduler);
  scheduler.stop();

  EXPECT_EQ(loop_iterations.load(std::memory_order_relaxed), 6);
  EXPECT_EQ(even_branches.load(std::memory_order_relaxed), 2);
  EXPECT_EQ(odd_branches.load(std::memory_order_relaxed), 4);
  EXPECT_EQ(completed_frames.load(std::memory_order_relaxed), 2);
}

TEST(FlowScenario, ReconstructsAndDestroysRetainedCutoffResources) {
  std::uint32_t revision = 1;
  std::atomic<std::uint32_t> constructions = 0;
  std::atomic<std::uint32_t> destructions = 0;
  std::atomic<std::uint32_t> observations = 0;

  {
    flow::begin();
    auto external_revision = flow::external<std::uint32_t>("revision");
    auto stable_revision = flow::cutoff(external_revision);
    auto retained = flow::resource<TrackedResource>();
    flow::construct(retained, flow::read(stable_revision),
                    [&](TrackedResource *output, const std::uint32_t &value) {
                      constructions.fetch_add(1, std::memory_order_relaxed);
                      std::construct_at(output, &destructions, value);
                    });
    flow::op(flow::OpMetadata("Observe", "", flow::OpSemantics::side_effect),
             flow::read(retained), [&](const TrackedResource &value) {
               observations.fetch_add(value.value, std::memory_order_relaxed);
             });
    auto program = flow::end();
    program.bind_external("revision", &revision);

    strobe::sync::allocator allocator;
    strobe::sync::JobScheduler scheduler(&allocator, 2);
    submit_and_wait(program, scheduler);
    submit_and_wait(program, scheduler);
    EXPECT_EQ(constructions.load(std::memory_order_relaxed), 1);
    EXPECT_EQ(destructions.load(std::memory_order_relaxed), 0);

    revision = 2;
    submit_and_wait(program, scheduler);
    scheduler.stop();
    EXPECT_EQ(constructions.load(std::memory_order_relaxed), 2);
    EXPECT_EQ(destructions.load(std::memory_order_relaxed), 1);
    EXPECT_EQ(observations.load(std::memory_order_relaxed), 4);
  }

  EXPECT_EQ(destructions.load(std::memory_order_relaxed), 2);
}

TEST(FlowScenario, ConstructsDiscardedFeedbackStorageOncePerSlot) {
  std::atomic<std::uint32_t> initial_destructions = 0;
  std::atomic<std::uint32_t> slot_constructions = 0;
  std::atomic<std::uint32_t> slot_destructions = 0;
  std::atomic<std::uint32_t> writes = 0;
  std::atomic<std::uint32_t> observed_sum = 0;

  {
    flow::begin(flow::CompileOptions{.max_slots = 3});
    flow::acquire("frame");
    auto initial = flow::resource<ReusableValue>();
    flow::construct(initial, [&](ReusableValue *output) {
      std::construct_at(output, &initial_destructions, 0);
    });
    auto previous = flow::phi(initial);

    auto current = flow::resource<ReusableValue>();
    flow::construct(current, [&](ReusableValue *output) {
      slot_constructions.fetch_add(1, std::memory_order_relaxed);
      std::construct_at(output, &slot_destructions, 0);
    });
    flow::discard(current);
    flow::op(flow::read(previous), flow::write(current),
             [&](const ReusableValue &prior, ReusableValue &output) {
               output.value = prior.value + 1;
               writes.fetch_add(1, std::memory_order_relaxed);
             });
    flow::op(flow::OpMetadata("Observe", "", flow::OpSemantics::side_effect),
             flow::read(current), [&](const ReusableValue &value) {
               observed_sum.fetch_add(value.value, std::memory_order_relaxed);
             });
    flow::feedback(previous, current);
    auto program = flow::end();

    std::uint32_t preconstructed_slots = 0;
    for (std::size_t resource = 0;
         resource < program.data().resource_lifetimes.size(); ++resource) {
      if (program.data().resource_lifetimes[resource].preconstructed) {
        preconstructed_slots =
            program.data().resource_slots[resource].slot_count;
      }
    }
    ASSERT_EQ(preconstructed_slots, 3);
    program.release("frame", 6);
    strobe::sync::allocator allocator;
    strobe::sync::JobScheduler scheduler(&allocator, 4);
    submit_and_wait(program, scheduler);
    scheduler.stop();

    EXPECT_EQ(slot_constructions.load(std::memory_order_relaxed), 3);
    EXPECT_EQ(slot_destructions.load(std::memory_order_relaxed), 0);
    EXPECT_EQ(writes.load(std::memory_order_relaxed), 6);
    EXPECT_EQ(observed_sum.load(std::memory_order_relaxed), 21);
  }

  EXPECT_EQ(initial_destructions.load(std::memory_order_relaxed), 1);
  EXPECT_EQ(slot_destructions.load(std::memory_order_relaxed), 3);
}

} // namespace
