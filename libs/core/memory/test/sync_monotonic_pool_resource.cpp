#include <gtest/gtest.h>

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <new>
#include <thread>
#include <unordered_set>
#include <vector>

#include <strobe/core/memory/sync_monotonic_pool_resource.hpp>

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

using Resource = strobe::SyncMonotonicPoolResource<32, 16, CountingAllocator>;
static_assert(strobe::Allocator<Resource>);

TEST(SyncMonotonicPoolResource, SerializesConcurrentAllocations) {
  CountingAllocator upstream;
  Resource resource(upstream);
  constexpr std::size_t thread_count = 8;
  constexpr std::size_t allocations_per_thread = 64;
  std::vector<void *> pointers(thread_count * allocations_per_thread);
  std::vector<std::thread> threads;
  threads.reserve(thread_count);

  for (std::size_t thread = 0; thread < thread_count; ++thread) {
    threads.emplace_back([&, thread] {
      for (std::size_t i = 0; i < allocations_per_thread; ++i) {
        pointers[thread * allocations_per_thread + i] = resource.allocate();
      }
    });
  }
  for (auto &thread : threads) {
    thread.join();
  }

  std::unordered_set<void *> unique(pointers.begin(), pointers.end());
  EXPECT_EQ(unique.size(), pointers.size());
  for (void *pointer : pointers) {
    ASSERT_NE(pointer, nullptr);
    resource.deallocate(pointer);
  }
}

TEST(SyncMonotonicPoolResource, DeallocationMayOverlapAllocation) {
  CountingAllocator upstream;
  Resource resource(upstream);
  constexpr std::size_t count = 128;
  std::vector<void *> pointers;
  pointers.reserve(count);
  for (std::size_t i = 0; i < count; ++i) {
    pointers.push_back(resource.allocate());
  }

  std::atomic<bool> start = false;
  std::thread deallocator([&] {
    while (!start.load(std::memory_order_acquire)) {
    }
    for (void *pointer : pointers) {
      resource.deallocate(pointer);
    }
  });

  start.store(true, std::memory_order_release);
  for (std::size_t i = 0; i < count; ++i) {
    ASSERT_NE(resource.allocate(), nullptr);
  }
  deallocator.join();
}

TEST(SyncMonotonicPoolResource, ReusesReturnedBlocks) {
  CountingAllocator upstream;
  Resource resource(upstream);

  void *first = resource.allocate();
  const auto allocations = upstream.state->allocations.load();
  resource.deallocate(first);
  void *second = resource.allocate();

  EXPECT_EQ(second, first);
  EXPECT_EQ(upstream.state->allocations.load(), allocations);
  resource.deallocate(second);
}

TEST(SyncMonotonicPoolResource, CopySelectionCreatesAnEmptyIndependentPool) {
  CountingAllocator upstream;
  Resource resource(upstream);
  void *original = resource.allocate();
  auto selected = resource.select_on_container_copy_construction();
  void *copy = selected.allocate();

  ASSERT_NE(original, nullptr);
  ASSERT_NE(copy, nullptr);
  EXPECT_NE(copy, original);
  resource.deallocate(original);
  selected.deallocate(copy);
}

} // namespace
