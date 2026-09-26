#include <gtest/gtest.h>

#include <atomic>
#include <cstddef>
#include <memory>
#include <new>
#include <thread>
#include <vector>

#include <strobe/core/memory/arc.hpp>

namespace {

struct TrackingAllocator {
  struct State {
    std::atomic<std::size_t> allocations = 0;
    std::atomic<std::size_t> deallocations = 0;
  };

  std::shared_ptr<State> state = std::make_shared<State>();

  void *allocate(std::size_t size, std::size_t alignment) {
    state->allocations.fetch_add(1);
    return ::operator new(size, std::align_val_t(alignment));
  }

  void deallocate(void *pointer, std::size_t,
                  std::size_t alignment) noexcept {
    state->deallocations.fetch_add(1);
    ::operator delete(pointer, std::align_val_t(alignment));
  }
};

struct Tracked {
  explicit Tracked(int value) : value(value) {}
  ~Tracked() { destructions.fetch_add(1); }

  int value;
  inline static std::atomic<std::size_t> destructions = 0;
};

using Arc = strobe::Arc<Tracked, TrackingAllocator>;

TEST(Arc, MakesAndSharesAnObject) {
  TrackingAllocator allocator;
  Arc first = strobe::alloc_arc<Tracked, TrackingAllocator>(allocator, 42);
  Arc second = first;

  ASSERT_TRUE(first);
  ASSERT_TRUE(second);
  EXPECT_EQ(first->value, 42);
  EXPECT_EQ(first.use_count(), 2U);
  EXPECT_EQ(second.use_count(), 2U);
  EXPECT_EQ(allocator.state->allocations.load(), 1U);
}

TEST(Arc, DestroysTheObjectAfterTheFinalOwner) {
  TrackingAllocator allocator;
  const auto destructions = Tracked::destructions.load();
  Arc first = strobe::alloc_arc<Tracked, TrackingAllocator>(allocator, 7);
  {
    Arc second = first;
    first.reset();
    EXPECT_EQ(destructions, Tracked::destructions.load());
  }

  EXPECT_EQ(Tracked::destructions.load(), destructions + 1);
  EXPECT_EQ(allocator.state->deallocations.load(), 1U);
}

TEST(Arc, MovesAndResetsOwnership) {
  TrackingAllocator allocator;
  Arc first = strobe::alloc_arc<Tracked, TrackingAllocator>(allocator, 9);
  Arc moved(std::move(first));
  EXPECT_FALSE(first);
  ASSERT_TRUE(moved);

  Arc assigned;
  assigned = std::move(moved);
  EXPECT_FALSE(moved);
  EXPECT_EQ(assigned->value, 9);
  assigned.reset();
  EXPECT_FALSE(assigned);
}

TEST(Arc, SupportsConcurrentCopiesAndReleases) {
  TrackingAllocator allocator;
  Arc source = strobe::alloc_arc<Tracked, TrackingAllocator>(allocator, 13);
  constexpr std::size_t thread_count = 8;
  constexpr std::size_t copies_per_thread = 128;
  std::vector<std::thread> threads;
  threads.reserve(thread_count);

  for (std::size_t thread = 0; thread < thread_count; ++thread) {
    threads.emplace_back([&] {
      for (std::size_t i = 0; i < copies_per_thread; ++i) {
        Arc copy = source;
        ASSERT_EQ(copy->value, 13);
      }
    });
  }
  for (auto &thread : threads) {
    thread.join();
  }

  EXPECT_EQ(source.use_count(), 1U);
}

TEST(Arc, EmptyArcsAreNullable) {
  Arc arc;

  EXPECT_FALSE(arc);
  EXPECT_EQ(arc.get(), nullptr);
  EXPECT_EQ(arc.use_count(), 0U);
}

TEST(Arc, DefaultFactoryUsesMallocator) {
  auto arc = strobe::make_arc<Tracked>(23);

  ASSERT_TRUE(arc);
  EXPECT_EQ(arc->value, 23);
}

} // namespace
