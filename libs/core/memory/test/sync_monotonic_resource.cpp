#include <gtest/gtest.h>

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <new>
#include <thread>
#include <unordered_set>
#include <vector>

#include <strobe/core/memory/sync_monotonic_resource.hpp>

namespace {

struct TrackingAllocator {
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

struct FailingAllocator {
  void *allocate(std::size_t, std::size_t) { return nullptr; }
  void deallocate(void *, std::size_t, std::size_t) noexcept {}
};

using Resource = strobe::SyncMonotonicResource<TrackingAllocator>;
static_assert(strobe::Allocator<Resource>);

TEST(SyncMonotonicResource, UsesBootstrapThenNormalChunks) {
  TrackingAllocator upstream;
  Resource resource(upstream, 32, 64);

  ASSERT_NE(resource.allocate(16, 8), nullptr);
  ASSERT_NE(resource.allocate(16, 8), nullptr);
  EXPECT_EQ(upstream.state->allocations.load(), 1U);

  ASSERT_NE(resource.allocate(16, 8), nullptr);
  EXPECT_EQ(upstream.state->allocations.load(), 2U);
}

TEST(SyncMonotonicResource, DedicatedAllocationsDoNotBecomeCurrentChunk) {
  TrackingAllocator upstream;
  Resource resource(upstream, 32, 64);

  ASSERT_NE(resource.allocate(128, 16), nullptr);
  ASSERT_NE(resource.allocate(8, 8), nullptr);
  EXPECT_EQ(upstream.state->allocations.load(), 2U);
  ASSERT_NE(resource.allocate(8, 8), nullptr);
  EXPECT_EQ(upstream.state->allocations.load(), 2U);
}

TEST(SyncMonotonicResource, ConcurrentAllocationsProduceDistinctStorage) {
  Resource resource(TrackingAllocator{}, 64, 64);
  constexpr std::size_t thread_count = 8;
  constexpr std::size_t allocations_per_thread = 64;
  std::vector<void *> pointers(thread_count * allocations_per_thread);
  std::vector<std::thread> threads;
  threads.reserve(thread_count);

  for (std::size_t thread = 0; thread < thread_count; ++thread) {
    threads.emplace_back([&, thread] {
      for (std::size_t i = 0; i < allocations_per_thread; ++i) {
        pointers[thread * allocations_per_thread + i] =
            resource.allocate(8, 8);
      }
    });
  }
  for (auto &thread : threads) {
    thread.join();
  }

  std::unordered_set<void *> unique(pointers.begin(), pointers.end());
  EXPECT_EQ(unique.size(), pointers.size());
}

TEST(SyncMonotonicResource, HonorsAlignmentAndIgnoresIndividualDeallocation) {
  TrackingAllocator upstream;
  Resource resource(upstream, 64, 64);

  void *first = resource.allocate(7, 64);
  ASSERT_NE(first, nullptr);
  EXPECT_EQ(reinterpret_cast<std::uintptr_t>(first) % 64U, 0U);
  resource.deallocate(first, 7, 64);

  void *second = resource.allocate(7, 64);
  ASSERT_NE(second, nullptr);
  EXPECT_NE(second, first);
  EXPECT_EQ(upstream.state->allocations.load(), 2U);
}

TEST(SyncMonotonicResource, ReleaseReclaimsChunksAndRestartsBootstrapPhase) {
  TrackingAllocator upstream;
  Resource resource(upstream, 32, 64);
  ASSERT_NE(resource.allocate(16, 8), nullptr);
  ASSERT_NE(resource.allocate(16, 8), nullptr);

  resource.release();

  EXPECT_EQ(upstream.state->deallocations.load(), 1U);
  ASSERT_NE(resource.allocate(16, 8), nullptr);
  EXPECT_EQ(upstream.state->allocations.load(), 2U);
}

} // namespace
