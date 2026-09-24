#include <strobe/core/containers/small_vector.hpp>

#include <gtest/gtest.h>

#include <array>
#include <utility>

TEST(SmallVector, stays_inline_until_capacity_is_exceeded) {
  strobe::SmallVector<int, 3> values;

  EXPECT_TRUE(values.using_inline_storage());
  EXPECT_EQ(values.capacity(), 3);

  values.push_back(1);
  values.push_back(2);
  values.push_back(3);
  EXPECT_TRUE(values.using_inline_storage());

  values.push_back(4);
  EXPECT_FALSE(values.using_inline_storage());
  EXPECT_TRUE(std::ranges::equal(values, std::array{1, 2, 3, 4}));
}

TEST(SmallVector, growth_preserves_elements) {
  strobe::SmallVector<int, 2> values;
  for (int i = 0; i < 20; ++i) {
    values.push_back(i);
  }

  ASSERT_EQ(values.size(), 20);
  for (int i = 0; i < 20; ++i) {
    EXPECT_EQ(values[i], i);
  }
}

TEST(SmallVector, move_handles_inline_and_allocated_storage) {
  strobe::SmallVector<int, 2> inlineSource{1, 2};
  strobe::SmallVector<int, 2> inlineMoved(std::move(inlineSource));

  EXPECT_TRUE(inlineSource.empty());
  EXPECT_TRUE(inlineMoved.using_inline_storage());
  EXPECT_TRUE(std::ranges::equal(inlineMoved, std::array{1, 2}));

  strobe::SmallVector<int, 2> heapSource{1, 2, 3};
  const int *allocation = heapSource.data();
  strobe::SmallVector<int, 2> heapMoved(std::move(heapSource));

  EXPECT_TRUE(heapSource.empty());
  EXPECT_TRUE(heapSource.using_inline_storage());
  EXPECT_EQ(heapMoved.data(), allocation);
  EXPECT_TRUE(std::ranges::equal(heapMoved, std::array{1, 2, 3}));
}

TEST(SmallVector, shrink_to_fit_can_return_to_inline_storage) {
  strobe::SmallVector<int, 2> values{1, 2, 3};
  ASSERT_FALSE(values.using_inline_storage());

  values.pop_back();
  values.shrink_to_fit();

  EXPECT_TRUE(values.using_inline_storage());
  EXPECT_TRUE(std::ranges::equal(values, std::array{1, 2}));
}

TEST(SmallVector, shrink_to_fit_reduces_allocated_capacity) {
  strobe::SmallVector<int, 2> values{1, 2, 3};
  values.reserve(20);

  values.shrink_to_fit();

  EXPECT_FALSE(values.using_inline_storage());
  EXPECT_EQ(values.capacity(), values.size());
  EXPECT_TRUE(std::ranges::equal(values, std::array{1, 2, 3}));
}

TEST(SmallVector, insertion_of_aliased_element_survives_growth) {
  strobe::SmallVector<int, 2> values{4, 7};

  values.push_back(values[0]);

  EXPECT_TRUE(std::ranges::equal(values, std::array{4, 7, 4}));
}

TEST(SmallVector, zero_inline_capacity_uses_allocation) {
  strobe::SmallVector<int, 0> values;

  EXPECT_TRUE(values.empty());
  EXPECT_TRUE(values.using_inline_storage());

  values.push_back(42);
  EXPECT_FALSE(values.using_inline_storage());
  EXPECT_EQ(values.front(), 42);
}

namespace {

struct LifetimeStats {
  int constructed = 0;
  int destroyed = 0;
};

struct Tracked {
  LifetimeStats *stats;
  int value;

  Tracked(LifetimeStats &stats, int value) noexcept
      : stats(&stats), value(value) {
    ++stats.constructed;
  }

  Tracked(const Tracked &other) noexcept
      : stats(other.stats), value(other.value) {
    ++stats->constructed;
  }

