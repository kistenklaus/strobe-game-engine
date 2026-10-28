#include <strobe/flow/flow.hpp>

#include <strobe/core/sync/job_scheduler.hpp>

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <future>
#include <limits>
#include <thread>
#include <utility>

namespace flow = strobe::flow;

namespace {

flow::Lifetime external_construct_lifetime{};
bool external_copy_metadata_valid = false;

void external_copy(const flow::ExternalOpContext &context) noexcept {
  external_copy_metadata_valid =
      context.abi_version == 7 && context.argument_count == 2 &&
      context.argument_types[0] == flow::type_id<int>() &&
      context.argument_types[1] == flow::type_id<int>() &&
      context.argument_accesses[0] == flow::AccessMode::read &&
      context.argument_accesses[1] == flow::AccessMode::write;
  *static_cast<int *>(context.arguments[1]) =
      *static_cast<const int *>(context.arguments[0]) + 1;
}

void external_construct(const flow::ExternalOpContext &context) noexcept {
  external_construct_lifetime = context.lifetime;
  std::construct_at(static_cast<int *>(context.arguments[0]),
                    *static_cast<const int *>(context.arguments[1]) + 2);
}

flow::ExternalOpFunction resolve_test_library(void *, std::string_view symbol,
                                              std::uint32_t abi) noexcept {
  if (symbol == "copy" && abi == 7) {
    return external_copy;
  }
  if (symbol == "construct" && abi == 3) {
    return external_construct;
  }
  return nullptr;
}

struct ExternalProjectionOwner {
  int value;
};

void *project_external_projection_owner(void *storage) noexcept {
  return &static_cast<ExternalProjectionOwner *>(storage)->value;
}

flow::ExternalProjectionFunction
resolve_test_projector(void *, std::string_view symbol,
                       std::uint32_t abi) noexcept {
  if (symbol == "value" && abi == 1) {
    return project_external_projection_owner;
  }
  return nullptr;
}

void submit_and_wait(flow::Program &program,
                     strobe::sync::JobScheduler &scheduler) {
  program.submit(&scheduler);
  program.wait();
}

TEST(FlowProgram, EndReturnsAProgramSummary) {
  flow::begin();
  auto external = flow::external<int>("external");
  auto value = flow::resource<int>();
  flow::construct(value, flow::read(external),
                  [](int *value, const int &external) {
                    std::construct_at(value, external);
                  });
  auto program = flow::end();

  EXPECT_EQ(program.operation_count(), 1);
  EXPECT_EQ(program.resource_count(), 2);
  EXPECT_EQ(program.external_count(), 1);
  ASSERT_EQ(program.data().resources.size(), 2);
  EXPECT_EQ(program.data().resources[0].slots.offset,
            flow::detail::null_storage_offset);
  EXPECT_EQ(program.data().resources[0].slots.slot_count, 0);
  EXPECT_EQ(program.data().resources[1].slots.slot_count, 1);
}

TEST(FlowProgram, PrettyPrintsCompiledGraphAndStoragePlan) {
  flow::begin();
  auto external = flow::external<int>("source");
  auto value = flow::resource<int>("value");
  flow::construct(
      value, flow::OpMetadata("Create value"), flow::read(external),
      [](int *output, const int &input) { std::construct_at(output, input); });
  flow::op(flow::OpMetadata("Observe value"), flow::read(value),
           [](const int &) {});
  auto program = flow::end();

  const auto pretty = flow::detail::to_pretty(program.data());
  EXPECT_NE(pretty.view().find("storage: size="), std::string_view::npos);
  EXPECT_NE(pretty.view().find("%0 = external \"source\""),
            std::string_view::npos);
  EXPECT_NE(pretty.view().find("resources:"), std::string_view::npos);
  EXPECT_NE(pretty.view().find("%1 \"value\" : int"), std::string_view::npos);
  EXPECT_NE(pretty.view().find("nodes:"), std::string_view::npos);
  EXPECT_NE(pretty.view().find("= construct r:%0.0"), std::string_view::npos);
  EXPECT_NE(pretty.view().find("[\"Create value\"]"), std::string_view::npos);
  EXPECT_NE(pretty.view().find("deps={@0}"), std::string_view::npos);
}

TEST(FlowProgram, SuppliesLogicalLifetimeToConstructionCallback) {
  flow::Lifetime observed{.begin = 99, .end = 99};

  flow::begin();
  auto value = flow::resource<int>();
  flow::construct(value, [&](int *storage, flow::Lifetime lifetime) {
    observed = lifetime;
    std::construct_at(storage, 42);
  });
  flow::op(flow::read(value), [](const int &) {});
  auto program = flow::end();

  ASSERT_EQ(program.data().resources.size(), 1);
  const auto compiled = program.data().resources[0].lifetime.interval;
  EXPECT_EQ(compiled.begin, 0);
  EXPECT_EQ(compiled.end, 2);

  strobe::sync::allocator allocator;
  strobe::sync::JobScheduler scheduler(&allocator, 2);
  submit_and_wait(program, scheduler);
  scheduler.stop();

  EXPECT_EQ(observed.begin, compiled.begin);
  EXPECT_EQ(observed.end, compiled.end);
}

TEST(FlowProgram, ResolvesAndRunsExternalOp) {
  external_copy_metadata_valid = false;
  int observed = 0;

  flow::begin();
  auto source = flow::resource<int>();
  auto target = flow::resource<int>();
  flow::construct(source, 41);
  flow::construct(target, 0);
  flow::op(flow::external_op("test-library", "copy", 7,
                             flow::OpMetadata("Copy", "Copies one integer")),
           flow::read(source), flow::write(target));
  flow::op(flow::read(target), [&](const int &value) { observed = value; });
  auto program = flow::end();

  ASSERT_EQ(program.data().callbacks.size(), 4);
  const auto &compiled = program.data().callbacks[2];
  ASSERT_TRUE(compiled.external.has_value());
  EXPECT_EQ(compiled.external->library_id.view(), "test-library");
  EXPECT_EQ(compiled.external->symbol.view(), "copy");
  EXPECT_EQ(compiled.metadata.title.view(), "Copy");
  EXPECT_EQ(compiled.argument_types[0], flow::type_id<int>());

  program.bind_library(
      "test-library",
      flow::Library{.state = nullptr, .resolve = resolve_test_library});
  EXPECT_TRUE(program.libraries_bound());
  strobe::sync::allocator allocator;
  strobe::sync::JobScheduler scheduler(&allocator, 2);
  submit_and_wait(program, scheduler);
  scheduler.stop();

  EXPECT_TRUE(external_copy_metadata_valid);
  EXPECT_EQ(observed, 42);
}

TEST(FlowProgram, RetainsMetadataForLambdaOperationsAndConstruction) {
  flow::begin();
  auto value = flow::resource<int>();
  flow::construct(value, flow::OpMetadata("Create value"), 1);
  flow::op(flow::OpMetadata("Observe value", "Runtime-only observer",
                            flow::OpSemantics::side_effect),
           flow::read(value), [](const int &) {});
  auto program = flow::end();

  ASSERT_EQ(program.data().callbacks.size(), 2);
  EXPECT_EQ(program.data().callbacks[0].metadata.title.view(), "Create value");
  EXPECT_EQ(program.data().callbacks[1].metadata.title.view(), "Observe value");
  EXPECT_EQ(program.data().callbacks[1].metadata.description.view(),
            "Runtime-only observer");
  EXPECT_FALSE(program.data().callbacks[1].external.has_value());
  EXPECT_TRUE(program.data().callbacks[1].runtime);
}

TEST(FlowProgram, SuppliesLifetimeToExternalConstruction) {
  external_construct_lifetime = {};
  int observed = 0;

  flow::begin();
  auto source = flow::resource<int>();
  auto target = flow::resource<int>();
  flow::construct(source, 40);
  flow::construct(target, flow::external_op("test-library", "construct", 3),
                  flow::read(source));
  flow::op(flow::read(target), [&](const int &value) { observed = value; });
  auto program = flow::end();
  const auto expected = program.data().resources[1].lifetime.interval;

  program.bind_library(
      "test-library",
      flow::Library{.state = nullptr, .resolve = resolve_test_library});
  strobe::sync::allocator allocator;
  strobe::sync::JobScheduler scheduler(&allocator, 2);
  submit_and_wait(program, scheduler);
  scheduler.stop();

  EXPECT_EQ(observed, 42);
  EXPECT_EQ(external_construct_lifetime.begin, expected.begin);
  EXPECT_EQ(external_construct_lifetime.end, expected.end);
}

TEST(FlowProgram, RequiresExternalOpLibraryBeforeSubmission) {
  flow::begin();
  auto value = flow::resource<int>();
  flow::construct(value, 0);
  flow::op(flow::external_op("missing", "copy"), flow::read(value));
  auto program = flow::end();

  strobe::sync::allocator allocator;
  strobe::sync::JobScheduler scheduler(&allocator, 2);
  EXPECT_THROW(program.submit(&scheduler), std::logic_error);
  scheduler.stop();
}

TEST(FlowProgram, FailedLibraryBindingDoesNotPartiallyResolveOperations) {
  flow::begin();
  flow::op(flow::external_op("test-library", "copy", 7));
  flow::op(flow::external_op("test-library", "missing", 7));
  auto program = flow::end();

  EXPECT_FALSE(program.libraries_bound());
  EXPECT_THROW(
      program.bind_library(
          "test-library",
          flow::Library{.state = nullptr, .resolve = resolve_test_library}),
      std::invalid_argument);
  EXPECT_FALSE(program.libraries_bound());
  EXPECT_FALSE(program.data().callbacks[0].runtime);
  EXPECT_FALSE(program.data().callbacks[1].runtime);
}

TEST(FlowProgram, AliasesStorageForOrderedNonOverlappingResources) {
  flow::begin();
  auto first = flow::resource<int>();
  auto bridge = flow::resource<int>();
  auto last = flow::resource<int>();
  flow::construct(first, 1);
  flow::construct(bridge, flow::read(first), [](int *bridge, const int &first) {
    std::construct_at(bridge, first);
  });
  flow::construct(last, flow::read(bridge), [](int *last, const int &bridge) {
    std::construct_at(last, bridge);
  });
  auto program = flow::end();

  const auto &resources = program.data().resources;
  ASSERT_EQ(resources.size(), 3);
  EXPECT_EQ(resources[0].slots.offset, resources[2].slots.offset);
  EXPECT_NE(resources[0].slots.offset, resources[1].slots.offset);
  EXPECT_EQ(program.data().storage_size, sizeof(int) * 2);
  EXPECT_EQ(program.data().storage_alignment, alignof(int));

  const auto &physical_offsets =
      program.data().physical_storage.resource_slot_offsets;
  const auto &physical_slots =
      program.data().physical_storage.slot_physical_ids;
  ASSERT_EQ(physical_offsets.size(), 4);
  ASSERT_EQ(physical_slots.size(), 3);
  EXPECT_EQ(physical_slots[physical_offsets[0]],
            physical_slots[physical_offsets[2]]);
  EXPECT_NE(physical_slots[physical_offsets[0]],
            physical_slots[physical_offsets[1]]);
  ASSERT_FALSE(program.data().physical_storage.frontier_offsets.empty());
}

TEST(FlowProgram, SeparatesInterferingStorage) {
  flow::begin();
  auto first = flow::resource<int>();
  auto second = flow::resource<int>();
  flow::construct(first, 1);
  flow::construct(second, 2);
  auto program = flow::end();

  const auto &resources = program.data().resources;
  ASSERT_EQ(resources.size(), 2);
  EXPECT_NE(resources[0].slots.offset, resources[1].slots.offset);
  EXPECT_EQ(program.data().storage_size, sizeof(int) * 2);
}

TEST(FlowProgram, AppliesBackpressureAcrossAliasedResources) {
  flow::begin();
  auto first = flow::resource<int>();
  auto bridge = flow::resource<int>();
  auto last = flow::resource<int>();
  flow::construct(first, 1);
  flow::construct(bridge, flow::read(first), [](int *bridge, const int &first) {
    std::construct_at(bridge, first);
  });
  flow::construct(last, flow::read(bridge), [](int *last, const int &bridge) {
    std::construct_at(last, bridge);
  });
  auto program = flow::end();

  const auto first_generation = program.acquire_generation(0);
  EXPECT_FALSE(program.try_acquire_generation(2).has_value());
  const auto bridge_generation = program.try_acquire_generation(1);
  ASSERT_TRUE(bridge_generation.has_value());
  program.release_generation(first_generation);
  const auto last_generation = program.acquire_generation(2);
  program.release_generation(*bridge_generation);
  program.release_generation(last_generation);
}

TEST(FlowProgram, ReservesAnAlignedRingForYieldingCycles) {
  struct alignas(64) Value {
    int value;
  };

  flow::begin();
  auto initial = flow::resource<Value>();
  flow::construct(initial, Value{0});
  auto state = flow::phi(initial);
  flow::acquire("step");
  auto next = flow::resource<Value>();
  flow::construct(next, flow::read(state), [](Value *next, const Value &state) {
    std::construct_at(next, Value{state.value + 1});
  });
  flow::yield();
  flow::feedback(state, next, flow::FeedbackOptions{.max_in_flight = 3});
  auto program = flow::end();

  const auto &slot = program.data().resources[2].slots;
  EXPECT_EQ(slot.slot_count, 3);
  EXPECT_EQ(slot.slot_stride, sizeof(Value));
  EXPECT_EQ(slot.offset % alignof(Value), 0);
  EXPECT_TRUE(slot.bounded_overlap);
  EXPECT_EQ(program.data().storage_alignment, alignof(Value));
}

TEST(FlowProgram, SchedulesDestructionAfterTheCompleteLastUseFrontier) {
  flow::begin();
  auto value = flow::resource<int>();
  flow::construct(value, 1);
  flow::op(flow::read(value), [](const int &) {});
  flow::op(flow::read(value), [](const int &) {});
  auto program = flow::end();

  const auto &data = program.data();
  ASSERT_EQ(data.resources.size(), 1);
  const auto &lifetime = data.resources[0].lifetime;
  ASSERT_EQ(lifetime.constructions.count, 1);
  EXPECT_EQ(data.constructions[lifetime.constructions.offset], 0);
  ASSERT_EQ(lifetime.release_after.count, 2);
  EXPECT_EQ(data.release_after[lifetime.release_after.offset], 1);
  EXPECT_EQ(data.release_after[lifetime.release_after.offset + 1], 2);
  EXPECT_NE(lifetime.destroy, nullptr);
  EXPECT_FALSE(lifetime.destroy_at_shutdown);
}

TEST(FlowProgram, EmitsDenseRuntimeDependenciesAndValueBindings) {
  flow::begin();
  auto first = flow::resource<int>();
  auto second = flow::resource<int>();
  flow::construct(first, 7);
  flow::construct(second, flow::read(first), [](int *second, const int &first) {
    std::construct_at(second, first + 1);
  });
  auto program = flow::end();

  const auto &data = program.data();
  ASSERT_EQ(data.nodes.size(), 2);
  ASSERT_EQ(data.values.size(), 2);
  EXPECT_EQ(data.values[0].resource, 0);
  EXPECT_EQ(data.values[0].version, 0);
  EXPECT_EQ(data.values[1].resource, 1);
  EXPECT_EQ(data.nodes[0].predecessors.count, 0);
  ASSERT_EQ(data.nodes[1].predecessors.count, 1);
  EXPECT_EQ(data.node_storage.predecessors[data.nodes[1].predecessors.offset],
            0);
  ASSERT_EQ(data.nodes[0].outputs.count, 1);
  ASSERT_EQ(data.nodes[1].inputs.count, 1);
  EXPECT_EQ(data.node_storage.outputs[data.nodes[0].outputs.offset],
            data.node_storage.inputs[data.nodes[1].inputs.offset]);
  EXPECT_EQ(data.callbacks.size(), 2);
}

TEST(FlowProgram, EmitsFeedbackAsTemporalRuntimeDependencies) {
  flow::begin();
  auto initial = flow::resource<int>();
  flow::construct(initial, 0);
  auto state = flow::phi(initial);
  flow::acquire("step");
  auto next = flow::resource<int>();
  flow::construct(next, flow::read(state), [](int *next, const int &state) {
    std::construct_at(next, state + 1);
  });
  flow::yield();
  flow::feedback(state, next);
  auto program = flow::end();

  const auto &data = program.data();
  ASSERT_EQ(data.nodes.size(), 5);
  ASSERT_EQ(data.feedback_regions.size(), 1);
  ASSERT_EQ(data.feedback_regions[0].phis.count, 1);
  EXPECT_GT(data.feedback_regions[0].nodes.count, 0);
  const auto &region = data.feedback_regions[0];
  const auto first_region_node =
      data.feedback_region_nodes[region.nodes.offset];
  for (std::uint32_t index = 0; index < region.nodes.count; ++index) {
    EXPECT_EQ(data.feedback_region_nodes[region.nodes.offset + index],
              first_region_node + index);
  }
  ASSERT_EQ(data.feedback_regions[0].acquires.count, 1);
  EXPECT_EQ(data.feedback_region_phis[data.feedback_regions[0].phis.offset], 0);
  EXPECT_EQ(
      data.feedback_region_acquires[data.feedback_regions[0].acquires.offset],
      0);
  ASSERT_EQ(data.resources.size(), 3);
  EXPECT_EQ(data.resources[0].lifetime.interval.begin, 0);
  EXPECT_EQ(data.resources[0].lifetime.interval.end,
            std::numeric_limits<std::uint32_t>::max());
  EXPECT_EQ(data.resources[2].lifetime.interval.begin, 0);
  EXPECT_EQ(data.resources[2].lifetime.interval.end,
            std::numeric_limits<std::uint32_t>::max());
  ASSERT_EQ(data.node_storage.temporal_predecessors.size(), 1);
  ASSERT_EQ(data.node_storage.temporal_successors.size(), 1);
  flow::detail::NodeId phi = flow::detail::null_node_id;
  for (flow::detail::NodeId node = 0; node < data.nodes.size(); ++node) {
    if (data.nodes[node].kind == flow::detail::OperationKind::phi) {
      phi = node;
      EXPECT_EQ(data.nodes[node].callback, flow::detail::null_callback_id);
    }
    for (std::uint32_t index = 0; index < data.nodes[node].predecessors.count;
         ++index) {
      EXPECT_LT(data.node_storage
                    .predecessors[data.nodes[node].predecessors.offset + index],
                node);
    }
  }
  ASSERT_NE(phi, flow::detail::null_node_id);
  ASSERT_EQ(data.nodes[phi].temporal_predecessors.count, 1);
  const auto feedback =
      data.node_storage
          .temporal_predecessors[data.nodes[phi].temporal_predecessors.offset];
  ASSERT_EQ(data.nodes[feedback].temporal_successors.count, 1);
  EXPECT_EQ(
      data.node_storage
          .temporal_successors[data.nodes[feedback].temporal_successors.offset],
      phi);
  ASSERT_EQ(data.yields.size(), 1);
  EXPECT_EQ(data.nodes[data.yields[0]].kind,
            flow::detail::OperationKind::yield);
}

TEST(FlowProgram, CompilesIndependentFeedbackRegions) {
  flow::begin();

  auto initial_a = flow::resource<int>();
  flow::construct(initial_a, 0);
  auto state_a = flow::phi(initial_a);
  flow::acquire("a");
  auto next_a = flow::resource<int>();
  flow::construct(next_a, 0);
  flow::op(flow::read(state_a), flow::write(next_a),
           [](const int &input, int &output) { output = input + 1; });
  flow::feedback(state_a, next_a);

  auto initial_b = flow::resource<int>();
  flow::construct(initial_b, 0);
  auto state_b = flow::phi(initial_b);
  flow::acquire("b");
  auto next_b = flow::resource<int>();
  flow::construct(next_b, 0);
  flow::op(flow::read(state_b), flow::write(next_b),
           [](const int &input, int &output) { output = input + 1; });
  flow::feedback(state_b, next_b);

  const auto program = flow::end();
  const auto &data = program.data();
  ASSERT_EQ(data.feedback_regions.size(), 2);
  ASSERT_EQ(data.feedback_regions[0].acquires.count, 1);
  ASSERT_EQ(data.feedback_regions[1].acquires.count, 1);
  ASSERT_EQ(data.feedback_regions[0].token_requirements.count, 1);
  ASSERT_EQ(data.feedback_regions[1].token_requirements.count, 1);
  const auto acquire_a =
      data.feedback_region_acquires[data.feedback_regions[0].acquires.offset];
  const auto acquire_b =
      data.feedback_region_acquires[data.feedback_regions[1].acquires.offset];
  EXPECT_NE(acquire_a, acquire_b);
  EXPECT_NE(data.acquires[acquire_a].semaphore,
            data.acquires[acquire_b].semaphore);
  const auto &requirement_a =
      data.feedback_token_requirements[data.feedback_regions[0]
                                           .token_requirements.offset];
  const auto &requirement_b =
      data.feedback_token_requirements[data.feedback_regions[1]
                                           .token_requirements.offset];
  EXPECT_EQ(requirement_a.semaphore, data.acquires[acquire_a].semaphore);
  EXPECT_EQ(requirement_b.semaphore, data.acquires[acquire_b].semaphore);
  EXPECT_EQ(requirement_a.count, 1);
  EXPECT_EQ(requirement_b.count, 1);
  EXPECT_GT(data.feedback_regions[0].nodes.count, 0);
  EXPECT_GT(data.feedback_regions[1].nodes.count, 0);
  for (const auto &region : data.feedback_regions) {
    const auto first_region_node =
        data.feedback_region_nodes[region.nodes.offset];
    for (std::uint32_t index = 0; index < region.nodes.count; ++index) {
      EXPECT_EQ(data.feedback_region_nodes[region.nodes.offset + index],
                first_region_node + index);
    }
  }
}

TEST(FlowProgram, CompilesFeedbackSpecificInFlightLimit) {
  flow::begin();
  auto initial = flow::resource<int>();
  flow::construct(initial, 0);
  auto state = flow::phi(initial);
  flow::acquire("frame");
  auto next = flow::resource<int>();
  flow::construct(next, 0);
  flow::op(flow::read(state), flow::write(next),
           [](const int &input, int &output) { output = input + 1; });
  flow::feedback(state, next, flow::FeedbackOptions{.max_in_flight = 5});

  const auto program = flow::end();
  ASSERT_EQ(program.data().feedback_regions.size(), 1);
  EXPECT_EQ(program.data().feedback_regions[0].max_in_flight, 5);
  EXPECT_EQ(program.data().activation_layout.capacity, 5);
}

TEST(FlowProgram, AdvancesIndependentFeedbackFrontiers) {
  std::atomic<std::uint32_t> runs_a = 0;
  std::atomic<std::uint32_t> runs_b = 0;
  flow::begin();

  auto initial_a = flow::resource<int>();
  flow::construct(initial_a, 0);
  auto state_a = flow::phi(initial_a);
  flow::acquire("a");
  auto next_a = flow::resource<int>();
  flow::construct(next_a, 0);
  flow::op(flow::read(state_a), flow::write(next_a),
           [&](const int &input, int &output) {
             output = input + 1;
             runs_a.fetch_add(1, std::memory_order_relaxed);
           });
  flow::feedback(state_a, next_a);

  auto initial_b = flow::resource<int>();
  flow::construct(initial_b, 0);
  auto state_b = flow::phi(initial_b);
  flow::acquire("b");
  auto next_b = flow::resource<int>();
  flow::construct(next_b, 0);
  flow::op(flow::read(state_b), flow::write(next_b),
           [&](const int &input, int &output) {
             output = input + 1;
             runs_b.fetch_add(1, std::memory_order_relaxed);
           });
  flow::feedback(state_b, next_b);

  auto program = flow::end();
  strobe::sync::allocator allocator;
  strobe::sync::JobScheduler scheduler(&allocator, 2);

  program.release("a", 2);
  submit_and_wait(program, scheduler);
  EXPECT_EQ(runs_a.load(std::memory_order_relaxed), 2);
  EXPECT_EQ(runs_b.load(std::memory_order_relaxed), 0);

  program.release("b");
  submit_and_wait(program, scheduler);
  scheduler.stop();
  EXPECT_EQ(runs_a.load(std::memory_order_relaxed), 2);
  EXPECT_EQ(runs_b.load(std::memory_order_relaxed), 1);
}

TEST(FlowProgram, PreservesSingleFeedbackFrontierAcrossSubmissions) {
  std::atomic<std::uint32_t> runs = 0;
  flow::begin();
  auto initial = flow::resource<int>();
  flow::construct(initial, 0);
  auto state = flow::phi(initial);
  flow::acquire("frame");
  auto next = flow::resource<int>();
  flow::construct(next, 0);
  flow::op(flow::read(state), flow::write(next),
           [&](const int &input, int &output) {
             output = input + 1;
             runs.fetch_add(1, std::memory_order_relaxed);
           });
  flow::feedback(state, next);

  auto program = flow::end();
  strobe::sync::allocator allocator;
  strobe::sync::JobScheduler scheduler(&allocator, 2);

  program.release("frame", 2);
  program.submit(&scheduler);
  program.release("frame", 2);
  program.submit(&scheduler);
  program.wait();
  scheduler.stop();

  EXPECT_EQ(runs.load(std::memory_order_relaxed), 4);
}

TEST(FlowProgram, SerializesResourcesAcrossFeedbackActivations) {
  std::atomic<std::uint32_t> entered = 0;
  std::atomic<std::uint32_t> active = 0;
  std::atomic<std::uint32_t> max_active = 0;
  std::atomic<bool> release = false;

  flow::begin();
  auto domain = flow::resource<int>();
  flow::construct(domain, 0);
  auto initial = flow::resource<int>();
  flow::construct(initial, 0);
  auto state = flow::phi(initial);
  flow::acquire("frame");
  flow::op(flow::serialize(domain), [&] {
    const auto current = active.fetch_add(1, std::memory_order_relaxed) + 1;
    auto maximum = max_active.load(std::memory_order_relaxed);
    while (maximum < current &&
           !max_active.compare_exchange_weak(maximum, current,
                                             std::memory_order_relaxed)) {
    }
    entered.fetch_add(1, std::memory_order_release);
    while (!release.load(std::memory_order_acquire)) {
      std::this_thread::yield();
    }
    active.fetch_sub(1, std::memory_order_relaxed);
  });
  auto next = flow::resource<int>();
  flow::construct(next, 0);
  flow::op(flow::read(state), flow::write(next),
           [](const int &input, int &output) { output = input + 1; });
  flow::feedback(state, next, flow::FeedbackOptions{.max_in_flight = 2});
  auto program = flow::end();

  strobe::sync::allocator allocator;
  strobe::sync::JobScheduler scheduler(&allocator, 2);
  program.release("frame", 2);
  program.submit(&scheduler);
  const auto deadline =
      std::chrono::steady_clock::now() + std::chrono::seconds(1);
  while (entered.load(std::memory_order_acquire) == 0 &&
         std::chrono::steady_clock::now() < deadline) {
    std::this_thread::yield();
  }
  EXPECT_EQ(entered.load(std::memory_order_acquire), 1);
  EXPECT_EQ(max_active.load(std::memory_order_relaxed), 1);
  release.store(true, std::memory_order_release);
  program.wait();
  scheduler.stop();
}

TEST(FlowProgram, BindsExternalResourcesByCompiledName) {
  int window = 7;
  flow::begin();
  auto external = flow::external<int>("window");
  flow::op(flow::read(external), [](const int &) {});
  auto program = flow::end();

  ASSERT_EQ(program.data().externals.size(), 1);
  EXPECT_EQ(program.data().externals[0].name.view(), "window");
  EXPECT_FALSE(program.externals_bound());
  EXPECT_THROW(program.external(0), std::logic_error);
  EXPECT_THROW(program.bind_external("missing", &window),
               std::invalid_argument);
  float wrong_type = 0.0F;
  EXPECT_THROW(program.bind_external("window", &wrong_type),
               std::invalid_argument);
  EXPECT_THROW(program.bind_external("window", static_cast<void *>(nullptr)),
               std::invalid_argument);
  program.bind_external("window", &window);
  EXPECT_TRUE(program.externals_bound());
  EXPECT_EQ(program.external(0), &window);
}

TEST(FlowProgram, AcquiresAlignedAndReusesGenerations) {
  struct alignas(64) Value {
    int value;
  };

  flow::begin();
  auto initial = flow::resource<Value>();
  flow::construct(initial, Value{0});
  auto state = flow::phi(initial);
  flow::acquire("step");
  auto next = flow::resource<Value>();
  flow::construct(next, flow::read(state), [](Value *next, const Value &state) {
    std::construct_at(next, Value{state.value + 1});
  });
  flow::yield();
  flow::feedback(state, next);
  auto program = flow::end();

  const auto first = program.acquire_generation(2);
  const auto second = program.acquire_generation(2);
  EXPECT_NE(first.slot, second.slot);
  EXPECT_EQ(reinterpret_cast<std::uintptr_t>(program.resolve(first)) % 64, 0);
  EXPECT_EQ(reinterpret_cast<std::uintptr_t>(program.resolve(second)) % 64, 0);
  EXPECT_FALSE(program.try_acquire_generation(2).has_value());

  program.release_generation(first);
  const auto reused = program.acquire_generation(2);
  EXPECT_EQ(reused.slot, first.slot);
  EXPECT_NE(reused.generation, first.generation);
  program.release_generation(second);
  program.release_generation(reused);
}

TEST(FlowProgram, DestroysConstructedGenerationsOnReleaseAndShutdown) {
  struct Value {
    std::atomic<std::uint32_t> *destructions;
    ~Value() { destructions->fetch_add(1, std::memory_order_relaxed); }
  };

  std::atomic<std::uint32_t> destructions = 0;
  {
    flow::begin();
    auto value = flow::resource<Value>();
    flow::construct(value, &destructions);
    auto program = flow::end();

    auto first = program.acquire_generation(0);
    std::construct_at(static_cast<Value *>(program.resolve(first)),
                      &destructions);
    program.mark_constructed(first);
    program.release_generation(first);
    EXPECT_EQ(destructions.load(std::memory_order_relaxed), 1);

    auto second = program.acquire_generation(0);
    std::construct_at(static_cast<Value *>(program.resolve(second)),
                      &destructions);
    program.mark_constructed(second);
  }
  EXPECT_EQ(destructions.load(std::memory_order_relaxed), 2);
}

TEST(FlowProgram, BlockingAcquisitionResumesWhenASlotIsReleased) {
  flow::begin();
  auto value = flow::resource<int>();
  flow::construct(value, 1);
  auto program = flow::end();
  const auto first = program.acquire_generation(0);

  auto waiter = std::async(std::launch::async,
                           [&] { return program.acquire_generation(0); });
  EXPECT_EQ(waiter.wait_for(std::chrono::milliseconds(10)),
            std::future_status::timeout);
  program.release_generation(first);
  const auto second = waiter.get();
  EXPECT_NE(second.generation, first.generation);
  program.release_generation(second);
}

TEST(FlowProgram, ExecutesConstructionAndPassesResourceArguments) {
  std::atomic<int> observed = 0;
  flow::begin();
  auto source = flow::resource<int>();
  auto target = flow::resource<int>();
  flow::construct(source, 20);
  flow::construct(target, flow::read(source),
                  [](int *target, const int &source) {
                    std::construct_at(target, source + 1);
                  });
  flow::op(flow::read_write(target), [](int &target) { target *= 2; });
  flow::op(flow::read(target), [&](const int &target) {
    observed.store(target, std::memory_order_relaxed);
  });
  auto program = flow::end();

  strobe::sync::allocator allocator;
  strobe::sync::JobScheduler scheduler(&allocator, 2);
  program.submit(&scheduler);
  program.wait();
  scheduler.stop();
  EXPECT_EQ(observed.load(std::memory_order_relaxed), 42);
}

TEST(FlowProgram, ReusesAliasedStorageDuringExecution) {
  std::atomic<int> observed = 0;
  flow::begin();
  auto first = flow::resource<int>();
  auto bridge = flow::resource<int>();
  auto last = flow::resource<int>();
  flow::construct(first, 5);
  flow::construct(bridge, flow::read(first), [](int *bridge, const int &first) {
    std::construct_at(bridge, first + 1);
  });
  flow::construct(last, flow::read(bridge), [](int *last, const int &bridge) {
    std::construct_at(last, bridge + 1);
  });
  flow::op(flow::read(last), [&](const int &last) {
    observed.store(last, std::memory_order_relaxed);
  });
  auto program = flow::end();
  ASSERT_EQ(program.data().resources[0].slots.offset,
            program.data().resources[2].slots.offset);

  strobe::sync::allocator allocator;
  strobe::sync::JobScheduler scheduler(&allocator, 2);
  program.submit(&scheduler);
  program.wait();
  scheduler.stop();
  EXPECT_EQ(observed.load(std::memory_order_relaxed), 7);
}

TEST(FlowProgram, ExecutesWithBoundExternalResources) {
  int external = 8;
  std::atomic<int> observed = 0;
  flow::begin();
  auto input = flow::external<int>("input");
  auto output = flow::resource<int>();
  flow::construct(output, flow::read(input), [](int *output, const int &input) {
    std::construct_at(output, input + 3);
  });
  flow::op(flow::read(output), [&](const int &output) {
    observed.store(output, std::memory_order_relaxed);
  });
  auto program = flow::end();
  EXPECT_THROW(program.submit(nullptr), std::invalid_argument);

  strobe::sync::allocator allocator;
  strobe::sync::JobScheduler scheduler(&allocator, 2);
  EXPECT_THROW(program.submit(&scheduler), std::logic_error);
  program.bind_external("input", &external);
  program.submit(&scheduler);
  program.wait();
  scheduler.stop();
  EXPECT_EQ(observed.load(std::memory_order_relaxed), 11);
}

TEST(FlowProgram, DestroysObjectsAfterTheirRuntimeFrontiers) {
  struct Value {
    std::atomic<std::uint32_t> *destructions;
    ~Value() { destructions->fetch_add(1, std::memory_order_relaxed); }
  };

  std::atomic<std::uint32_t> destructions = 0;
  flow::begin();
  auto value = flow::resource<Value>();
  flow::construct(value, &destructions);
  flow::op(flow::read(value), [](const Value &) {});
  flow::op(flow::read(value), [](const Value &) {});
  auto program = flow::end();

  strobe::sync::allocator allocator;
  strobe::sync::JobScheduler scheduler(&allocator, 2);
  program.submit(&scheduler);
  program.wait();
  scheduler.stop();
  EXPECT_EQ(destructions.load(std::memory_order_relaxed), 1);
}

TEST(FlowProgram, ReportsCallbackFailuresWithoutConstructingTheOutput) {
  struct Value {
    std::atomic<std::uint32_t> *destructions;
    ~Value() { destructions->fetch_add(1, std::memory_order_relaxed); }
  };
  std::atomic<std::uint32_t> destructions = 0;

  flow::begin();
  auto value = flow::resource<Value>();
  flow::construct(value, [](Value *) { throw std::runtime_error("failure"); });
  auto program = flow::end();

  strobe::sync::allocator allocator;
  strobe::sync::JobScheduler scheduler(&allocator, 2);
  program.submit(&scheduler);
  EXPECT_THROW(program.wait(), std::runtime_error);
  scheduler.stop();
  EXPECT_EQ(destructions.load(std::memory_order_relaxed), 0);
}

TEST(FlowProgram, PhiSelectsInitialThenRetainedFeedbackValues) {
  std::atomic<std::uint32_t> invocation = 0;
  std::atomic<int> observed[3] = {};
  flow::begin();
  auto initial = flow::resource<int>();
  flow::construct(initial, 0);
  auto state = flow::phi(initial);
  flow::acquire("step");
  auto next = flow::resource<int>();
  flow::construct(next, flow::read(state), [](int *next, const int &state) {
    std::construct_at(next, state + 1);
  });
  flow::op(flow::read(next), [&](const int &next) {
    const auto index = invocation.fetch_add(1, std::memory_order_relaxed);
    observed[index].store(next, std::memory_order_relaxed);
  });
  flow::feedback(state, next);
  auto program = flow::end();

  strobe::sync::allocator allocator;
  strobe::sync::JobScheduler scheduler(&allocator, 2);
  for (std::uint32_t activation = 0; activation < 3; ++activation) {
    program.release("step");
    submit_and_wait(program, scheduler);
  }
  scheduler.stop();

  EXPECT_EQ(invocation.load(std::memory_order_relaxed), 3);
  EXPECT_EQ(observed[0].load(std::memory_order_relaxed), 1);
  EXPECT_EQ(observed[1].load(std::memory_order_relaxed), 2);
  EXPECT_EQ(observed[2].load(std::memory_order_relaxed), 3);
}

TEST(FlowProgram, WhileExecutesSequentiallyWithinOneSchedulerJob) {
  std::atomic<std::uint32_t> invocations = 0;
  std::atomic<int> final_value = 0;

  flow::begin();
  auto counter = flow::resource<int>();
  flow::construct(counter, 0);
  auto condition = flow::resource<bool>();
  flow::construct(condition, true);
  flow::while_(condition, [&] {
    flow::op(flow::read_write(counter), [](int &value) { ++value; });
    flow::op(flow::read(counter), flow::write(condition),
             [](const int &value, bool &repeat) { repeat = value < 4; });
    flow::op(flow::read(counter), [&](const int &value) {
      invocations.fetch_add(1, std::memory_order_relaxed);
      final_value.store(value, std::memory_order_relaxed);
    });
  });
  auto program = flow::end();

  ASSERT_EQ(program.data().controls.size(), 1);
  strobe::sync::allocator allocator;
  strobe::sync::JobScheduler scheduler(&allocator, 2);
  submit_and_wait(program, scheduler);
  scheduler.stop();

  EXPECT_EQ(invocations.load(std::memory_order_relaxed), 4);
  EXPECT_EQ(final_value.load(std::memory_order_relaxed), 4);
}

TEST(FlowProgram, WhileUsesPretestSemantics) {
  std::atomic<std::uint32_t> invocations = 0;
  flow::begin();
  auto condition = flow::resource<bool>();
  flow::construct(condition, false);
  flow::while_(condition, [&] {
    flow::op(flow::write(condition), [&](bool &repeat) {
      invocations.fetch_add(1, std::memory_order_relaxed);
      repeat = false;
    });
  });
  auto program = flow::end();

  strobe::sync::allocator allocator;
  strobe::sync::JobScheduler scheduler(&allocator, 2);
  submit_and_wait(program, scheduler);
  scheduler.stop();
  EXPECT_EQ(invocations.load(std::memory_order_relaxed), 0);
}

TEST(FlowProgram, ReconstructsAndDestroysResourcesInsideWhile) {
  struct Tracked {
    std::atomic<std::uint32_t> *destroyed;

    explicit Tracked(std::atomic<std::uint32_t> *constructed,
                     std::atomic<std::uint32_t> *destroyed)
        : destroyed(destroyed) {
      constructed->fetch_add(1, std::memory_order_relaxed);
    }

    ~Tracked() { destroyed->fetch_add(1, std::memory_order_relaxed); }
  };

  std::atomic<std::uint32_t> constructed = 0;
  std::atomic<std::uint32_t> destroyed = 0;
  strobe::sync::allocator allocator;
  strobe::sync::JobScheduler scheduler(&allocator, 2);
  {
    flow::begin();
    auto counter = flow::resource<int>();
    flow::construct(counter, 0);
    auto condition = flow::resource<bool>();
    flow::construct(condition, true);
    auto value = flow::resource<Tracked>();
    flow::while_(condition, [&] {
      flow::construct(value, &constructed, &destroyed);
      flow::op(flow::read(value), flow::read_write(counter),
               flow::write(condition),
               [](const Tracked &, int &count, bool &repeat) {
                 ++count;
                 repeat = count < 3;
               });
    });
    auto program = flow::end();
    submit_and_wait(program, scheduler);

    EXPECT_EQ(constructed.load(std::memory_order_relaxed), 3);
    EXPECT_EQ(destroyed.load(std::memory_order_relaxed), 2);
  }
  scheduler.stop();

  EXPECT_EQ(constructed.load(std::memory_order_relaxed), 3);
  EXPECT_EQ(destroyed.load(std::memory_order_relaxed), 3);
}

TEST(FlowProgram, ExecutesNestedWhileRegionsSequentially) {
  std::atomic<std::uint32_t> inner_invocations = 0;
  std::atomic<std::uint32_t> outer_invocations = 0;

  flow::begin();
  auto outer_counter = flow::resource<int>();
  flow::construct(outer_counter, 0);
  auto outer_condition = flow::resource<bool>();
  flow::construct(outer_condition, true);
  auto inner_counter = flow::resource<int>();
  flow::construct(inner_counter, 0);
  auto inner_condition = flow::resource<bool>();
  flow::construct(inner_condition, true);

  flow::while_(outer_condition, [&] {
    flow::op(flow::write(inner_counter), flow::write(inner_condition),
             [](int &counter, bool &repeat) {
               counter = 0;
               repeat = true;
             });
    flow::while_(inner_condition, [&] {
      flow::op(flow::read_write(inner_counter),
               [](int &counter) { ++counter; });
      flow::op(flow::read(inner_counter), flow::write(inner_condition),
               [](const int &counter, bool &repeat) { repeat = counter < 2; });
      flow::op(flow::read(inner_counter), [&](const int &) {
        inner_invocations.fetch_add(1, std::memory_order_relaxed);
      });
    });
    flow::op(flow::read_write(outer_counter), [](int &counter) { ++counter; });
    flow::op(flow::read(outer_counter), flow::write(outer_condition),
             [](const int &counter, bool &repeat) { repeat = counter < 3; });
    flow::op(flow::read(outer_counter), [&](const int &) {
      outer_invocations.fetch_add(1, std::memory_order_relaxed);
    });
  });
  auto program = flow::end();

  ASSERT_EQ(program.data().controls.size(), 2);
  strobe::sync::allocator allocator;
  strobe::sync::JobScheduler scheduler(&allocator, 2);
  submit_and_wait(program, scheduler);
  scheduler.stop();

  EXPECT_EQ(inner_invocations.load(std::memory_order_relaxed), 6);
  EXPECT_EQ(outer_invocations.load(std::memory_order_relaxed), 3);
}

TEST(FlowProgram, ExecutesIfInsideWhile) {
  std::atomic<std::uint32_t> even_invocations = 0;
  std::atomic<std::uint32_t> odd_invocations = 0;

  flow::begin();
  auto counter = flow::resource<int>();
  flow::construct(counter, 0);
  auto repeat = flow::resource<bool>();
  flow::construct(repeat, true);
  auto even = flow::resource<bool>();
  flow::construct(even, true);
  flow::while_(repeat, [&] {
    flow::op(flow::read_write(counter), flow::write(even),
             [](int &value, bool &condition) {
               ++value;
               condition = value % 2 == 0;
             });
    flow::if_(
        even,
        [&] {
          flow::op([&] {
            even_invocations.fetch_add(1, std::memory_order_relaxed);
          });
        },
        [&] {
          flow::op(
              [&] { odd_invocations.fetch_add(1, std::memory_order_relaxed); });
        });
    flow::op(flow::read(counter), flow::write(repeat),
             [](const int &value, bool &condition) { condition = value < 4; });
  });
  auto program = flow::end();

  strobe::sync::allocator allocator;
  strobe::sync::JobScheduler scheduler(&allocator, 2);
  submit_and_wait(program, scheduler);
  scheduler.stop();

  EXPECT_EQ(even_invocations.load(std::memory_order_relaxed), 2);
  EXPECT_EQ(odd_invocations.load(std::memory_order_relaxed), 2);
}

TEST(FlowProgram, IfExecutesOnlyTheSelectedBranch) {
  bool choose_then = true;
  std::atomic<std::uint32_t> then_invocations = 0;
  std::atomic<std::uint32_t> else_invocations = 0;
  std::atomic<int> observed = 0;

  flow::begin();
  auto condition = flow::external<bool>("condition");
  auto value = flow::resource<int>();
  flow::construct(value, 0);
  flow::if_(
      condition,
      [&] {
        flow::op(flow::write(value), [&](int &output) {
          output = 10;
          then_invocations.fetch_add(1, std::memory_order_relaxed);
        });
      },
      [&] {
        flow::op(flow::write(value), [&](int &output) {
          output = 20;
          else_invocations.fetch_add(1, std::memory_order_relaxed);
        });
      });
  flow::op(flow::read(value), [&](const int &output) {
    observed.store(output, std::memory_order_relaxed);
  });
  auto program = flow::end();
  program.bind_external("condition", &choose_then);

  strobe::sync::allocator allocator;
  strobe::sync::JobScheduler scheduler(&allocator, 2);
  submit_and_wait(program, scheduler);
  EXPECT_EQ(observed.load(std::memory_order_relaxed), 10);

  choose_then = false;
  submit_and_wait(program, scheduler);
  scheduler.stop();
  EXPECT_EQ(observed.load(std::memory_order_relaxed), 20);
  EXPECT_EQ(then_invocations.load(std::memory_order_relaxed), 1);
  EXPECT_EQ(else_invocations.load(std::memory_order_relaxed), 1);
}

TEST(FlowProgram, RunsWhileInsideAcquireDrivenFeedback) {
  std::atomic<std::uint32_t> inner_invocations = 0;
  std::atomic<std::uint32_t> outer_invocations = 0;

  flow::begin();
  auto outer_initial = flow::resource<int>();
  flow::construct(outer_initial, 0);
  auto inner_counter = flow::resource<int>();
  flow::construct(inner_counter, 0);
  auto inner_condition = flow::resource<bool>();
  flow::construct(inner_condition, true);
  auto outer_state = flow::phi(outer_initial);
  flow::acquire("frame");
  flow::op(flow::write(inner_counter), flow::write(inner_condition),
           [](int &counter, bool &repeat) {
             counter = 0;
             repeat = true;
           });
  flow::while_(inner_condition, [&] {
    flow::op(flow::read_write(inner_counter), [](int &counter) { ++counter; });
    flow::op(flow::read(inner_counter), flow::write(inner_condition),
             [](const int &counter, bool &repeat) { repeat = counter < 2; });
    flow::op(flow::read(inner_counter), [&](const int &) {
      inner_invocations.fetch_add(1, std::memory_order_relaxed);
    });
  });
  auto outer_next = flow::resource<int>();
  flow::construct(outer_next, flow::read(outer_state),
                  [](int *output, const int &input) {
                    std::construct_at(output, input + 1);
                  });
  flow::op(flow::read(outer_next), [&](const int &) {
    outer_invocations.fetch_add(1, std::memory_order_relaxed);
  });
  flow::feedback(outer_state, outer_next);
  auto program = flow::end();
  program.release("frame", 2);

  strobe::sync::allocator allocator;
  strobe::sync::JobScheduler scheduler(&allocator, 2);
  submit_and_wait(program, scheduler);
  scheduler.stop();

  EXPECT_EQ(inner_invocations.load(std::memory_order_relaxed), 4);
  EXPECT_EQ(outer_invocations.load(std::memory_order_relaxed), 2);
}

TEST(FlowProgram, CutoffSkipsMemoizedOperationsForUnchangedValues) {
  int external = 4;
  std::atomic<std::uint32_t> invocations = 0;
  std::atomic<int> observed = 0;
  flow::begin();
  auto input = flow::external<int>("input");
  auto changed = flow::cutoff(input);
  flow::op(flow::read(changed), [&](const int &value) {
    observed.store(value, std::memory_order_relaxed);
    invocations.fetch_add(1, std::memory_order_relaxed);
  });
  auto program = flow::end();
  program.bind_external("input", &external);

  strobe::sync::allocator allocator;
  strobe::sync::JobScheduler scheduler(&allocator, 2);
  const auto run = [&] { submit_and_wait(program, scheduler); };
  run();
  run();
  EXPECT_EQ(invocations.load(std::memory_order_relaxed), 1);
  EXPECT_EQ(observed.load(std::memory_order_relaxed), 4);

  external = 9;
  run();
  scheduler.stop();
  EXPECT_EQ(invocations.load(std::memory_order_relaxed), 2);
  EXPECT_EQ(observed.load(std::memory_order_relaxed), 9);
}

TEST(FlowProgram, SideEffectsRunWhenCutoffInputsAreUnchanged) {
  int external = 4;
  std::atomic<std::uint32_t> invocations = 0;

  flow::begin();
  auto input = flow::external<int>("input");
  auto changed = flow::cutoff(input);
  flow::op(flow::OpMetadata("Effect", "", flow::OpSemantics::side_effect),
           flow::read(changed), [&](const int &) {
             invocations.fetch_add(1, std::memory_order_relaxed);
           });
  auto program = flow::end();
  program.bind_external("input", &external);

  strobe::sync::allocator allocator;
  strobe::sync::JobScheduler scheduler(&allocator, 2);
  submit_and_wait(program, scheduler);
  submit_and_wait(program, scheduler);
  scheduler.stop();

  EXPECT_EQ(invocations.load(std::memory_order_relaxed), 2);
}

TEST(FlowProgram, CutoffMemoizesUsingAllInputs) {
  int initial = 0;
  int setting = 2;
  std::atomic<std::uint32_t> invocations = 0;
  std::atomic<int> observed = 0;

  flow::begin();
  auto initial_value = flow::external<int>("initial");
  auto external_setting = flow::external<int>("setting");
  auto stable_setting = flow::cutoff(external_setting);
  auto previous = flow::phi(initial_value);
  flow::acquire("step");
  auto current = flow::resource<int>();
  flow::construct(current, 0);
  flow::op(flow::read(previous), flow::read(stable_setting),
           flow::write(current),
           [&](const int &prior, const int &step, int &value) {
             value = prior + step;
             observed.store(value, std::memory_order_relaxed);
             invocations.fetch_add(1, std::memory_order_relaxed);
           });
  flow::feedback(previous, current);
  auto program = flow::end();
  program.bind_external("initial", &initial);
  program.bind_external("setting", &setting);

  strobe::sync::allocator allocator;
  strobe::sync::JobScheduler scheduler(&allocator, 2);
  program.release("step");
  submit_and_wait(program, scheduler);
  program.release("step");
  submit_and_wait(program, scheduler);
  program.release("step");
  submit_and_wait(program, scheduler);
  scheduler.stop();

  EXPECT_EQ(invocations.load(std::memory_order_relaxed), 3);
  EXPECT_EQ(observed.load(std::memory_order_relaxed), 6);
}

TEST(FlowProgram, CutoffRunsWhenEitherInputChanges) {
  int left = 2;
  int right = 3;
  std::atomic<std::uint32_t> invocations = 0;
  std::atomic<int> observed = 0;

  flow::begin();
  auto stable_left = flow::cutoff(flow::external<int>("left"));
  auto stable_right = flow::cutoff(flow::external<int>("right"));
  flow::op(flow::read(stable_left), flow::read(stable_right),
           [&](const int &first, const int &second) {
             observed.store(first + second, std::memory_order_relaxed);
             invocations.fetch_add(1, std::memory_order_relaxed);
           });
  auto program = flow::end();
  program.bind_external("left", &left);
  program.bind_external("right", &right);

  strobe::sync::allocator allocator;
  strobe::sync::JobScheduler scheduler(&allocator, 2);
  submit_and_wait(program, scheduler);
  submit_and_wait(program, scheduler);
  EXPECT_EQ(invocations.load(std::memory_order_relaxed), 1);

  left = 5;
  submit_and_wait(program, scheduler);
  EXPECT_EQ(invocations.load(std::memory_order_relaxed), 2);
  EXPECT_EQ(observed.load(std::memory_order_relaxed), 8);

  right = 7;
  submit_and_wait(program, scheduler);
  scheduler.stop();
  EXPECT_EQ(invocations.load(std::memory_order_relaxed), 3);
  EXPECT_EQ(observed.load(std::memory_order_relaxed), 12);
}

TEST(FlowProgram, CutoffMemoizesReadWriteOperations) {
  int input = 2;
  std::atomic<std::uint32_t> mutations = 0;
  std::atomic<std::uint32_t> observations = 0;
  std::atomic<int> observed = 0;

  flow::begin();
  auto stable = flow::cutoff(flow::external<int>("input"));
  auto value = flow::resource<int>();
  flow::construct(value, 1);
  flow::op(flow::read(stable), flow::read_write(value),
           [&](const int &increment, int &target) {
             target += increment;
             mutations.fetch_add(1, std::memory_order_relaxed);
           });
  flow::op(flow::read(value), [&](const int &target) {
    observed.store(target, std::memory_order_relaxed);
    observations.fetch_add(1, std::memory_order_relaxed);
  });
  auto program = flow::end();
  program.bind_external("input", &input);

  strobe::sync::allocator allocator;
  strobe::sync::JobScheduler scheduler(&allocator, 2);
  submit_and_wait(program, scheduler);
  submit_and_wait(program, scheduler);
  EXPECT_EQ(mutations.load(std::memory_order_relaxed), 1);
  EXPECT_EQ(observations.load(std::memory_order_relaxed), 1);
  EXPECT_EQ(observed.load(std::memory_order_relaxed), 3);

  input = 4;
  submit_and_wait(program, scheduler);
  scheduler.stop();
  EXPECT_EQ(mutations.load(std::memory_order_relaxed), 2);
  EXPECT_EQ(observations.load(std::memory_order_relaxed), 2);
  EXPECT_EQ(observed.load(std::memory_order_relaxed), 7);
}

TEST(FlowProgram, CutoffRetainsMemoizedConstructionOutputs) {
  int input = 4;
  std::atomic<std::uint32_t> constructions = 0;
  std::atomic<std::uint32_t> reads = 0;

  flow::begin();
  auto external = flow::external<int>("input");
  auto stable = flow::cutoff(external);
  auto value = flow::resource<int>();
  flow::construct(value, flow::read(stable),
                  [&](int *storage, const int &source) {
                    std::construct_at(storage, source * 2);
                    constructions.fetch_add(1, std::memory_order_relaxed);
                  });
  flow::op(flow::read(value), [&](const int &source) {
    EXPECT_EQ(source, input * 2);
    reads.fetch_add(1, std::memory_order_relaxed);
  });
  auto program = flow::end();
  program.bind_external("input", &input);

  strobe::sync::allocator allocator;
  strobe::sync::JobScheduler scheduler(&allocator, 2);
  submit_and_wait(program, scheduler);
  submit_and_wait(program, scheduler);
  EXPECT_EQ(constructions.load(std::memory_order_relaxed), 1);
  EXPECT_EQ(reads.load(std::memory_order_relaxed), 1);

  input = 7;
  submit_and_wait(program, scheduler);
  scheduler.stop();
  EXPECT_EQ(constructions.load(std::memory_order_relaxed), 2);
  EXPECT_EQ(reads.load(std::memory_order_relaxed), 2);
}

TEST(FlowProgram, ReservesCapacityForRetainedMemoizedValues) {
  flow::begin();
  flow::acquire("frame");
  auto stable = flow::cutoff(flow::external<int>("input"));
  auto value = flow::resource<int>();
  flow::construct(value, flow::read(stable),
                  [](int *output, const int &source) {
                    std::construct_at(output, source);
                  });
  flow::op(flow::read(value), [](const int &) {});
  auto program = flow::end();

  EXPECT_EQ(program.data().activation_layout.capacity, 1);
  ASSERT_GE(program.data().resources.size(), 3);
  EXPECT_EQ(program.data().resources[2].slots.slot_count, 2);
}

TEST(FlowProgram, ResumesAcyclicExecutionAfterYield) {
  std::atomic<std::uint32_t> before = 0;
  std::atomic<std::uint32_t> after = 0;
  flow::begin();
  auto value = flow::resource<int>();
  flow::construct(value, 7);
  flow::op(flow::read(value), [&](const int &input) {
    EXPECT_EQ(input, 7);
    before.fetch_add(1, std::memory_order_relaxed);
  });
  flow::yield();
  flow::op(flow::read(value), [&](const int &input) {
    EXPECT_EQ(input, 7);
    after.fetch_add(1, std::memory_order_relaxed);
  });
  auto program = flow::end();

  strobe::sync::allocator allocator;
  strobe::sync::JobScheduler scheduler(&allocator, 2);
  submit_and_wait(program, scheduler);
  EXPECT_EQ(before.load(std::memory_order_relaxed), 1);
  EXPECT_EQ(after.load(std::memory_order_relaxed), 0);

  submit_and_wait(program, scheduler);
  scheduler.stop();
  EXPECT_EQ(before.load(std::memory_order_relaxed), 1);
  EXPECT_EQ(after.load(std::memory_order_relaxed), 1);
}

TEST(FlowProgram, StartsTheNextFeedbackActivationAfterYield) {
  std::atomic<std::uint32_t> invocation = 0;
  std::atomic<std::uint32_t> observed[3] = {};
  flow::begin();
  auto initial = flow::resource<int>();
  flow::construct(initial, 0);
  auto state = flow::phi(initial);
  flow::acquire("step");
  auto next = flow::resource<int>();
  flow::construct(next, flow::read(state), [](int *output, const int &input) {
    std::construct_at(output, input + 1);
  });
  flow::op(flow::read(next), [&](const int &value) {
    invocation.fetch_add(1, std::memory_order_relaxed);
    if (value >= 1 && value <= 3) {
      observed[value - 1].fetch_add(1, std::memory_order_relaxed);
    }
  });
  flow::yield();
  flow::feedback(state, next);
  auto program = flow::end();
  program.release("step", 3);

  strobe::sync::allocator allocator;
  strobe::sync::JobScheduler scheduler(&allocator, 2);
  for (std::uint32_t activation = 0; activation < 3; ++activation) {
    submit_and_wait(program, scheduler);
  }
  scheduler.stop();

  EXPECT_EQ(invocation.load(std::memory_order_relaxed), 3);
  EXPECT_EQ(observed[0].load(std::memory_order_relaxed), 1);
  EXPECT_EQ(observed[1].load(std::memory_order_relaxed), 1);
  EXPECT_EQ(observed[2].load(std::memory_order_relaxed), 1);
}

TEST(FlowProgram, AcquireConsumesReleasedTokens) {
  std::atomic<std::uint32_t> invocations = 0;
  flow::begin();
  flow::acquire("frame");
  flow::op([&] { invocations.fetch_add(1, std::memory_order_relaxed); });
  auto program = flow::end();

  strobe::sync::allocator allocator;
  strobe::sync::JobScheduler scheduler(&allocator, 2);
  const auto submit = [&] { submit_and_wait(program, scheduler); };

  submit();
  EXPECT_EQ(invocations.load(std::memory_order_relaxed), 0);
  EXPECT_THROW(program.release("missing"), std::invalid_argument);

  program.release("frame", 2);
  submit();
  EXPECT_EQ(invocations.load(std::memory_order_relaxed), 2);
  submit();
  EXPECT_EQ(invocations.load(std::memory_order_relaxed), 2);
  scheduler.stop();
}

TEST(FlowProgram, KeepsTheSchedulerFromItsFirstSubmission) {
  flow::begin();
  flow::op([] {});
  auto program = flow::end();

  strobe::sync::allocator first_allocator;
  strobe::sync::allocator second_allocator;
  strobe::sync::JobScheduler first(&first_allocator, 2);
  strobe::sync::JobScheduler second(&second_allocator, 2);
  submit_and_wait(program, first);
  EXPECT_THROW(program.submit(&second), std::invalid_argument);
  first.stop();
  second.stop();
}

TEST(FlowProgram, DrainsTokensAcrossFeedbackActivations) {
  int initial = 0;
  std::atomic<std::uint32_t> invocation = 0;
  std::atomic<std::uint32_t> observed[3] = {};
  flow::begin();
  auto initial_value = flow::external<int>("initial");
  auto state = flow::phi(initial_value);
  flow::acquire("frame");
  auto next = flow::resource<int>();
  flow::construct(next, flow::read(state), [](int *next, const int &state) {
    std::construct_at(next, state + 1);
  });
  flow::op(flow::read(next), [&](const int &value) {
    invocation.fetch_add(1, std::memory_order_relaxed);
    if (value >= 1 && value <= 3) {
      observed[value - 1].fetch_add(1, std::memory_order_relaxed);
    }
  });
  flow::feedback(state, next);
  auto program = flow::end();
  program.bind_external("initial", &initial);
  program.release("frame", 3);

  strobe::sync::allocator allocator;
  strobe::sync::JobScheduler scheduler(&allocator, 3);
  program.submit(&scheduler);
  std::atomic<std::uint32_t> observed_after_completion = 0;
  program.wait();
  observed_after_completion.store(invocation.load(std::memory_order_relaxed),
                                  std::memory_order_relaxed);
  scheduler.stop();

  EXPECT_EQ(invocation.load(std::memory_order_relaxed), 3);
  EXPECT_EQ(observed_after_completion.load(std::memory_order_relaxed), 3);
  EXPECT_EQ(observed[0].load(std::memory_order_relaxed), 1);
  EXPECT_EQ(observed[1].load(std::memory_order_relaxed), 1);
  EXPECT_EQ(observed[2].load(std::memory_order_relaxed), 1);
}

TEST(FlowProgram, DoesNotBackpressureAtTheCompiledActivationCapacity) {
  std::atomic<std::uint32_t> entered = 0;
  std::atomic<bool> proceed = false;
  flow::begin();
  flow::acquire("frame");
  flow::op([&] {
    entered.fetch_add(1, std::memory_order_release);
    while (!proceed.load(std::memory_order_acquire)) {
      std::this_thread::yield();
    }
  });
  auto program = flow::end();
  program.release("frame", 3);

  strobe::sync::allocator allocator;
  strobe::sync::JobScheduler scheduler(&allocator, 3);
  auto submission = std::async(std::launch::async,
                               [&] { return program.submit(&scheduler); });
  const auto deadline =
      std::chrono::steady_clock::now() + std::chrono::seconds(2);
  while (entered.load(std::memory_order_acquire) == 0 &&
         std::chrono::steady_clock::now() < deadline) {
    std::this_thread::yield();
  }
  EXPECT_GT(entered.load(std::memory_order_acquire), 0);
  EXPECT_EQ(submission.wait_for(std::chrono::milliseconds(10)),
            std::future_status::ready);

  proceed.store(true, std::memory_order_release);
  submission.get();
  program.wait();
  scheduler.stop();
  EXPECT_EQ(entered.load(std::memory_order_acquire), 3);
}

TEST(FlowProgram, AYieldStopsTokenDrainingAndRetainsItsActivation) {
  std::atomic<std::uint32_t> before = 0;
  std::atomic<std::uint32_t> after = 0;
  flow::begin();
  flow::acquire("frame");
  flow::op([&] { before.fetch_add(1, std::memory_order_relaxed); });
  flow::yield();
  flow::op([&] { after.fetch_add(1, std::memory_order_relaxed); });
  auto program = flow::end();
  program.release("frame", 3);

  strobe::sync::allocator allocator;
  strobe::sync::JobScheduler scheduler(&allocator, 3);
  const auto submit = [&] { submit_and_wait(program, scheduler); };
  submit();
  EXPECT_EQ(before.load(std::memory_order_relaxed), 1);
  EXPECT_EQ(after.load(std::memory_order_relaxed), 0);

  submit();
  EXPECT_EQ(before.load(std::memory_order_relaxed), 2);
  EXPECT_EQ(after.load(std::memory_order_relaxed), 1);
  scheduler.stop();
}

TEST(FlowProgram, ClaimsRunOnceConstructionForOneConcurrentActivation) {
  std::atomic<std::uint32_t> constructions = 0;
  std::atomic<std::uint32_t> reads = 0;
  flow::begin();
  auto value = flow::resource<int>();
  flow::construct(value, [&](int *storage) {
    constructions.fetch_add(1, std::memory_order_relaxed);
    std::construct_at(storage, 7);
  });
  flow::acquire("frame");
  flow::op(flow::read(value), [&](const int &input) {
    if (input == 7) {
      reads.fetch_add(1, std::memory_order_relaxed);
    }
  });
  auto program = flow::end();
  program.release("frame", 3);

  strobe::sync::allocator allocator;
  strobe::sync::JobScheduler scheduler(&allocator, 3);
  program.submit(&scheduler);
  program.wait();
  scheduler.stop();

  EXPECT_EQ(constructions.load(std::memory_order_relaxed), 1);
  EXPECT_EQ(reads.load(std::memory_order_relaxed), 3);
}

TEST(FlowProgram, HoistsImmutablePhiSeedBeforeAcquire) {
  std::atomic<std::uint32_t> seed_constructions = 0;
  std::atomic<std::uint32_t> frames = 0;

  flow::begin();
  auto initial = flow::resource<int>();
  flow::construct(initial, [&](int *storage) {
    seed_constructions.fetch_add(1, std::memory_order_relaxed);
    std::construct_at(storage, 0);
  });
  auto previous = flow::phi(initial);
  flow::acquire("frame");
  auto current = flow::resource<int>();
  flow::construct(current, 0);
  flow::op(flow::read(previous), flow::write(current),
           [&](const int &input, int &output) {
             output = input + 1;
             frames.fetch_add(1, std::memory_order_relaxed);
           });
  flow::feedback(previous, current);
  auto program = flow::end();

  ASSERT_LT(program.data().nodes[0].source, program.data().nodes.size());
  EXPECT_EQ(program.data().nodes[0].kind,
            flow::detail::OperationKind::construct);
  EXPECT_TRUE(program.data().nodes[0].initialization);

  strobe::sync::allocator allocator;
  strobe::sync::JobScheduler scheduler(&allocator, 3);
  submit_and_wait(program, scheduler);
  EXPECT_EQ(seed_constructions.load(std::memory_order_relaxed), 1);
  EXPECT_EQ(frames.load(std::memory_order_relaxed), 0);

  program.release("frame", 3);
  submit_and_wait(program, scheduler);
  scheduler.stop();

  EXPECT_EQ(seed_constructions.load(std::memory_order_relaxed), 1);
  EXPECT_EQ(frames.load(std::memory_order_relaxed), 3);
}

TEST(FlowProgram, OrdersCutoffsAcrossDrainedActivations) {
  int external = 5;
  std::atomic<std::uint32_t> invocations = 0;
  flow::begin();
  flow::acquire("frame");
  auto input = flow::external<int>("input");
  auto changed = flow::cutoff(input);
  flow::op(flow::read(changed), [&](const int &) {
    invocations.fetch_add(1, std::memory_order_relaxed);
  });
  auto program = flow::end();
  program.bind_external("input", &external);
  program.release("frame", 3);

  strobe::sync::allocator allocator;
  strobe::sync::JobScheduler scheduler(&allocator, 3);
  program.submit(&scheduler);
  program.wait();
  scheduler.stop();

  EXPECT_EQ(invocations.load(std::memory_order_relaxed), 1);
}

TEST(FlowProgram, CompilesAFixedActivationStateRing) {
  flow::begin();
  flow::acquire("frame");
  auto value = flow::resource<int>();
  flow::construct(value, 4);
  flow::op(flow::read(value), [](const int &) {});
  auto program = flow::end();

  const auto &layout = program.data().activation_layout;
  EXPECT_EQ(layout.capacity, 1);
  EXPECT_GT(layout.stride, 0);
  EXPECT_EQ(layout.size, layout.stride * layout.capacity);
  EXPECT_EQ(layout.size % layout.alignment, 0);
  EXPECT_EQ(layout.values_offset % alignof(void *), 0);
  EXPECT_EQ(layout.arguments_offset % alignof(void *), 0);
  EXPECT_LT(layout.generations_offset, layout.stride);
  EXPECT_LT(layout.changed_values_offset, layout.stride);
}

TEST(FlowProgram, ResolvesProjectedResourcesThroughTheirOwner) {
  struct Pair {
    int left;
    int right;
  };

  int result = 0;
  flow::begin();
  auto owner = flow::resource<Pair>();
  flow::construct(owner, Pair{1, 2});
  auto left = flow::project<int>(
      owner, "left", flow::ProjectionKind::disjoint,
      [](Pair &pair) noexcept -> int * { return &pair.left; });
  auto right = flow::project<int>(
      owner, "right", flow::ProjectionKind::disjoint,
      [](Pair &pair) noexcept -> int * { return &pair.right; });
  flow::op(flow::write(left), [](int &value) noexcept { value = 4; });
  flow::op(flow::write(right), [](int &value) noexcept { value = 7; });
  flow::op(flow::read(owner),
           [&](const Pair &pair) noexcept { result = pair.left + pair.right; });
  auto program = flow::end();

  strobe::sync::allocator allocator;
  strobe::sync::JobScheduler scheduler(&allocator, 2);
  submit_and_wait(program, scheduler);
  scheduler.stop();

  EXPECT_EQ(result, 11);
  EXPECT_EQ(program.data().projections.size(), 2);
  EXPECT_EQ(program.data().resources[1].slots.slot_count, 0);
  EXPECT_EQ(program.data().resources[2].slots.slot_count, 0);
}

TEST(FlowProgram, ReusesProjectionIdentityForOwnerAndName) {
  struct Pair {
    int value;
  };

  flow::begin();
  auto owner = flow::resource<Pair>();
  flow::construct(owner, Pair{3});
  auto first = flow::project<int>(
      owner, "value", flow::ProjectionKind::overlapping,
      [](Pair &pair) noexcept -> int * { return &pair.value; });
  auto second = flow::project<int>(
      owner, "value", flow::ProjectionKind::overlapping,
      [](Pair &pair) noexcept -> int * { return &pair.value; });
  flow::op(flow::read(first), [](const int &) noexcept {});
  flow::op(flow::read(second), [](const int &) noexcept {});
  auto program = flow::end();

  ASSERT_EQ(program.data().projections.size(), 1);
  ASSERT_EQ(program.data().nodes.size(), 3);
  ASSERT_FALSE(program.data().accesses_of(1).empty());
  ASSERT_FALSE(program.data().accesses_of(2).empty());
  EXPECT_EQ(program.data().accesses_of(1).front().resource,
            program.data().accesses_of(2).front().resource);

  strobe::sync::allocator allocator;
  strobe::sync::JobScheduler scheduler(&allocator, 2);
  submit_and_wait(program, scheduler);
  scheduler.stop();
}

TEST(FlowProgram, ResolvesExternalProjectorsWhenBindingALibrary) {
  int observed = 0;
  flow::begin();
  auto owner = flow::resource<ExternalProjectionOwner>();
  flow::construct(owner, ExternalProjectionOwner{42});
  auto value = flow::project<int>(
      owner, "value", flow::ProjectionKind::disjoint,
      flow::external_projector("test-projectors", "value", 1));
  flow::op(flow::read(value),
           [&](const int &input) noexcept { observed = input; });
  auto program = flow::end();
  program.bind_library("test-projectors",
                       flow::Library{
                           .resolve_projection = resolve_test_projector,
                       });

  strobe::sync::allocator allocator;
  strobe::sync::JobScheduler scheduler(&allocator, 2);
  submit_and_wait(program, scheduler);
  scheduler.stop();

  EXPECT_EQ(observed, 42);
}

TEST(FlowProgram, RunsManuallyConstructedDependencyGraph) {
  std::atomic<std::uint32_t> state = 0;
  flow::detail::ProgramData data;
  data.callbacks.emplace_back([&](flow::Lifetime, strobe::span<void *const>,
                                  strobe::span<const std::uint32_t>,
                                  strobe::span<const flow::ArgumentGroup>,
                                  strobe::span<const std::uint32_t>,
                                  strobe::span<void *const>) {
    EXPECT_EQ(state.load(std::memory_order_acquire), 0);
    state.store(1, std::memory_order_release);
  });
  data.callbacks.emplace_back([&](flow::Lifetime, strobe::span<void *const>,
                                  strobe::span<const std::uint32_t>,
                                  strobe::span<const flow::ArgumentGroup>,
                                  strobe::span<const std::uint32_t>,
                                  strobe::span<void *const>) {
    EXPECT_EQ(state.load(std::memory_order_acquire), 1);
    state.store(2, std::memory_order_release);
  });
  data.node_storage.predecessors.push_back(0);
  data.node_storage.successors.push_back(1);
  data.nodes.push_back(flow::detail::CompiledNode{
      .callback = 0,
      .inputs = {},
      .outputs = {},
      .accesses = {},
      .predecessors = {},
      .successors = {.offset = 0, .count = 1},
      .temporal_predecessors = {},
      .temporal_successors = {},
      .initialization = false,
  });
  data.nodes.push_back(flow::detail::CompiledNode{
      .callback = 1,
      .inputs = {},
      .outputs = {},
      .accesses = {},
      .predecessors = {.offset = 0, .count = 1},
      .successors = {},
      .temporal_predecessors = {},
      .temporal_successors = {},
      .initialization = false,
  });

  flow::Program program(
      flow::detail::compiler::build_program_artifact(std::move(data)));
  strobe::sync::allocator allocator;
  strobe::sync::JobScheduler scheduler(&allocator, 2);
  program.submit(&scheduler);
  program.wait();
  scheduler.stop();

  EXPECT_EQ(state.load(std::memory_order_acquire), 2);
}

TEST(FlowProgram, RunsIndependentNodesInOneSubmission) {
  std::atomic<std::uint32_t> calls = 0;
  flow::detail::ProgramData data;
  data.callbacks.emplace_back([&](flow::Lifetime, strobe::span<void *const>,
                                  strobe::span<const std::uint32_t>,
                                  strobe::span<const flow::ArgumentGroup>,
                                  strobe::span<const std::uint32_t>,
                                  strobe::span<void *const>) {
    calls.fetch_add(1, std::memory_order_relaxed);
  });
  data.callbacks.emplace_back([&](flow::Lifetime, strobe::span<void *const>,
                                  strobe::span<const std::uint32_t>,
                                  strobe::span<const flow::ArgumentGroup>,
                                  strobe::span<const std::uint32_t>,
                                  strobe::span<void *const>) {
    calls.fetch_add(1, std::memory_order_relaxed);
  });
  data.nodes.push_back(flow::detail::CompiledNode{
      .callback = 0,
      .inputs = {},
      .outputs = {},
      .accesses = {},
      .predecessors = {},
      .successors = {},
      .temporal_predecessors = {},
      .temporal_successors = {},
      .initialization = false,
  });
  data.nodes.push_back(flow::detail::CompiledNode{
      .callback = 1,
      .inputs = {},
      .outputs = {},
      .accesses = {},
      .predecessors = {},
      .successors = {},
      .temporal_predecessors = {},
      .temporal_successors = {},
      .initialization = false,
  });

  flow::Program program(
      flow::detail::compiler::build_program_artifact(std::move(data)));
  strobe::sync::allocator allocator;
  strobe::sync::JobScheduler scheduler(&allocator, 2);
  program.submit(&scheduler);
  program.wait();
  scheduler.stop();

  EXPECT_EQ(calls.load(std::memory_order_relaxed), 2);
}

} // namespace
