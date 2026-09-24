#include "strobe/core/containers/vector_deque.hpp"

#include <gtest/gtest.h>

#include <string>
#include <utility>

TEST(VectorDeque, pushes_and_pops_at_both_ends) {
  strobe::VectorDeque<int> values;

  values.push_back(2);
  values.push_front(1);
  values.emplace_back(3);
  values.emplace_front(0);

  ASSERT_EQ(values.size(), 4);
  for (size_t i = 0; i < values.size(); ++i) {
    EXPECT_EQ(values[i], static_cast<int>(i));
  }
  EXPECT_EQ(values.front(), 0);
  EXPECT_EQ(values.back(), 3);

  values.pop_front();
  values.pop_back();
  EXPECT_EQ(values.front(), 1);
  EXPECT_EQ(values.back(), 2);

  values.pop_front();
  values.pop_back();
  EXPECT_TRUE(values.empty());

  values.push_front(7);
  EXPECT_EQ(values.front(), 7);
  EXPECT_EQ(values.back(), 7);
}

TEST(VectorDeque, preserves_order_through_wraparound_and_growth) {
  strobe::VectorDeque<int> values(4);

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

  values.reserve(values.capacity() + 1);
  values.push_front(1);
  values.push_back(6);

  ASSERT_EQ(values.size(), 6);
  for (size_t i = 0; i < values.size(); ++i) {
    EXPECT_EQ(values[i], static_cast<int>(i + 1));
  }
}

TEST(VectorDeque, copy_and_move_preserve_values) {
  strobe::VectorDeque<std::string> source(3);
  source.push_back("discard");
  source.push_back("middle");
  source.pop_front();
  source.push_front("front");
  source.push_back("back");

  strobe::VectorDeque<std::string> copy(source);
  ASSERT_EQ(copy.size(), source.size());
  for (size_t i = 0; i < source.size(); ++i) {
    EXPECT_EQ(copy[i], source[i]);
  }

  copy.front() = "changed";
  EXPECT_EQ(source.front(), "front");

  strobe::VectorDeque<std::string> assigned;
  assigned = source;
  EXPECT_EQ(assigned.front(), "front");
  EXPECT_EQ(assigned.back(), "back");

  strobe::VectorDeque<std::string> moved(std::move(copy));
  EXPECT_EQ(moved.front(), "changed");
  EXPECT_EQ(moved.back(), "back");

  strobe::VectorDeque<std::string> move_assigned;
  move_assigned = std::move(assigned);
  EXPECT_EQ(move_assigned.front(), "front");
  EXPECT_EQ(move_assigned.back(), "back");
}

TEST(VectorDeque, insertion_from_an_existing_element_survives_growth) {
  strobe::VectorDeque<std::string> values(1);
  values.push_back("original");

  values.push_back(values.front());
  ASSERT_EQ(values.size(), 2);
  EXPECT_EQ(values.front(), "original");
  EXPECT_EQ(values.back(), "original");

  values.reserve(values.capacity() + 1);
  while (!values.full()) {
    values.push_back("filler");
  }

  values.push_front(values.back());
  EXPECT_EQ(values.front(), "filler");
  EXPECT_EQ(values[1], "original");
}

TEST(VectorDeque, removed_elements_are_destroyed) {
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
    strobe::VectorDeque<Tracked> values(2);
    values.emplace_back(live, 1);
    values.emplace_front(live, 2);
    EXPECT_EQ(live, 2);

    values.emplace_back(live, 3); // Growth must destroy relocated objects.
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