  Tracked(Tracked &&other) noexcept : stats(other.stats), value(other.value) {
    ++stats->constructed;
    other.value = -1;
  }

  Tracked &operator=(const Tracked &other) noexcept {
    stats = other.stats;
    value = other.value;
    return *this;
  }

  Tracked &operator=(Tracked &&other) noexcept {
    stats = other.stats;
    value = other.value;
    other.value = -1;
    return *this;
  }

  ~Tracked() { ++stats->destroyed; }
};

} // namespace

TEST(SmallVector, copying_inline_and_allocated_vectors_is_independent) {
  strobe::SmallVector<int, 2> inlineSource{1, 2};
  strobe::SmallVector<int, 2> inlineCopy(inlineSource);
  EXPECT_TRUE(inlineCopy.using_inline_storage());
  inlineCopy[0] = 9;
  EXPECT_EQ(inlineSource[0], 1);

  strobe::SmallVector<int, 2> heapSource{1, 2, 3};
  strobe::SmallVector<int, 2> heapCopy(heapSource);
  EXPECT_FALSE(heapCopy.using_inline_storage());
  EXPECT_NE(heapCopy.data(), heapSource.data());
  heapCopy[0] = 9;
  EXPECT_EQ(heapSource[0], 1);
}

TEST(SmallVector, copy_assignment_handles_both_size_directions) {
  strobe::SmallVector<int, 2> values{8};
  strobe::SmallVector<int, 2> large{1, 2, 3, 4};

  values = large;
  EXPECT_TRUE(std::ranges::equal(values, std::array{1, 2, 3, 4}));
  EXPECT_NE(values.data(), large.data());

  strobe::SmallVector<int, 2> small{5};
  values = small;
  EXPECT_TRUE(std::ranges::equal(values, std::array{5}));

  values = values;
  EXPECT_TRUE(std::ranges::equal(values, std::array{5}));
}

TEST(SmallVector, move_assignment_handles_inline_and_allocated_sources) {
  strobe::SmallVector<int, 2> destination{9, 8, 7};

  strobe::SmallVector<int, 2> inlineSource{1, 2};
  destination = std::move(inlineSource);
  EXPECT_TRUE(inlineSource.empty());
  EXPECT_TRUE(std::ranges::equal(destination, std::array{1, 2}));

  strobe::SmallVector<int, 2> heapSource{3, 4, 5};
  const int *allocation = heapSource.data();
  destination = std::move(heapSource);

  EXPECT_TRUE(heapSource.empty());
  EXPECT_EQ(destination.data(), allocation);
  EXPECT_TRUE(std::ranges::equal(destination, std::array{3, 4, 5}));

  destination = std::move(destination);
  EXPECT_TRUE(std::ranges::equal(destination, std::array{3, 4, 5}));
}

TEST(SmallVector, relocation_and_removal_destroy_every_object_once) {
  LifetimeStats stats;

  {
    strobe::SmallVector<Tracked, 2> values;
    values.emplace_back(stats, 1);
    values.emplace_back(stats, 2);
    values.emplace_back(stats, 3); // Relocate inline elements to the heap.

    EXPECT_EQ(values.size(), 3);
    EXPECT_EQ(values[0].value, 1);
    EXPECT_EQ(values[1].value, 2);
    EXPECT_EQ(values[2].value, 3);
    EXPECT_EQ(stats.constructed - stats.destroyed, 3);

    values.pop_back();
    EXPECT_EQ(stats.constructed - stats.destroyed, 2);

    values.shrink_to_fit(); // Relocate back to inline storage.
    EXPECT_TRUE(values.using_inline_storage());
    EXPECT_EQ(stats.constructed - stats.destroyed, 2);

    values.clear();
    EXPECT_EQ(stats.constructed, stats.destroyed);

    values.emplace_back(stats, 4);
    EXPECT_EQ(stats.constructed - stats.destroyed, 1);
  }

  EXPECT_EQ(stats.constructed, stats.destroyed);
}
