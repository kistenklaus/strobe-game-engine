#include <strobe/flow/detail/compile.hpp>
#include <strobe/flow/flow.hpp>

#include <gtest/gtest.h>

#include <cstdint>
#include <functional>
#include <limits>
#include <vector>

namespace flow = strobe::flow;

namespace {

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
