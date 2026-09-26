#include <gtest/gtest.h>

#include <atomic>
#include <cstddef>
#include <memory>
#include <new>
#include <utility>

#include <strobe/core/memory/rc.hpp>

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

using Rc = strobe::Rc<Tracked, TrackingAllocator>;

TEST(Rc, MakesAndSharesAnObject) {
  TrackingAllocator allocator;
  Rc first = strobe::alloc_rc<Tracked, TrackingAllocator>(allocator, 42);
  Rc second = first;

  ASSERT_TRUE(first);
  ASSERT_TRUE(second);
  EXPECT_EQ(first->value, 42);
  EXPECT_EQ(first.use_count(), 2U);
  EXPECT_EQ(second.use_count(), 2U);
  EXPECT_EQ(allocator.state->allocations.load(), 1U);
}

TEST(Rc, DestroysTheObjectAfterTheFinalOwner) {
  TrackingAllocator allocator;
  const auto destructions = Tracked::destructions.load();
  Rc first = strobe::alloc_rc<Tracked, TrackingAllocator>(allocator, 7);
  {
    Rc second = first;
    first.reset();
    EXPECT_EQ(destructions, Tracked::destructions.load());
  }

  EXPECT_EQ(Tracked::destructions.load(), destructions + 1);
  EXPECT_EQ(allocator.state->deallocations.load(), 1U);
}

TEST(Rc, MovesAndResetsOwnership) {
  TrackingAllocator allocator;
  Rc first = strobe::alloc_rc<Tracked, TrackingAllocator>(allocator, 9);
  Rc moved(std::move(first));
  EXPECT_FALSE(first);
  ASSERT_TRUE(moved);

  Rc assigned;
  assigned = std::move(moved);
  EXPECT_FALSE(moved);
  EXPECT_EQ(assigned->value, 9);
  assigned.reset();
  EXPECT_FALSE(assigned);
}


TEST(Rc, EmptyRcsAreNullable) {
  Rc rc;

  EXPECT_FALSE(rc);
  EXPECT_EQ(rc.get(), nullptr);
  EXPECT_EQ(rc.use_count(), 0U);
}

TEST(Rc, DefaultFactoryUsesMallocator) {
  auto rc = strobe::make_rc<Tracked>(23);

  ASSERT_TRUE(rc);
  EXPECT_EQ(rc->value, 23);
}

} // namespace
