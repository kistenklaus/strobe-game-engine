#include <gtest/gtest.h>

#include <atomic>
#include <barrier>
#include <cstddef>
#include <memory>
#include <new>
#include <thread>
#include <unordered_set>
#include <vector>

#include <strobe/core/memory/allocator_traits.hpp>
#include <strobe/core/memory/mpmc_monotonic_pool_resource.hpp>

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

static_assert(strobe::Allocator<ThreadSafeAllocator>);
using Pool = strobe::MPMCMonotonicPoolResource<int, ThreadSafeAllocator, 8>;

TEST(MPMCMonotonicPoolResource, ReusesNodesWithoutGrowingAfterDeallocation) {
  ThreadSafeAllocator upstream;
  Pool pool(upstream);
  std::vector<Pool::Node *> nodes;

  for (std::size_t i = 0; i < 8; ++i) {
    nodes.push_back(pool.allocate());
  }
  EXPECT_EQ(upstream.state->allocations.load(), 1U);

  for (Pool::Node *node : nodes) {
    std::construct_at(node->value(), 42);
    EXPECT_EQ(*node->value(), 42);
    std::destroy_at(node->value());
    pool.deallocate(node);
  }

  for (std::size_t i = 0; i < 8; ++i) {
    EXPECT_NE(pool.allocate(), nullptr);
  }
  EXPECT_EQ(upstream.state->allocations.load(), 1U);
}

TEST(MPMCMonotonicPoolResource, SupportsConcurrentAllocationAndDeallocation) {
  ThreadSafeAllocator upstream;
  Pool pool(upstream);

  constexpr std::size_t threadCount = 8;
  constexpr std::size_t nodesPerThread = 64;
  std::vector<Pool::Node *> nodes(threadCount * nodesPerThread);
  std::barrier allocationBarrier(static_cast<std::ptrdiff_t>(threadCount));
  std::barrier deallocationBarrier(static_cast<std::ptrdiff_t>(threadCount));
  std::vector<std::thread> threads;
  threads.reserve(threadCount);

  for (std::size_t thread = 0; thread < threadCount; ++thread) {
    threads.emplace_back([&, thread] {
      const std::size_t offset = thread * nodesPerThread;
      for (std::size_t i = 0; i < nodesPerThread; ++i) {
        nodes[offset + i] = pool.allocate();
      }
      allocationBarrier.arrive_and_wait();
      deallocationBarrier.arrive_and_wait();
      for (std::size_t i = 0; i < nodesPerThread; ++i) {
        pool.deallocate(nodes[offset + i]);
      }
    });
  }

  for (std::thread &thread : threads) {
    thread.join();
  }

  std::unordered_set<Pool::Node *> unique(nodes.begin(), nodes.end());
  EXPECT_EQ(unique.size(), nodes.size());
  EXPECT_GE(upstream.state->allocations.load(), 1U);
}

} // namespace
