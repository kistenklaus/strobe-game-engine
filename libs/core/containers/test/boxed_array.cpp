#include <strobe/core/containers/boxed_array.hpp>

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <utility>

TEST(BoxedArray, size_constructor_default_constructs_elements) {
  strobe::BoxedArray<int> values(3);

  ASSERT_EQ(values.size(), 3);
  EXPECT_FALSE(values.empty());
  EXPECT_TRUE(std::ranges::equal(values, std::array{0, 0, 0}));

  values[1] = 7;
  EXPECT_EQ(values[1], 7);
}

TEST(BoxedArray, range_constructor_preserves_order) {
  const std::array source{4, 7, 9};
  strobe::BoxedArray<int> values(source);

  ASSERT_EQ(values.size(), source.size());
  EXPECT_TRUE(std::ranges::equal(values, source));

  values[0] = 12;
  EXPECT_EQ(source[0], 4);
}

TEST(BoxedArray, copy_is_independent) {
  strobe::BoxedArray<int> original(std::array{1, 2, 3});
  strobe::BoxedArray<int> copy(original);

  EXPECT_TRUE(std::ranges::equal(copy, original));

  copy[0] = 9;
  EXPECT_EQ(original[0], 1);

  strobe::BoxedArray<int> assigned(1);
  assigned = original;
  EXPECT_TRUE(std::ranges::equal(assigned, original));
}

TEST(BoxedArray, move_transfers_elements) {
  strobe::BoxedArray<int> source(std::array{1, 2, 3});
  strobe::BoxedArray<int> moved(std::move(source));

  EXPECT_TRUE(source.empty());
  EXPECT_TRUE(std::ranges::equal(moved, std::array{1, 2, 3}));

  strobe::BoxedArray<int> assigned(1);
  assigned = std::move(moved);

  EXPECT_TRUE(moved.empty());
  EXPECT_TRUE(std::ranges::equal(assigned, std::array{1, 2, 3}));
}

TEST(BoxedArray, zero_size_is_empty) {
  strobe::BoxedArray<int> values(0);

  EXPECT_TRUE(values.empty());
  EXPECT_EQ(values.size(), 0);
  EXPECT_EQ(values.begin(), values.end());
}

TEST(BoxedArray, destroys_its_elements) {
  struct Tracked {
    int *live;

    Tracked() = delete;
    explicit Tracked(int &count) : live(&count) { ++*live; }
    Tracked(const Tracked &other) : live(other.live) { ++*live; }
    ~Tracked() { --*live; }
  };

  int live = 0;
  {
    const std::array source{Tracked(live), Tracked(live)};
    EXPECT_EQ(live, 2);

    {
      strobe::BoxedArray<Tracked> values(source);
      EXPECT_EQ(live, 4);
    }
    EXPECT_EQ(live, 2);
  }
  EXPECT_EQ(live, 0);
}
