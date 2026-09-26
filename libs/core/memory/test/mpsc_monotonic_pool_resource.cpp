#include <gtest/gtest.h>

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <new>
#include <ratio>
#include <thread>
#include <vector>

#include <strobe/core/memory/allocator_traits.hpp>
#include <strobe/core/memory/mpsc_monotonic_pool_resource.hpp>

namespace {

struct ThreadSafeAllocator {
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

using Pool = strobe::MPSCMonotonicPoolResource<32, 16, ThreadSafeAllocator>;
static_assert(strobe::Allocator<Pool>);
static_assert(strobe::Allocator<FailingAllocator>);

TEST(MPSCMonotonicPoolResource, GrowsChunksAndReusesLocalFreelist) {
  ThreadSafeAllocator upstream;
  Pool pool(upstream);

  void *first = pool.allocate();
  void *second = pool.allocate();
  void *third = pool.allocate();

  ASSERT_NE(first, nullptr);
  ASSERT_NE(second, nullptr);
  ASSERT_NE(third, nullptr);
  EXPECT_EQ(upstream.state->allocations.load(), 2U);
  EXPECT_TRUE(pool.owns(first));
  EXPECT_TRUE(pool.owns(second));
  EXPECT_TRUE(pool.owns(third));
}

TEST(MPSCMonotonicPoolResource, ConcurrentReturnsAreReusedByAllocator) {
  ThreadSafeAllocator upstream;
  Pool pool(upstream);
  constexpr std::size_t nodeCount = 256;
  constexpr std::size_t threadCount = 8;
  std::vector<void *> nodes;
  nodes.reserve(nodeCount);
  for (std::size_t i = 0; i < nodeCount; ++i) {
    nodes.push_back(pool.allocate());
  }
  const auto allocations = upstream.state->allocations.load();

  std::vector<std::thread> threads;
  threads.reserve(threadCount);
  for (std::size_t thread = 0; thread < threadCount; ++thread) {
    threads.emplace_back([&, thread] {
      for (std::size_t i = thread; i < nodeCount; i += threadCount) {
        pool.deallocate(nodes[i]);
      }
    });
  }
  for (std::thread &thread : threads) {
    thread.join();
  }

  for (std::size_t i = 0; i < nodeCount; ++i) {
    ASSERT_NE(pool.allocate(), nullptr);
  }
  EXPECT_EQ(upstream.state->allocations.load(), allocations);
}

TEST(MPSCMonotonicPoolResource, AllocationMayOverlapConcurrentReturns) {
  ThreadSafeAllocator upstream;
  Pool pool(upstream);
  constexpr std::size_t returnedCount = 64;
  std::vector<void *> returned;
  returned.reserve(returnedCount);
  for (std::size_t i = 0; i < returnedCount; ++i) {
    returned.push_back(pool.allocate());
  }

  std::atomic<bool> start = false;
  std::thread deallocator([&] {
    while (!start.load(std::memory_order_acquire)) {
    }
    for (void *pointer : returned) {
      pool.deallocate(pointer);
    }
  });

  start.store(true, std::memory_order_release);
  for (std::size_t i = 0; i < returnedCount; ++i) {
    ASSERT_NE(pool.allocate(), nullptr);
  }
  deallocator.join();
}

TEST(MPSCMonotonicPoolResource, HonorsAlignmentAndOwnershipQueries) {
  Pool pool;
  void *memory = pool.allocate(7, 8);

  ASSERT_NE(memory, nullptr);
  EXPECT_EQ(reinterpret_cast<std::uintptr_t>(memory) % 8U, 0U);
  EXPECT_TRUE(pool.owns(memory));
  EXPECT_FALSE(pool.owns(nullptr));
  EXPECT_TRUE(pool.owns(static_cast<std::byte *>(memory) + 1));
  int outside = 0;
  EXPECT_FALSE(pool.owns(&outside));

  pool.deallocate(memory, 7, 8);
}


} // namespace
