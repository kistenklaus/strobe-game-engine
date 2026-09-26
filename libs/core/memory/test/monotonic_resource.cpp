#include <gtest/gtest.h>

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <new>

#include <strobe/core/memory/allocator_traits.hpp>
#include <strobe/core/memory/monotonic_resource.hpp>

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

using Resource = strobe::MonotonicResource<TrackingAllocator>;
static_assert(strobe::Allocator<Resource>);
static_assert(strobe::Allocator<FailingAllocator>);

TEST(MonotonicResource, UsesBootstrapThenNormalChunks) {
  TrackingAllocator upstream;
  Resource resource(upstream, 32, 64);

  ASSERT_NE(resource.allocate(16, 8), nullptr);
  ASSERT_NE(resource.allocate(16, 8), nullptr);
  EXPECT_EQ(upstream.state->allocations.load(), 1U);

  ASSERT_NE(resource.allocate(16, 8), nullptr);
  EXPECT_EQ(upstream.state->allocations.load(), 2U);
}

TEST(MonotonicResource, DedicatedAllocationsDoNotBecomeCurrentChunk) {
  TrackingAllocator upstream;
  Resource resource(upstream, 32, 64);

  ASSERT_NE(resource.allocate(128, 16), nullptr);
  EXPECT_EQ(upstream.state->allocations.load(), 1U);

  ASSERT_NE(resource.allocate(8, 8), nullptr);
  EXPECT_EQ(upstream.state->allocations.load(), 2U);
  ASSERT_NE(resource.allocate(8, 8), nullptr);
  EXPECT_EQ(upstream.state->allocations.load(), 2U);
}

TEST(MonotonicResource, HonorsRequestedAlignment) {
  Resource resource;

  void *memory = resource.allocate(7, 64);

  ASSERT_NE(memory, nullptr);
  EXPECT_EQ(reinterpret_cast<std::uintptr_t>(memory) % 64U, 0U);
}

TEST(MonotonicResource, IndividualDeallocationDoesNotRecycleStorage) {
  TrackingAllocator upstream;
  Resource resource(upstream, 32, 64);

  void *first = resource.allocate(16, 8);
  ASSERT_NE(first, nullptr);
  resource.deallocate(first, 16, 8);

  void *second = resource.allocate(16, 8);
  ASSERT_NE(second, nullptr);
  EXPECT_NE(second, first);
  EXPECT_EQ(upstream.state->allocations.load(), 1U);
}

TEST(MonotonicResource, ReleaseReclaimsChunksAndRestartsBootstrapPhase) {
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
