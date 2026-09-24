#include "strobe/core/containers/inplace_vector_deque.hpp"

#include <gtest/gtest.h>

#include <string>
#include <utility>

TEST(InplaceVectorDeque, pushes_and_pops_at_both_ends) {
  strobe::InplaceVectorDeque<int, 4> values;

  EXPECT_TRUE(values.empty());
  EXPECT_EQ(values.capacity(), 4);

  values.push_back(2);
  values.push_front(1);
  values.emplace_back(3);
  values.emplace_front(0);

  ASSERT_TRUE(values.full());
  for (size_t i = 0; i < values.size(); ++i) {
    EXPECT_EQ(values[i], static_cast<int>(i));
  }

  values.pop_front();
  values.pop_back();
  EXPECT_EQ(values.front(), 1);
  EXPECT_EQ(values.back(), 2);

  values.clear();
  EXPECT_TRUE(values.empty());

  values.push_front(7);
  EXPECT_EQ(values.front(), 7);
  EXPECT_EQ(values.back(), 7);
}

TEST(InplaceVectorDeque, preserves_order_when_wrapping) {
  strobe::InplaceVectorDeque<int, 4> values;

  for (int i = 0; i < 4; ++i) {
    values.push_back(i);
  }

  values.pop_front();
  values.pop_front();
  values.push_back(4);
  values.push_back(5);

  ASSERT_EQ(values.size(), 4);
  for (size_t i = 0; i < values.size(); ++i) {
    EXPECT_EQ(values[i], static_cast<int>(i + 2));
  }

  values.pop_back();
  values.push_front(1);
  EXPECT_EQ(values.front(), 1);
  EXPECT_EQ(values.back(), 4);
}

TEST(InplaceVectorDeque, copy_and_move_preserve_values) {
  strobe::InplaceVectorDeque<std::string, 4> source;
  source.push_back("discard");
  source.push_back("middle");
  source.pop_front();
  source.push_front("front");
  source.push_back("back");

  strobe::InplaceVectorDeque<std::string, 4> copy(source);
  ASSERT_EQ(copy.size(), source.size());
  for (size_t i = 0; i < source.size(); ++i) {
    EXPECT_EQ(copy[i], source[i]);
  }

  copy.front() = "changed";
  EXPECT_EQ(source.front(), "front");

  strobe::InplaceVectorDeque<std::string, 4> assigned;
  assigned = source;
  EXPECT_EQ(assigned.front(), "front");
  EXPECT_EQ(assigned.back(), "back");

  strobe::InplaceVectorDeque<std::string, 4> moved(std::move(copy));
  EXPECT_EQ(moved.front(), "changed");
  EXPECT_EQ(moved.back(), "back");

  strobe::InplaceVectorDeque<std::string, 4> move_assigned;
  move_assigned = std::move(assigned);
  EXPECT_EQ(move_assigned.front(), "front");
  EXPECT_EQ(move_assigned.back(), "back");
}

TEST(InplaceVectorDeque, removed_elements_are_destroyed) {
  struct Tracked {
    int *live;
    int value;

    Tracked(int &count, int v) : live(&count), value(v) { ++*live; }
    Tracked(const Tracked &other) : live(other.live), value(other.value) {
      ++*live;
    }
    Tracked(Tracked &&other) noexcept : live(other.live), value(other.value) {
      ++*live;
    }
    ~Tracked() { --*live; }
  };

  int live = 0;
  {
    strobe::InplaceVectorDeque<Tracked, 3> values;
    values.emplace_back(live, 1);
    values.emplace_front(live, 2);
    values.emplace_back(live, 3);
    EXPECT_EQ(live, 3);

    values.pop_front();
    EXPECT_EQ(live, 2);

    values.pop_back();
    EXPECT_EQ(live, 1);

    values.clear();
    EXPECT_EQ(live, 0);
  }
  EXPECT_EQ(live, 0);
}

TEST(InplaceVectorDeque, zero_capacity_stays_empty) {
  strobe::InplaceVectorDeque<int, 0> values;

  EXPECT_EQ(values.capacity(), 0);
  EXPECT_EQ(values.size(), 0);
  EXPECT_TRUE(values.empty());
  EXPECT_TRUE(values.full());
}
