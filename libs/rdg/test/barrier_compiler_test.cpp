#include "strobe/rdg/plugins.hpp"

#include "strobe/flow/flow.hpp"
#include "strobe/flow/loop.hpp"
#include "strobe/rdg/command_buffer.hpp"
#include "strobe/rdg/command_pool.hpp"
#include "strobe/rdg/rdg.hpp"
#include "strobe/rdg/swapchain.hpp"
#include "strobe/rhi/objects/image.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>

namespace strobe::rdg::test {
namespace {

TEST(BarrierCompiler, LowersSwapchainStateAndAccessIntrinsics) {
  flow::CompilerPlugins plugins;
  rdg::append_plugins(plugins);
  flow::begin(plugins);
  rdg::begin();

  rdg::Swapchain swapchain{"main"};
  rdg::CommandPool pool;
  flow::loop([&] {
    flow::acquire("frame");
    auto image = rdg::acquire(swapchain);
    auto command = pool.alloc();
    command.present(image);
    rdg::submit(command);
  });

  rdg::end();
  auto program = flow::end();
  EXPECT_FALSE(std::ranges::any_of(program.data().nodes, [](const auto &node) {
    return node.kind == flow::detail::OperationKind::callback &&
           node.callback == flow::detail::null_callback_id;
  }));
  EXPECT_TRUE(
      std::ranges::any_of(program.data().callbacks, [](const auto &callback) {
        return callback.metadata.title.view() == "pipeline_barrier";
      }));
}

TEST(BarrierCompiler, RemovesCompatibleReadAndKeepsHazards) {
  bool inspected = false;
  std::size_t declaration = 0;
  std::size_t first_read = 0;
  std::size_t second_read = 0;
  std::size_t following_write = 0;
  flow::CompilerPlugins plugins;
  rdg::append_plugins(plugins);
  plugins.normalized.emplace_back(
      [&](flow::detail::compiler::NormalizedIR &ir, flow::CompileContext &) {
        inspected = true;
        const auto contains = [&](std::size_t position) {
          return std::ranges::any_of(ir.nodes, [&](const auto &node) {
            return node.source_operation == position;
          });
        };
        EXPECT_FALSE(contains(declaration));
        EXPECT_TRUE(contains(first_read));
        EXPECT_FALSE(contains(second_read));
        EXPECT_TRUE(contains(following_write));
      });

  flow::begin(plugins);
  rdg::begin();

  auto pool = flow::resource<rhi::CommandPool>();
  flow::construct(pool);
  auto image = flow::resource<rhi::Image>();
  flow::construct(image);
  auto command = flow::resource<rhi::CommandBuffer>();
  flow::construct(command);

  declaration = flow::op(flow::intrinsic("strobe.rdg", "decl-image-state", 1)
                             .immediate(rhi::PipelineStage::none)
                             .immediate(rhi::Access::none)
                             .immediate(rhi::ImageLayout::undefined),
                         flow::read(image));
  first_read = flow::op(flow::intrinsic("strobe.rdg", "image-access", 1)
                            .immediate(rhi::PipelineStage::fragment_shader)
                            .immediate(rhi::Access::shader_sampled_read)
                            .immediate(rhi::ImageLayout::read_only),
                        flow::read_write(command), flow::read(image),
                        flow::serialize(pool));
  second_read = flow::op(flow::intrinsic("strobe.rdg", "image-access", 1)
                             .immediate(rhi::PipelineStage::fragment_shader)
                             .immediate(rhi::Access::shader_sampled_read)
                             .immediate(rhi::ImageLayout::read_only),
                         flow::read_write(command), flow::read(image),
                         flow::serialize(pool));
  following_write = flow::op(flow::intrinsic("strobe.rdg", "image-access", 1)
                                 .immediate(rhi::PipelineStage::compute_shader)
                                 .immediate(rhi::Access::shader_storage_write)
                                 .immediate(rhi::ImageLayout::read_only),
                             flow::read_write(command), flow::read(image),
                             flow::serialize(pool));
  flow::op(flow::intrinsic("strobe.rdg", "submit", 1), flow::consume(command),
           flow::serialize(pool));
  rdg::end();

  EXPECT_NO_THROW({ auto program = flow::end(); });
  EXPECT_TRUE(inspected);
}

TEST(BarrierCompiler, WaitsAtFirstImageUsingSubmission) {
  bool inspected = false;
  std::size_t first_submit = 0;
  std::size_t presenting_submit = 0;
  flow::CompilerPlugins plugins;
  rdg::append_plugins(plugins);
  plugins.normalized.emplace_back(
      [&](flow::detail::compiler::NormalizedIR &ir, flow::CompileContext &) {
        inspected = true;
        const auto count_at = [&](std::size_t position) {
          return std::ranges::count_if(ir.nodes, [&](const auto &node) {
            return node.source_operation == position;
          });
        };
        EXPECT_EQ(count_at(first_submit), 2);
        EXPECT_EQ(count_at(presenting_submit), 1);
      });

  flow::begin(plugins);
  rdg::begin();

  auto pool = flow::resource<rhi::CommandPool>();
  flow::construct(pool);
  auto image = flow::resource<rhi::Image>();
  flow::construct(image);
  auto acquired = flow::resource<swapchain_image_runtime>();
  flow::construct(acquired);
  auto first_command = flow::resource<rhi::CommandBuffer>();
  flow::construct(first_command);
  auto presenting_command = flow::resource<rhi::CommandBuffer>();
  flow::construct(presenting_command);

  flow::op(flow::intrinsic("strobe.rdg", "decl-image-state", 1)
               .immediate(rhi::PipelineStage::none)
               .immediate(rhi::Access::none)
               .immediate(rhi::ImageLayout::undefined),
           flow::read(image));
  flow::op(flow::intrinsic("strobe.rdg", "image-access", 1)
               .immediate(rhi::PipelineStage::transfer)
               .immediate(rhi::Access::transfer_write)
               .immediate(rhi::ImageLayout::transfer_dst),
           flow::read_write(first_command), flow::read(image),
           flow::serialize(pool));
  flow::op(flow::intrinsic("strobe.rdg", "image-access", 1)
               .immediate(rhi::PipelineStage::none)
               .immediate(rhi::Access::none)
               .immediate(rhi::ImageLayout::present),
           flow::read_write(presenting_command), flow::read(image),
           flow::serialize(pool));
  flow::op(flow::intrinsic("strobe.rdg", "present", 1), flow::consume(acquired),
           flow::read(image), flow::serialize(presenting_command));
  first_submit = flow::op(flow::intrinsic("strobe.rdg", "submit", 1),
                          flow::consume(first_command), flow::serialize(pool));
  presenting_submit =
      flow::op(flow::intrinsic("strobe.rdg", "submit", 1),
               flow::consume(presenting_command), flow::serialize(pool));
  rdg::end();

  EXPECT_NO_THROW({ auto program = flow::end(); });
  EXPECT_TRUE(inspected);
}

} // namespace
} // namespace strobe::rdg::test
