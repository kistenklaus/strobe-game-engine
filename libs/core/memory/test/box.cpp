#include <gtest/gtest.h>

#include <atomic>
#include <cstddef>
#include <memory>
#include <new>

#include <strobe/core/memory/box.hpp>

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
  ~Tracked() { ++destructions; }

  int value;
  inline static std::atomic<std::size_t> destructions = 0;
};

using Box = strobe::Box<Tracked, TrackingAllocator>;

TEST(Box, MakesAndOwnsAnObject) {
  TrackingAllocator allocator;
  {
    Box box = strobe::make_box<Tracked, TrackingAllocator>(allocator, 42);
    ASSERT_TRUE(box);
    EXPECT_EQ(box->value, 42);
    EXPECT_EQ(box.get()->value, 42);
    EXPECT_EQ(allocator.state->allocations.load(), 1U);
  }
  EXPECT_EQ(allocator.state->deallocations.load(), 1U);
}

TEST(Box, MovesOwnership) {
  TrackingAllocator allocator;
  Box original = strobe::make_box<Tracked, TrackingAllocator>(allocator, 7);
  Box moved(std::move(original));

  EXPECT_FALSE(original);
  ASSERT_TRUE(moved);
  EXPECT_EQ(moved->value, 7);

  Box assigned(allocator);
  assigned = std::move(moved);
  EXPECT_FALSE(moved);
  ASSERT_TRUE(assigned);
  EXPECT_EQ(assigned->value, 7);
}

TEST(Box, ResetDestroysAndReleasesTheObject) {
  TrackingAllocator allocator;
  Box box = strobe::make_box<Tracked, TrackingAllocator>(allocator, 11);
  const auto destructions = Tracked::destructions.load();

  box.reset();

  EXPECT_FALSE(box);
  EXPECT_EQ(Tracked::destructions.load(), destructions + 1);
  EXPECT_EQ(allocator.state->deallocations.load(), 1U);
}

TEST(Box, ReleaseTransfersResponsibility) {
  TrackingAllocator allocator;
  Box box = strobe::make_box<Tracked, TrackingAllocator>(allocator, 19);

  Tracked *pointer = box.release();

  ASSERT_NE(pointer, nullptr);
  EXPECT_FALSE(box);
  EXPECT_EQ(pointer->value, 19);
  std::destroy_at(pointer);
  allocator.deallocate(pointer, sizeof(Tracked), alignof(Tracked));
  EXPECT_EQ(allocator.state->deallocations.load(), 1U);
}

TEST(Box, EmptyBoxRetainsItsAllocator) {
  TrackingAllocator allocator;
  Box box(allocator);

  EXPECT_FALSE(box);
  EXPECT_EQ(box.allocator().state, allocator.state);
  EXPECT_EQ(box.get(), nullptr);
  box.reset();
}

} // namespace
