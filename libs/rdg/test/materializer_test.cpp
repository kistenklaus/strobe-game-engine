#include "materializer.hpp"

#include <strobe/core/sync/job_scheduler.hpp>
#include <strobe/flow/flow.hpp>

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>
#include <string_view>

namespace strobe::rdg::test {
namespace {

std::array<std::uint32_t, 3> external_values{};
std::uint32_t external_count = 0;
std::uint32_t external_resource_sum = 0;

void receive_span(const flow::ExternalOpContext &context) noexcept {
  external_count = context.immediate_count == 1
                       ? context.immediate_counts[0]
                       : std::numeric_limits<std::uint32_t>::max();
  if (external_count == external_values.size() &&
      context.immediate_types[0] == flow::type_id<std::uint32_t>()) {
    const auto *values =
        static_cast<const std::uint32_t *>(context.immediates[0]);
    std::copy_n(values, external_values.size(), external_values.begin());
  }
}

void receive_resource_span(const flow::ExternalOpContext &context) noexcept {
  external_resource_sum = 0;
  if (context.argument_count != 3 || context.argument_group_count != 1 ||
      context.argument_groups[0].offset != 0 ||
      context.argument_groups[0].count != 3) {
    external_resource_sum = std::numeric_limits<std::uint32_t>::max();
    return;
  }
  for (std::uint32_t index = 0; index < context.argument_count; ++index) {
    external_resource_sum +=
        *static_cast<const std::uint32_t *>(context.arguments[index]);
  }
}

flow::ExternalOpFunction resolve_span(void *, std::string_view symbol,
                                      std::uint32_t version) noexcept {
  if (version != 1) {
    return nullptr;
  }
  if (symbol == "receive_span") {
    return receive_span;
  }
  if (symbol == "receive_resource_span") {
    return receive_resource_span;
  }
  return nullptr;
}

TEST(IntrinsicMaterializer, SuppliesTypedImmediateSpansToLambdaBodies) {
  std::array<std::uint32_t, 3> observed{};
  flow::CompilerPlugins plugins;
  plugins.normalized.emplace_back([&](flow::detail::compiler::NormalizedIR &ir,
                                      flow::CompileContext &context) {
    detail::IntrinsicMaterializer pass(ir, context, "strobe.test", 1);
    pass.define("span")
        .metadata("span", flow::OpSemantics::side_effect)
        .immediate_span<std::uint32_t>()
        .body([&](span<const std::uint32_t> values) noexcept {
          ASSERT_EQ(values.size(), observed.size());
          for (std::size_t index = 0; index < values.size(); ++index) {
            observed[index] = values[index];
          }
        });
    pass.run();
  });

  const std::array<std::uint32_t, 3> values{4, 8, 15};
  flow::begin(plugins);
  flow::op(flow::intrinsic("strobe.test", "span")
               .immediate_span(span<const std::uint32_t>{values}));
  auto program = flow::end();

  sync::allocator allocator;
  sync::JobScheduler scheduler(&allocator, 2);
  program.submit(&scheduler);
  program.wait();
  scheduler.stop();

  EXPECT_EQ(observed, values);
}

TEST(IntrinsicMaterializer, SuppliesImmediateSpansToExternalBodies) {
  external_values = {};
  external_count = 0;
  flow::CompilerPlugins plugins;
  plugins.normalized.emplace_back([](flow::detail::compiler::NormalizedIR &ir,
                                     flow::CompileContext &context) {
    detail::IntrinsicMaterializer pass(ir, context, "strobe.test", 1);
    pass.define("external_span")
        .metadata("external span", flow::OpSemantics::side_effect)
        .immediate_span<std::uint32_t>()
        .body(flow::external_op("span-library", "receive_span"));
    pass.run();
  });

  const std::array<std::uint32_t, 3> values{16, 23, 42};
  flow::begin(plugins);
  flow::op(flow::intrinsic("strobe.test", "external_span")
               .immediate_span(span<const std::uint32_t>{values}));
  auto program = flow::end();
  program.bind_library(
      "span-library", flow::Library{.state = nullptr, .resolve = resolve_span});

  sync::allocator allocator;
  sync::JobScheduler scheduler(&allocator, 2);
  program.submit(&scheduler);
  program.wait();
  scheduler.stop();

  EXPECT_EQ(external_count, values.size());
  EXPECT_EQ(external_values, values);
}

TEST(IntrinsicMaterializer, SupportsEmptyImmediateSpans) {
  std::size_t observed = 1;
  flow::CompilerPlugins plugins;
  plugins.normalized.emplace_back([&](flow::detail::compiler::NormalizedIR &ir,
                                      flow::CompileContext &context) {
    detail::IntrinsicMaterializer pass(ir, context, "strobe.test", 1);
    pass.define("empty_span")
        .metadata("empty span", flow::OpSemantics::side_effect)
        .immediate_span<std::uint32_t>()
        .body([&](span<const std::uint32_t> values) noexcept {
          observed = values.size();
        });
    pass.run();
  });

  flow::begin(plugins);
  flow::op(flow::intrinsic("strobe.test", "empty_span")
               .immediate_span(span<const std::uint32_t>{}));
  auto program = flow::end();

  sync::allocator allocator;
  sync::JobScheduler scheduler(&allocator, 2);
  program.submit(&scheduler);
  program.wait();
  scheduler.stop();

  EXPECT_EQ(observed, 0);
}

TEST(IntrinsicMaterializer, SuppliesFixedResourceGroupsToLambdaBodies) {
  std::uint32_t observed = 0;
  flow::CompilerPlugins plugins;
  plugins.normalized.emplace_back([&](flow::detail::compiler::NormalizedIR &ir,
                                      flow::CompileContext &context) {
    detail::IntrinsicMaterializer pass(ir, context, "strobe.test", 1);
    pass.define("resource_span")
        .metadata("resource span", flow::OpSemantics::side_effect)
        .arg_span<std::uint32_t>(flow::AccessMode::read)
        .body([&](flow::ArgumentSpan<const std::uint32_t> values) noexcept {
          for (const auto value : values) {
            observed += *value;
          }
        });
    pass.run();
  });

  flow::begin(plugins);
  std::array<flow::Resource<std::uint32_t>, 3> resources{
      flow::resource<std::uint32_t>(), flow::resource<std::uint32_t>(),
      flow::resource<std::uint32_t>()};
  flow::construct(resources[0], 4U);
  flow::construct(resources[1], 8U);
  flow::construct(resources[2], 15U);
  flow::op(
      flow::intrinsic("strobe.test", "resource_span"),
      flow::read_span(span<const flow::Resource<std::uint32_t>>{resources}));
  auto program = flow::end();

  sync::allocator allocator;
  sync::JobScheduler scheduler(&allocator, 2);
  program.submit(&scheduler);
  program.wait();
  scheduler.stop();

  EXPECT_EQ(observed, 27);
}

TEST(IntrinsicMaterializer, SuppliesResourceGroupsToExternalBodies) {
  external_resource_sum = 0;
  flow::CompilerPlugins plugins;
  plugins.normalized.emplace_back([](flow::detail::compiler::NormalizedIR &ir,
                                     flow::CompileContext &context) {
    detail::IntrinsicMaterializer pass(ir, context, "strobe.test", 1);
    pass.define("external_resource_span")
        .metadata("external resource span", flow::OpSemantics::side_effect)
        .arg_span<std::uint32_t>(flow::AccessMode::read)
        .body(flow::external_op("span-library", "receive_resource_span"));
    pass.run();
  });

  flow::begin(plugins);
  std::array<flow::Resource<std::uint32_t>, 3> resources{
      flow::resource<std::uint32_t>(), flow::resource<std::uint32_t>(),
      flow::resource<std::uint32_t>()};
  flow::construct(resources[0], 16U);
  flow::construct(resources[1], 23U);
  flow::construct(resources[2], 42U);
  flow::op(
      flow::intrinsic("strobe.test", "external_resource_span"),
      flow::read_span(span<const flow::Resource<std::uint32_t>>{resources}));
  auto program = flow::end();
  program.bind_library(
      "span-library", flow::Library{.state = nullptr, .resolve = resolve_span});

  sync::allocator allocator;
  sync::JobScheduler scheduler(&allocator, 2);
  program.submit(&scheduler);
  program.wait();
  scheduler.stop();

  EXPECT_EQ(external_resource_sum, 81);
}

TEST(IntrinsicMaterializer, PreservesNullElementsInResourceSpans) {
  std::array<const std::uint32_t *, 3> observed{};
  flow::CompilerPlugins plugins;
  plugins.normalized.emplace_back([&](flow::detail::compiler::NormalizedIR &ir,
                                      flow::CompileContext &context) {
    detail::IntrinsicMaterializer pass(ir, context, "strobe.test", 1);
    pass.define("nullable_resource_span")
        .metadata("nullable resource span", flow::OpSemantics::side_effect)
        .arg_span<std::uint32_t>(flow::AccessMode::read)
        .body([&](flow::ArgumentSpan<const std::uint32_t> values) noexcept {
          for (std::size_t index = 0; index < values.size(); ++index) {
            observed[index] = values[index];
          }
        });
    pass.run();
  });

  flow::begin(plugins);
  auto first = flow::resource<std::uint32_t>();
  auto last = flow::resource<std::uint32_t>();
  flow::construct(first, 11U);
  flow::construct(last, 29U);
  const std::array<flow::Resource<std::uint32_t>, 3> resources{
      first, flow::null_resource, last};
  flow::op(flow::intrinsic("strobe.test", "nullable_resource_span"),
           flow::read_span(span<const flow::Resource<std::uint32_t>>{resources}));
  auto program = flow::end();

  sync::allocator allocator;
  sync::JobScheduler scheduler(&allocator, 2);
  program.submit(&scheduler);
  program.wait();
  scheduler.stop();

  ASSERT_NE(observed[0], nullptr);
  EXPECT_EQ(*observed[0], 11U);
  EXPECT_EQ(observed[1], nullptr);
  ASSERT_NE(observed[2], nullptr);
  EXPECT_EQ(*observed[2], 29U);
}

TEST(IntrinsicMaterializer, SuppliesNullableResourcesAsPointers) {
  std::uint32_t observed = 0;
  flow::CompilerPlugins plugins;
  plugins.normalized.emplace_back([&](flow::detail::compiler::NormalizedIR &ir,
                                      flow::CompileContext &context) {
    detail::IntrinsicMaterializer pass(ir, context, "strobe.test", 1);
    pass.define("optional_resources")
        .metadata("optional resources", flow::OpSemantics::side_effect)
        .arg<std::uint32_t>(flow::AccessMode::read)
        .arg<std::uint32_t>(flow::AccessMode::read)
        .body([&](const std::uint32_t *present,
                  const std::uint32_t *absent) noexcept {
          observed = present != nullptr && absent == nullptr ? *present : 0;
        });
    pass.run();
  });

  flow::begin(plugins);
  auto resource = flow::resource<std::uint32_t>();
  flow::construct(resource, 42U);
  flow::op(flow::intrinsic("strobe.test", "optional_resources"),
           flow::read(resource),
           flow::read(flow::Resource<std::uint32_t>{}));
  auto program = flow::end();

  sync::allocator allocator;
  sync::JobScheduler scheduler(&allocator, 2);
  program.submit(&scheduler);
  program.wait();
  scheduler.stop();

  EXPECT_EQ(observed, 42);
}

TEST(IntrinsicMaterializer, RejectsNullForNonnullArguments) {
  flow::CompilerPlugins plugins;
  plugins.normalized.emplace_back([](flow::detail::compiler::NormalizedIR &ir,
                                     flow::CompileContext &context) {
    detail::IntrinsicMaterializer pass(ir, context, "strobe.test", 1);
    pass.define("nonnull_resource")
        .arg_nonnull<std::uint32_t>(flow::AccessMode::read)
        .body([](const std::uint32_t &) noexcept {});
    pass.run();
  });

  flow::begin(plugins);
  flow::op(flow::intrinsic("strobe.test", "nonnull_resource"),
           flow::read(flow::Resource<std::uint32_t>{}));
  EXPECT_THROW((void)flow::end(), std::logic_error);
}

TEST(IntrinsicMaterializer, SuppliesOptionalImmediatesAsNullablePointers) {
  std::uint32_t observed = 0;
  flow::CompilerPlugins plugins;
  plugins.normalized.emplace_back([&](flow::detail::compiler::NormalizedIR &ir,
                                      flow::CompileContext &context) {
    detail::IntrinsicMaterializer pass(ir, context, "strobe.test", 1);
    pass.define("optional_immediates")
        .metadata("optional immediates", flow::OpSemantics::side_effect)
        .immediate_optional<std::uint32_t>()
        .immediate_optional<std::uint32_t>()
        .body([&](const std::uint32_t *present,
                  const std::uint32_t *absent) noexcept {
          observed = present != nullptr && absent == nullptr ? *present : 0;
        });
    pass.run();
  });

  constexpr std::uint32_t value = 42;
  flow::begin(plugins);
  flow::op(flow::intrinsic("strobe.test", "optional_immediates")
               .immediate_span(span<const std::uint32_t>{&value, 1})
               .immediate_span(span<const std::uint32_t>{}));
  auto program = flow::end();

  sync::allocator allocator;
  sync::JobScheduler scheduler(&allocator, 2);
  program.submit(&scheduler);
  program.wait();
  scheduler.stop();

  EXPECT_EQ(observed, 42);
}

TEST(IntrinsicMaterializer, SuppliesConstructionLifetimeToLambdaBodies) {
  flow::Lifetime observed{};
  std::uint32_t result = 0;
  flow::CompilerPlugins plugins;
  plugins.normalized.emplace_back([&](flow::detail::compiler::NormalizedIR &ir,
                                      flow::CompileContext &context) {
    detail::IntrinsicMaterializer pass(ir, context, "strobe.test", 1);
    pass.define("construct_with_lifetime")
        .arg_nonnull<std::uint32_t>(flow::AccessMode::write)
        .arg_nonnull<std::uint32_t>(flow::AccessMode::read)
        .body([&](std::uint32_t *output, const std::uint32_t &input,
                  flow::Lifetime lifetime) noexcept {
          observed = lifetime;
          std::construct_at(output, input + 1);
        });
    pass.run();
  });

  flow::begin(plugins);
  const auto input = flow::resource<std::uint32_t>();
  const auto output = flow::resource<std::uint32_t>();
  flow::construct(input, [] (std::uint32_t *value) noexcept {
    std::construct_at(value, 41U);
  });
  flow::construct(output,
                  flow::intrinsic("strobe.test", "construct_with_lifetime"),
                  flow::read(input));
  flow::op(flow::read(output), [&result](const std::uint32_t &value) noexcept {
    result = value;
  });
  auto program = flow::end();

  sync::allocator allocator;
  sync::JobScheduler scheduler(&allocator, 2);
  program.submit(&scheduler);
  program.wait();
  scheduler.stop();

  EXPECT_EQ(result, 42);
  EXPECT_LT(observed.begin, observed.end);
}

} // namespace
} // namespace strobe::rdg::test
