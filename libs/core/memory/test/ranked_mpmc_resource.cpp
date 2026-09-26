#include <gtest/gtest.h>

#include <array>
#include <atomic>
#include <barrier>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <new>
#include <thread>
#include <unordered_set>
#include <vector>

#include <strobe/core/memory/allocator_traits.hpp>
#include <strobe/core/memory/ranked_mpmc_resource.hpp>

namespace {

struct ConcurrentCountingAllocator {
  struct State {
    std::atomic<std::size_t> allocations = 0;
    std::atomic<std::size_t> deallocations = 0;
  };

  std::shared_ptr<State> state = std::make_shared<State>();

  void *allocate(std::size_t size, std::size_t alignment) {
    state->allocations.fetch_add(1, std::memory_order_relaxed);
    return ::operator new(size, std::align_val_t(alignment));
  }

  void deallocate(void *pointer, std::size_t, std::size_t alignment) noexcept {
    state->deallocations.fetch_add(1, std::memory_order_relaxed);
    ::operator delete(pointer, std::align_val_t(alignment));
  }
};

using Resource =
    strobe::RankedMPMCResource<ConcurrentCountingAllocator, 4, 6, 8>;
static_assert(strobe::Allocator<Resource>);
static_assert(strobe::OverAllocator<Resource>);

TEST(RankedMPMCResource, RoundsRequestsBySizeAndAlignment) {
  ConcurrentCountingAllocator upstream;
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
  EXPECT_EQ(reinterpret_cast<std::uintptr_t>(small) % 16U, 0U);
  EXPECT_EQ(reinterpret_cast<std::uintptr_t>(middle) % 32U, 0U);
  EXPECT_EQ(reinterpret_cast<std::uintptr_t>(aligned) % 32U, 0U);

  resource.deallocate(small, 1, 1);
  resource.deallocate(middle, 17, 1);
  resource.deallocate(aligned, 8, 32);
}

TEST(RankedMPMCResource, ProvidesWritableStorageAndReusesReturnedBlocks) {
  ConcurrentCountingAllocator upstream;
  Resource resource(upstream);

  auto [first, available] = resource.allocate_at_least(33, 8);
  ASSERT_NE(first, nullptr);
  ASSERT_EQ(available, 64U);

  auto *bytes = static_cast<std::byte *>(first);
  for (std::size_t index = 0; index < available; ++index) {
    bytes[index] = static_cast<std::byte>(index);
  }

  const auto allocations = upstream.state->allocations.load();
  resource.deallocate(first, 33, 8);
  void *second = resource.allocate(33, 8);

  EXPECT_EQ(second, first);
  EXPECT_EQ(upstream.state->allocations.load(), allocations);
  resource.deallocate(second, 33, 8);
}

TEST(RankedMPMCResource, ForwardsOversizedRequestsUpstream) {
  ConcurrentCountingAllocator upstream;
  Resource resource(upstream);

  auto [pointer, available] = resource.allocate_at_least(65, 8);

  ASSERT_NE(pointer, nullptr);
  EXPECT_EQ(available, 65U);
  EXPECT_EQ(upstream.state->allocations.load(), 1U);
  resource.deallocate(pointer, 65, 8);
  EXPECT_EQ(upstream.state->deallocations.load(), 1U);
}

TEST(RankedMPMCResource, SupportsConcurrentAllocationAndDeallocation) {
  ConcurrentCountingAllocator upstream;
  Resource resource(upstream);
  constexpr std::size_t thread_count = 8;
  constexpr std::size_t allocations_per_thread = 128;
  constexpr std::size_t allocation_count =
      thread_count * allocations_per_thread;
  constexpr std::array sizes{std::size_t{8}, std::size_t{24}, std::size_t{48},
                             std::size_t{80}};
  std::array<void *, allocation_count> pointers{};
  std::barrier allocation_phase(static_cast<std::ptrdiff_t>(thread_count));
  std::barrier deallocation_phase(static_cast<std::ptrdiff_t>(thread_count));
  std::vector<std::thread> threads;
  threads.reserve(thread_count);

  for (std::size_t thread = 0; thread < thread_count; ++thread) {
    threads.emplace_back([&, thread] {
      for (std::size_t index = thread; index < allocation_count;
           index += thread_count) {
        pointers[index] = resource.allocate(sizes[index % sizes.size()], 16);
      }

      allocation_phase.arrive_and_wait();
      deallocation_phase.arrive_and_wait();

      const std::size_t source = (thread + 1) % thread_count;
      for (std::size_t index = source; index < allocation_count;
           index += thread_count) {
        resource.deallocate(pointers[index], sizes[index % sizes.size()], 16);
      }
    });
  }

  for (std::thread &thread : threads) {
    thread.join();
  }

  std::unordered_set<void *> unique(pointers.begin(), pointers.end());
  EXPECT_EQ(unique.size(), pointers.size());
}

} // namespace
