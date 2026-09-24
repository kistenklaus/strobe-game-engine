#include <strobe/core/containers/inplace_vector.hpp>

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <utility>

TEST(InplaceVector, push_pop_and_access) {
  strobe::InplaceVector<int, 3> values;

  EXPECT_TRUE(values.empty());
  EXPECT_EQ(values.capacity(), 3);

  values.push_back(10);
  values.emplace_back(20);
  values.push_back(30);

  ASSERT_EQ(values.size(), 3);
  EXPECT_EQ(values.front(), 10);
  EXPECT_EQ(values[1], 20);
  EXPECT_EQ(values.back(), 30);
  EXPECT_EQ(values.at(2), 30);

  values.pop_back();
  EXPECT_EQ(values.size(), 2);
  EXPECT_EQ(values.back(), 20);
}

TEST(InplaceVector, iteration_and_span_expose_elements_in_order) {
  strobe::InplaceVector<int, 4> values;
  values.push_back(4);
  values.push_back(7);
  values.push_back(9);

  EXPECT_TRUE(std::ranges::equal(values, std::array{4, 7, 9}));
  EXPECT_TRUE(std::ranges::equal(values.span(), std::array{4, 7, 9}));

  const std::array reversed{9, 7, 4};
  EXPECT_TRUE(std::ranges::equal(values.rbegin(), values.rend(),
                                 reversed.begin(), reversed.end()));

  values.span()[1] = 8;
  EXPECT_EQ(values[1], 8);
}

TEST(InplaceVector, resize_grows_and_shrinks) {
  strobe::InplaceVector<int, 5> values;

  values.resize(2, 7);
  EXPECT_TRUE(std::ranges::equal(values, std::array{7, 7}));

  values.resize(4);
  EXPECT_TRUE(std::ranges::equal(values, std::array{7, 7, 0, 0}));

  values.resize(1);
  ASSERT_EQ(values.size(), 1);
  EXPECT_EQ(values[0], 7);
}

TEST(InplaceVector, copy_and_move_preserve_values) {
  strobe::InplaceVector<int, 4> source;
  source.push_back(1);
  source.push_back(2);
  source.push_back(3);

  auto copy = source;
  EXPECT_TRUE(std::ranges::equal(copy, std::array{1, 2, 3}));
  copy[0] = 9;
  EXPECT_EQ(source[0], 1);

  strobe::InplaceVector<int, 4> moved(std::move(copy));
  EXPECT_TRUE(copy.empty());
  EXPECT_TRUE(std::ranges::equal(moved, std::array{9, 2, 3}));

  strobe::InplaceVector<int, 4> assigned;
  assigned = source;
  EXPECT_TRUE(std::ranges::equal(assigned, std::array{1, 2, 3}));

  assigned = std::move(moved);
  EXPECT_TRUE(moved.empty());
  EXPECT_TRUE(std::ranges::equal(assigned, std::array{9, 2, 3}));
}

TEST(InplaceVector, assignment_handles_growth_shrinkage_and_self_assignment) {
  strobe::InplaceVector<int, 4> shortValues;
  shortValues.push_back(1);

  strobe::InplaceVector<int, 4> longValues;
  longValues.push_back(2);
  longValues.push_back(3);
  longValues.push_back(4);

  shortValues = longValues;
  EXPECT_TRUE(std::ranges::equal(shortValues, std::array{2, 3, 4}));

  longValues.resize(1);
  shortValues = longValues;
  EXPECT_TRUE(std::ranges::equal(shortValues, std::array{2}));

  shortValues = shortValues;
  EXPECT_TRUE(std::ranges::equal(shortValues, std::array{2}));
}

TEST(InplaceVector, destroys_removed_elements) {
  struct Tracked {
    int *live;

    explicit Tracked(int &count) : live(&count) { ++*live; }
    Tracked(const Tracked &other) : live(other.live) { ++*live; }
    Tracked(Tracked &&other) noexcept : live(other.live) { ++*live; }
    ~Tracked() { --*live; }
  };

  int live = 0;
  {
    strobe::InplaceVector<Tracked, 3> values;
    values.emplace_back(live);
    values.emplace_back(live);
    EXPECT_EQ(live, 2);

    values.pop_back();
    EXPECT_EQ(live, 1);

    values.clear();
    EXPECT_EQ(live, 0);

    values.emplace_back(live);
    EXPECT_EQ(live, 1);
  }
  EXPECT_EQ(live, 0);
}

TEST(InplaceVector, zero_capacity_stays_empty) {
  strobe::InplaceVector<int, 0> values;
  EXPECT_TRUE(values.empty());
  EXPECT_EQ(values.size(), 0);
  EXPECT_EQ(values.capacity(), 0);
}
