#include <strobe/flow/detail/compile.hpp>
#include <strobe/flow/flow.hpp>

#include <strobe/core/sync/job_scheduler.hpp>

#include <gtest/gtest.h>

#include <cstdint>
#include <cstring>
#include <functional>
#include <limits>
#include <vector>

namespace flow = strobe::flow;

namespace {

std::uint64_t external_immediate_observed = 0;

void external_immediate(const flow::ExternalOpContext &context) noexcept {
  if (context.immediate_count == 2 && context.immediate_counts[0] == 1 &&
      context.immediate_counts[1] == 2 &&
      context.immediate_types[0] == flow::type_id<std::uint32_t>() &&
      context.immediate_types[1] == flow::type_id<std::uint64_t>()) {
    external_immediate_observed =
        *static_cast<const std::uint32_t *>(context.immediates[0]) +
        static_cast<const std::uint64_t *>(context.immediates[1])[0] +
        static_cast<const std::uint64_t *>(context.immediates[1])[1];
  }
}

flow::ExternalOpFunction
resolve_immediate_library(void *, std::string_view symbol,
                          std::uint32_t version) noexcept {
  return symbol == "invoke" && version == 1 ? external_immediate : nullptr;
}

TEST(FlowCompilerPlugins, ExecutesStableStagesInPipelineOrder) {
  std::vector<std::uint32_t> order;
  flow::CompilerPlugins plugins;
  plugins.normalized.emplace_back(
      [&](flow::detail::compiler::NormalizedIR &ir, flow::CompileContext &) {
        EXPECT_EQ(ir.nodes.size(), 2);
        order.push_back(0);
      });
  plugins.normalized.emplace_back(
      [&](flow::detail::compiler::NormalizedIR &, flow::CompileContext &) {
        order.push_back(1);
      });
  plugins.values.emplace_back(
      [&](flow::detail::compiler::ValueIR &ir, flow::CompileContext &) {
        EXPECT_EQ(ir.values.size(), 1);
        order.push_back(2);
      });
  plugins.dependencies.emplace_back(
      [&](flow::detail::compiler::DependencyIR &, flow::CompileContext &) {
        order.push_back(3);
      });
  plugins.ordered.emplace_back(
      [&](flow::detail::compiler::OrderedIR &, flow::CompileContext &) {
        order.push_back(4);
      });
  plugins.graph.emplace_back(
      [&](flow::detail::compiler::GraphIR &ir, flow::CompileContext &) {
        EXPECT_EQ(ir.graph.node_count(), 2);
        order.push_back(5);
      });
  plugins.storage.emplace_back(
      [&](flow::detail::compiler::StorageIR &, flow::CompileContext &) {
        order.push_back(6);
      });
  plugins.lifetimes.emplace_back(
      [&](flow::detail::compiler::LifetimeIR &, flow::CompileContext &) {
        order.push_back(7);
      });
  plugins.storage_layout.emplace_back(
      [&](flow::detail::compiler::StorageLayoutIR &, flow::CompileContext &) {
        order.push_back(8);
      });
  plugins.activation_layout.emplace_back(
      [&](flow::detail::compiler::ActivationLayoutIR &,
          flow::CompileContext &) { order.push_back(9); });
  plugins.program.emplace_back(
      [&](flow::detail::ProgramData &data, flow::CompileContext &) {
        order.push_back(10);
        data.source_operation_count = 41;
      });

  flow::begin(plugins);
  auto value = flow::resource<int>();
  flow::construct(value, 1);
  flow::op(flow::read(value), [](const int &) {});
  auto program = flow::end();

  EXPECT_EQ(order,
            (std::vector<std::uint32_t>{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10}));
  EXPECT_EQ(program.operation_count(), 41);
}

TEST(FlowCompilerPlugins, RejectsEmptyPasses) {
  flow::detail::OpStream stream;
  auto value = stream.resource<int>();
  stream.construct(value, 1);
  stream.validate();

  flow::CompilerPlugins plugins;
  plugins.normalized.push_back(
      std::function<void(flow::detail::compiler::NormalizedIR &,
                         flow::CompileContext &)>{});

  EXPECT_THROW(flow::detail::compile(stream, plugins), std::logic_error);
}

TEST(FlowCompilerPlugins, BeginSnapshotsPluginTable) {
  std::uint32_t calls = 0;
  flow::CompilerPlugins plugins;
  plugins.program.emplace_back(
      [&](flow::detail::ProgramData &, flow::CompileContext &) { ++calls; });

  flow::begin(plugins);
  plugins.program.clear();
  auto value = flow::resource<int>();
  flow::construct(value, 1);
  [[maybe_unused]] auto program = flow::end();

  EXPECT_EQ(calls, 1);
}

TEST(FlowCompilerPlugins, ExposesIntrinsicsToNormalizedPasses) {
  bool observed = false;
  flow::CompilerPlugins plugins;
  plugins.normalized.emplace_back(
      [&](flow::detail::compiler::NormalizedIR &ir, flow::CompileContext &) {
        ASSERT_EQ(ir.nodes.size(), 2);
        ASSERT_TRUE(ir.nodes[1].intrinsic.has_value());
        EXPECT_EQ(ir.nodes[1].intrinsic->domain.view(), "strobe.test");
        EXPECT_EQ(ir.nodes[1].intrinsic->opcode.view(), "consume");
        EXPECT_EQ(ir.nodes[1].intrinsic->version, 2);
        ir.nodes[1].intrinsic.reset();
        observed = true;
      });

  flow::begin(plugins);
  auto value = flow::resource<int>();
  flow::construct(value, 1);
  flow::op(flow::intrinsic("strobe.test", "consume", 2), flow::read(value));
  auto program = flow::end();

  EXPECT_TRUE(observed);
  ASSERT_EQ(program.data().nodes.size(), 2);
  EXPECT_EQ(program.data().nodes[1].callback, flow::detail::null_callback_id);
}

TEST(FlowCompilerPlugins, CarriesResolvedOperationsToProgramEmission) {
  flow::CompilerPlugins plugins;
  plugins.normalized.emplace_back([](flow::detail::compiler::NormalizedIR &ir,
                                     flow::CompileContext &context) {
    auto &node = ir.nodes[1];
    flow::detail::ResolvedOperation operation;
    operation.metadata = flow::OpMetadata("resolved callback");
    operation.argument_types.push_back(flow::type_id<int>());
    operation.argument_accesses.push_back(flow::AccessMode::read);
    operation.argument_groups.push_back({0, 1, 0, 1});
    operation.argument_elements.push_back(0);
    operation.runtime = [](flow::Lifetime, strobe::span<void *const>,
                           strobe::span<const std::uint32_t>,
                           strobe::span<const flow::ArgumentGroup>,
                           strobe::span<const std::uint32_t>,
                           strobe::span<void *const>) {};
    node.resolved_operation = context.resolve_operation(std::move(operation));
    node.intrinsic.reset();
  });

  flow::begin(plugins);
  auto value = flow::resource<int>();
  flow::construct(value, 1);
  flow::op(flow::intrinsic("test", "resolved", 1), flow::read(value));
  auto program = flow::end();

  ASSERT_EQ(program.data().callbacks.size(), 2);
  EXPECT_EQ(program.data().callbacks[1].metadata.title.view(),
            "resolved callback");
  ASSERT_EQ(program.data().nodes.size(), 2);
  EXPECT_EQ(program.data().nodes[1].callback, 1);
}

TEST(FlowCompilerPlugins, ExecutesResolvedOperationWithStoredImmediates) {
  constexpr std::uint32_t first = 17;
  constexpr std::array<std::uint64_t, 2> second{21, 34};
  std::uint64_t observed = 0;
  flow::CompilerPlugins plugins;
  plugins.normalized.emplace_back([&](flow::detail::compiler::NormalizedIR &ir,
                                      flow::CompileContext &context) {
    auto &node = ir.nodes[1];
    flow::detail::ResolvedOperation operation;
    operation.metadata = flow::OpMetadata("resolved immediates", {},
                                          flow::OpSemantics::side_effect);
    operation.immediate_data.resize(24);
    std::memcpy(operation.immediate_data.data(), &first, sizeof(first));
    std::memcpy(operation.immediate_data.data() + 8, second.data(),
                sizeof(second));
    operation.immediates.push_back({flow::type_id<std::uint32_t>(), 0,
                                    sizeof(first), alignof(decltype(first)),
                                    sizeof(std::uint32_t), 1});
    operation.immediates.push_back({flow::type_id<std::uint64_t>(), 8,
                                    sizeof(second), alignof(decltype(second)),
                                    sizeof(std::uint64_t), 2});
    operation.runtime = [&](flow::Lifetime,
                            strobe::span<void *const> immediates,
                            strobe::span<const std::uint32_t> counts,
                            strobe::span<const flow::ArgumentGroup>,
                            strobe::span<const std::uint32_t>,
                            strobe::span<void *const>) noexcept {
      EXPECT_EQ(counts[0], 1);
      EXPECT_EQ(counts[1], 2);
      observed = *static_cast<const std::uint32_t *>(immediates[0]) +
                 static_cast<const std::uint64_t *>(immediates[1])[0] +
                 static_cast<const std::uint64_t *>(immediates[1])[1];
    };
    node.semantics = flow::OpSemantics::side_effect;
    node.resolved_operation = context.resolve_operation(std::move(operation));
    node.intrinsic.reset();
  });

  flow::begin(plugins);
  auto value = flow::resource<int>();
  flow::construct(value, 1);
  flow::op(flow::intrinsic("test", "immediates", 1), flow::read(value));
  auto program = flow::end();

  strobe::sync::allocator allocator;
  strobe::sync::JobScheduler scheduler(&allocator, 2);
  const auto completion = program.submit(&scheduler);
  scheduler.wait(completion);
  program.rethrow_execution_error();
  scheduler.stop();

  EXPECT_EQ(observed, first + second[0] + second[1]);
}

TEST(FlowCompilerPlugins, CarriesGeneratedExternalOperationsToProgramEmission) {
  flow::CompilerPlugins plugins;
  plugins.normalized.emplace_back([](flow::detail::compiler::NormalizedIR &ir,
                                     flow::CompileContext &context) {
    auto &node = ir.nodes[1];
    flow::detail::ResolvedOperation operation;
    operation.metadata = flow::OpMetadata("generated external", {},
                                          flow::OpSemantics::side_effect);
    operation.external = flow::external_op("generated-library", "invoke", 3);
    operation.argument_types.push_back(flow::type_id<int>());
    operation.argument_accesses.push_back(flow::AccessMode::read);
    operation.argument_groups.push_back({0, 1, 0, 1});
    operation.argument_elements.push_back(0);
    node.resolved_operation = context.resolve_operation(std::move(operation));
    node.intrinsic.reset();
  });

  flow::begin(plugins);
  auto value = flow::resource<int>();
  flow::construct(value, 1);
  flow::op(flow::intrinsic("test", "generated-external", 1), flow::read(value));
  auto program = flow::end();

  ASSERT_EQ(program.data().callbacks.size(), 2);
  const auto &callback = program.data().callbacks[1];
  EXPECT_EQ(callback.metadata.title.view(), "generated external");
  ASSERT_TRUE(callback.external.has_value());
  EXPECT_EQ(callback.external->library_id.view(), "generated-library");
  EXPECT_EQ(callback.external->symbol.view(), "invoke");
  EXPECT_EQ(callback.external->abi_version, 3);
  EXPECT_FALSE(callback.runtime);
  EXPECT_FALSE(program.libraries_bound());
}

TEST(FlowCompilerPlugins, SuppliesStoredImmediatesToExternalOperations) {
  constexpr std::uint32_t first = 9;
  constexpr std::array<std::uint64_t, 2> second{33, 44};
  external_immediate_observed = 0;
  flow::CompilerPlugins plugins;
  plugins.normalized.emplace_back([=](flow::detail::compiler::NormalizedIR &ir,
                                      flow::CompileContext &context) {
    auto &node = ir.nodes[1];
    flow::detail::ResolvedOperation operation;
    operation.metadata = flow::OpMetadata("external immediates", {},
                                          flow::OpSemantics::side_effect);
    operation.external = flow::external_op("immediate-library", "invoke", 1);
    operation.immediate_data.resize(24);
    std::memcpy(operation.immediate_data.data(), &first, sizeof(first));
    std::memcpy(operation.immediate_data.data() + 8, second.data(),
                sizeof(second));
    operation.immediates.push_back({flow::type_id<std::uint32_t>(), 0,
                                    sizeof(first), alignof(decltype(first)),
                                    sizeof(std::uint32_t), 1});
    operation.immediates.push_back({flow::type_id<std::uint64_t>(), 8,
                                    sizeof(second), alignof(decltype(second)),
                                    sizeof(std::uint64_t), 2});
    node.semantics = flow::OpSemantics::side_effect;
    node.resolved_operation = context.resolve_operation(std::move(operation));
    node.intrinsic.reset();
  });

  flow::begin(plugins);
  auto value = flow::resource<int>();
  flow::construct(value, 1);
  flow::op(flow::intrinsic("test", "external-immediates", 1),
           flow::read(value));
  auto program = flow::end();
  program.bind_library(
      "immediate-library",
      flow::Library{.state = nullptr, .resolve = resolve_immediate_library});

  strobe::sync::allocator allocator;
  strobe::sync::JobScheduler scheduler(&allocator, 2);
  const auto completion = program.submit(&scheduler);
  scheduler.wait(completion);
  program.rethrow_execution_error();
  scheduler.stop();

  EXPECT_EQ(external_immediate_observed, first + second[0] + second[1]);
}

TEST(FlowCompilerPlugins, RejectsIntrinsicsLeftAfterCompilerPasses) {
  flow::begin();
  auto value = flow::resource<int>();
  flow::construct(value, flow::intrinsic("strobe.test", "construct", 4));

  try {
    [[maybe_unused]] auto program = flow::end();
    FAIL() << "expected unhandled intrinsic validation to fail";
  } catch (const std::logic_error &error) {
    const std::string message = error.what();
    EXPECT_NE(message.find("strobe.test/construct"), std::string::npos);
    EXPECT_NE(message.find("version 4"), std::string::npos);
    EXPECT_NE(message.find("source operation 0"), std::string::npos);
  }
}

TEST(FlowCompilerPlugins, SharesTypedStateAcrossStages) {
  flow::CompilerPlugins plugins;
  plugins.normalized.emplace_back([](flow::detail::compiler::NormalizedIR &,
                                     flow::CompileContext &context) {
    context.emplace_plugin_data<std::uint32_t>("test.counter", 17);
  });
  plugins.program.emplace_back(
      [](flow::detail::ProgramData &data, flow::CompileContext &context) {
        auto &counter = context.plugin_data<std::uint32_t>("test.counter");
        ++counter;
        data.source_operation_count = counter;
      });

  flow::begin(plugins);
  auto value = flow::resource<int>();
  flow::construct(value, 1);
  auto program = flow::end();

  EXPECT_EQ(program.operation_count(), 18);
}

TEST(FlowCompileContext, ValidatesIdentifiersAndTypes) {
  flow::CompileContext context;
  EXPECT_THROW(context.emplace_plugin_data<int>("", 1), std::invalid_argument);
  context.emplace_plugin_data<int>("value", 1);
  EXPECT_THROW(context.emplace_plugin_data<int>("value", 2),
               std::invalid_argument);
  EXPECT_THROW(context.plugin_data<float>("value"), std::invalid_argument);
  EXPECT_THROW(context.plugin_data<int>("missing"), std::out_of_range);
}

TEST(FlowCompileContext, RetainsPluginDataForTheProgramLifetime) {
  struct State {
    explicit State(std::uint32_t *destructions) : destructions(destructions) {}
    ~State() noexcept { ++*destructions; }
    std::uint32_t *destructions;
  };

  std::uint32_t destructions = 0;
  {
    flow::CompilerPlugins plugins;
    plugins.normalized.emplace_back([&](flow::detail::compiler::NormalizedIR &,
                                        flow::CompileContext &context) {
      context.emplace_plugin_data<State>("test.state", &destructions);
    });
    flow::begin(plugins);
    auto value = flow::resource<int>();
    flow::construct(value, 1);
    auto program = flow::end();
    EXPECT_EQ(destructions, 0);
    EXPECT_EQ(program.data()
                  .plugin_data.plugin_data<State>("test.state")
                  .destructions,
              &destructions);
  }
  EXPECT_EQ(destructions, 1);
}

TEST(FlowCompilerPlugins, RejectsInvalidIrImmediatelyAfterPass) {
  flow::detail::OpStream stream;
  auto value = stream.resource<int>();
  stream.construct(value, 1);
  stream.validate();

  flow::CompilerPlugins plugins;
  plugins.normalized.emplace_back(
      [](flow::detail::compiler::NormalizedIR &ir, flow::CompileContext &) {
        ir.nodes[0].accesses.count = std::numeric_limits<std::uint32_t>::max();
      });

  EXPECT_THROW(flow::detail::compile(stream, plugins), std::logic_error);
}

TEST(FlowCompilerPlugins, PreservesResourceTypesInProgramData) {
  flow::begin();
  auto owned = flow::resource<int>();
  flow::construct(owned, 1);
  [[maybe_unused]] auto external = flow::external<float>("external");
  auto program = flow::end();

  ASSERT_EQ(program.data().resource_types.size(), 2);
  EXPECT_EQ(program.data().resource_types[0], flow::type_id<int>());
  EXPECT_EQ(program.data().resource_types[1], flow::type_id<float>());
}

} // namespace
