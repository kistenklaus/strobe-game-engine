#include <gtest/gtest.h>

#include <atomic>
#include <cstddef>
#include <memory>
#include <new>
#include <cstdint>
#include <thread>
#include <vector>

#include <strobe/core/memory/allocator_traits.hpp>
#include <strobe/core/memory/ranked_mpsc_resource.hpp>

namespace {

struct CountingAllocator {
  struct State {
    std::atomic<std::size_t> allocations = 0;
    std::atomic<std::size_t> deallocations = 0;
  };

  std::shared_ptr<State> state = std::make_shared<State>();

  void *allocate(std::size_t size, std::size_t alignment) {
    state->allocations.fetch_add(1, std::memory_order_relaxed);
    return ::operator new(size, std::align_val_t(alignment));
  }

  void deallocate(void *pointer, std::size_t,
                  std::size_t alignment) noexcept {
    state->deallocations.fetch_add(1, std::memory_order_relaxed);
    ::operator delete(pointer, std::align_val_t(alignment));
  }
};

using Resource = strobe::RankedMPSCResource<CountingAllocator, 4, 6>;
static_assert(strobe::Allocator<Resource>);
static_assert(strobe::OverAllocator<Resource>);

TEST(RankedMPSCResource, RoundsRequestsBySizeAndAlignment) {
  CountingAllocator upstream;
  Resource resource(upstream);

  auto [small, small_size] = resource.allocate_at_least(1, 1);
  auto [middle, middle_size] = resource.allocate_at_least(17, 1);
  auto [aligned, aligned_size] = resource.allocate_at_least(8, 32);

  ASSERT_NE(small, nullptr);
  ASSERT_NE(middle, nullptr);
  ASSERT_NE(aligned, nullptr);
  EXPECT_EQ(small_size, 16U);
  EXPECT_EQ(middle_size, 32U);
  EXPECT_EQ(aligned_size, 32U);
  EXPECT_EQ(reinterpret_cast<std::uintptr_t>(aligned) % 32U, 0U);

  resource.deallocate(small, 1, 1);
  resource.deallocate(middle, 17, 1);
  resource.deallocate(aligned, 8, 32);
}

TEST(RankedMPSCResource, ReusesReturnedBlocks) {
  CountingAllocator upstream;
  Resource resource(upstream);

  void *first = resource.allocate(8, 8);
  const auto allocations = upstream.state->allocations.load();
  resource.deallocate(first, 8, 8);
  void *second = resource.allocate(8, 8);

  EXPECT_EQ(second, first);
  EXPECT_EQ(upstream.state->allocations.load(), allocations);
  resource.deallocate(second, 8, 8);
}

TEST(RankedMPSCResource, ForwardsOversizedRequestsUpstream) {
  CountingAllocator upstream;
  Resource resource(upstream);

  auto [pointer, available] = resource.allocate_at_least(65, 8);

  ASSERT_NE(pointer, nullptr);
  EXPECT_EQ(available, 65U);
  EXPECT_EQ(upstream.state->allocations.load(), 1U);
  resource.deallocate(pointer, 65, 8);
  EXPECT_EQ(upstream.state->deallocations.load(), 1U);
}

TEST(RankedMPSCResource, DeallocationMayRunConcurrently) {
  CountingAllocator upstream;
  Resource resource(upstream);
  constexpr std::size_t count = 256;
  constexpr std::size_t thread_count = 8;
  std::vector<void *> pointers;
  pointers.reserve(count);
  for (std::size_t i = 0; i < count; ++i) {
    pointers.push_back(resource.allocate(8, 8));
  }

  std::vector<std::thread> threads;
  threads.reserve(thread_count);
  for (std::size_t thread = 0; thread < thread_count; ++thread) {
    threads.emplace_back([&, thread] {
      for (std::size_t i = thread; i < count; i += thread_count) {
        resource.deallocate(pointers[i], 8, 8);
      }
    });
  }
  for (auto &thread : threads) {
    thread.join();
  }

  EXPECT_EQ(upstream.state->deallocations.load(), 0U);
}

} // namespace
