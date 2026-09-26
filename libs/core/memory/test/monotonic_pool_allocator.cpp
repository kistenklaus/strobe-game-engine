#include <gtest/gtest.h>

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <new>
#include <ratio>

#include <strobe/core/memory/allocator_traits.hpp>
#include <strobe/core/memory/monotonic_pool_allocator.hpp>

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

using Pool = strobe::MonotonicPoolResource<32, 16, TrackingAllocator>;
static_assert(strobe::Allocator<Pool>);
static_assert(strobe::Allocator<FailingAllocator>);

TEST(MonotonicPoolResource, GrowsAndReusesFixedSizeBlocks) {
  TrackingAllocator upstream;
  Pool pool(upstream);

  void *first = pool.allocate();
  void *second = pool.allocate();

  ASSERT_NE(first, nullptr);
  ASSERT_NE(second, nullptr);
  EXPECT_NE(first, second);
  EXPECT_EQ(upstream.state->allocations.load(), 2U);

  pool.deallocate(first, Pool::block_size, Pool::block_align);
  void *reused = pool.allocate();
  EXPECT_EQ(reused, first);
  EXPECT_EQ(upstream.state->allocations.load(), 2U);
}

TEST(MonotonicPoolResource, ForwardsValidSizeAndAlignment) {
  Pool pool;

  void *memory = pool.allocate(17, 8);

  ASSERT_NE(memory, nullptr);
  EXPECT_EQ(reinterpret_cast<std::uintptr_t>(memory) % 8U, 0U);
  EXPECT_TRUE(pool.owns(memory));
  EXPECT_FALSE(pool.owns(nullptr));
  EXPECT_FALSE(pool.owns(static_cast<std::byte *>(memory) + 1));

  pool.deallocate(memory, 17, 8);
}

TEST(MonotonicPoolResource, ReleaseReturnsAllChunksAndAllowsReuse) {
  TrackingAllocator upstream;
  Pool pool(upstream);
  ASSERT_NE(pool.allocate(), nullptr);
  ASSERT_NE(pool.allocate(), nullptr);
  EXPECT_EQ(upstream.state->allocations.load(), 2U);

  pool.release();

  EXPECT_EQ(upstream.state->deallocations.load(), 2U);
  ASSERT_NE(pool.allocate(), nullptr);
  EXPECT_EQ(upstream.state->allocations.load(), 3U);
}

TEST(MonotonicPoolResource, MoveConstructionTransfersOwnership) {
  TrackingAllocator upstream;
  Pool source(upstream);
  void *memory = source.allocate();

  Pool moved(std::move(source));

  EXPECT_FALSE(source.owns(memory));
  EXPECT_TRUE(moved.owns(memory));
  moved.deallocate(memory, 32, 16);
}

TEST(MonotonicPoolResource, MoveAssignmentReleasesDestinationChunks) {
  TrackingAllocator upstream;
  Pool source(upstream);
  Pool destination(upstream);
  void *memory = source.allocate();
  ASSERT_NE(destination.allocate(), nullptr);
  EXPECT_EQ(upstream.state->allocations.load(), 2U);

  destination = std::move(source);

  EXPECT_EQ(upstream.state->deallocations.load(), 1U);
  EXPECT_TRUE(destination.owns(memory));
  destination.deallocate(memory, 32, 16);
}

TEST(MonotonicPoolResource, CopySelectionCreatesAnIndependentResource) {
  TrackingAllocator upstream;
  const Pool source(upstream);
  Pool selected = source.select_on_container_copy_construction();

  EXPECT_NE(&source, &selected);
  EXPECT_NE(source, selected);
  EXPECT_FALSE(selected.owns(nullptr));
}

} // namespace
