#include <strobe/rdg/rdg.hpp>

#include <gtest/gtest.h>

#include <algorithm>
#include <limits>

namespace rdg = strobe::rdg;
namespace flow = strobe::flow;
namespace rhi = strobe::rhi;

TEST(RdgLifetime, ConvertsHalfOpenAndPersistentRanges) {
  EXPECT_EQ(rdg::memory_lifetime({.begin = 2, .end = 7}).begin, 2);
  EXPECT_EQ(rdg::memory_lifetime({.begin = 2, .end = 7}).end, 6);

  const auto persistent = rdg::memory_lifetime(
      {.begin = 0, .end = std::numeric_limits<std::uint32_t>::max()});
  EXPECT_EQ(persistent.begin, 0);
  EXPECT_EQ(persistent.end, std::numeric_limits<std::uint32_t>::max());
}

TEST(RdgResources, RecordsLifetimeAwareRhiConstruction) {
  rdg::begin();
  auto device = rdg::external<rhi::Device>("device");
  auto info = rdg::resource<rhi::BufferInfo>();
  rdg::construct(info, rhi::BufferInfo{.size = 256});
  [[maybe_unused]] auto buffer = rdg::buffer(device, info);
  auto program = rdg::end();

  ASSERT_EQ(program.data().resource_types.size(), 3);
  EXPECT_EQ(program.data().resource_types[0], flow::type_id<rhi::Device>());
  EXPECT_EQ(program.data().resource_types[1], flow::type_id<rhi::BufferInfo>());
  EXPECT_EQ(program.data().resource_types[2], flow::type_id<rhi::Buffer>());
}

TEST(RdgBarrierPlugin, FindsOnlyRhiResourceHazards) {
  auto plugins = rdg::compiler_plugins();

  flow::begin(plugins);
  auto buffer = flow::resource<rhi::Buffer>();
  flow::construct(buffer);
  flow::op(flow::read(buffer), [](const rhi::Buffer &) {});
  auto host = flow::resource<int>();
  flow::construct(host, 1);
  flow::op(flow::read(host), [](const int &) {});
  auto program = flow::end();

  EXPECT_GT(rdg::barrier_plan(program.data()).dependencies.size(), 0);
}

TEST(RdgBarrierPlugin, MaterializesExplicitRhiAccessScopes) {
  rdg::begin();
  auto buffer = rdg::resource<rhi::Buffer>();
  rdg::construct(buffer);
  rdg::op(rdg::write(buffer, rhi::access::transfer_write),
          [](rhi::Buffer &) {});
  rdg::op(rdg::read(buffer, rhi::access::transfer_read),
          [](const rhi::Buffer &) {});
  auto program = rdg::end();

  const auto &plan = rdg::barrier_plan(program.data());
  const auto materialized =
      std::find_if(plan.dependencies.begin(), plan.dependencies.end(),
                   [](const rdg::BarrierDependency &dependency) {
                     return dependency.materialized;
                   });
  ASSERT_NE(materialized, plan.dependencies.end());
  EXPECT_EQ(materialized->barrier.srcStage, rhi::PipelineStage::transfer);
  EXPECT_EQ(materialized->barrier.srcAccess, rhi::Access::transfer_write);
  EXPECT_EQ(materialized->barrier.dstStage, rhi::PipelineStage::transfer);
  EXPECT_EQ(materialized->barrier.dstAccess, rhi::Access::transfer_read);
}
