#include "mock_swapchain_lib.hpp"

#include "strobe/rhi/objects/queue.hpp"

#include <gtest/gtest.h>

#include <array>
#include <memory>

namespace strobe::rdg::test {
namespace {

flow::ExternalOpContext context(flow::Lifetime lifetime,
                                std::array<void *, 2> &arguments) {
  return flow::ExternalOpContext{
      .abi_version = 1,
      .lifetime = lifetime,
      .argument_count = static_cast<std::uint32_t>(arguments.size()),
      .argument_group_count = 0,
      .argument_element_count = 0,
      .immediate_count = 0,
      .arguments = arguments.data(),
      .immediates = nullptr,
      .immediate_types = nullptr,
      .immediate_counts = nullptr,
      .argument_types = nullptr,
      .argument_accesses = nullptr,
      .argument_groups = nullptr,
      .argument_elements = nullptr,
  };
}

TEST(MockSwapchainLib, AcquiresWaitsAndPresentsRingImages) {
  MockSwapchain swapchain;
  swapchain.add(rhi::Image{}, rhi::ImageView{});
  swapchain.add(rhi::Image{}, rhi::ImageView{});

  auto library = make_mock_swapchain_lib();
  auto runtime = swapchain.runtime();
  swapchain_image_runtime first = nullptr;
  std::array<void *, 2> acquire_arguments{&first, &runtime};
  library.resolve(library.state, "acquire",
                  1)(context(flow::Lifetime{1, 2}, acquire_arguments));

  rhi::Queue queue;
  std::array<void *, 2> image_arguments{&queue, &first};
  library.resolve(library.state, "wait",
                  1)(context(flow::Lifetime{3, 4}, image_arguments));
  library.resolve(library.state, "present",
                  1)(context(flow::Lifetime{5, 6}, image_arguments));

  swapchain_image_runtime second = nullptr;
  acquire_arguments[0] = &second;
  library.resolve(library.state, "acquire",
                  1)(context(flow::Lifetime{7, 8}, acquire_arguments));

  const auto events = swapchain.events();
  ASSERT_EQ(events.size(), 4);
  EXPECT_EQ(events[0].kind, MockSwapchainEventKind::acquire);
  EXPECT_EQ(events[0].image, 0);
  EXPECT_EQ(events[1].kind, MockSwapchainEventKind::wait);
  EXPECT_EQ(events[1].image, 0);
  EXPECT_EQ(events[2].kind, MockSwapchainEventKind::present);
  EXPECT_EQ(events[2].image, 0);
  EXPECT_EQ(events[3].kind, MockSwapchainEventKind::acquire);
  EXPECT_EQ(events[3].image, 1);
}

TEST(MockSwapchainLib, ResolvesKnownAbiOnly) {
  const auto library = make_mock_swapchain_lib();
  EXPECT_NE(library.resolve(library.state, "acquire", 1), nullptr);
  EXPECT_NE(library.resolve(library.state, "get_image", 1), nullptr);
  EXPECT_NE(library.resolve(library.state, "get_view", 1), nullptr);
  EXPECT_NE(library.resolve(library.state, "wait", 1), nullptr);
  EXPECT_NE(library.resolve(library.state, "present", 1), nullptr);
  EXPECT_EQ(library.resolve(library.state, "missing", 1), nullptr);
  EXPECT_EQ(library.resolve(library.state, "acquire", 2), nullptr);
}

} // namespace
} // namespace strobe::rdg::test
