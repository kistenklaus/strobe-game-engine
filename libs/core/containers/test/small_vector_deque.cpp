#include "strobe/core/containers/small_vector_deque.hpp"

#include <gtest/gtest.h>

#include <string>
#include <utility>

TEST(SmallVectorDeque, pushes_and_pops_at_both_ends) {
  strobe::SmallVectorDeque<int, 4> values;

  values.push_back(2);
  values.push_front(1);
  values.emplace_back(3);
  values.emplace_front(0);

  ASSERT_EQ(values.size(), 4);
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

TEST(SmallVectorDeque, preserves_order_through_wraparound_and_growth) {
  strobe::SmallVectorDeque<int, 4> values;

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

  values.push_front(1); // Grows beyond inline storage.
  values.push_back(6);

  ASSERT_EQ(values.size(), 6);
  for (size_t i = 0; i < values.size(); ++i) {
    EXPECT_EQ(values[i], static_cast<int>(i + 1));
  }
}

TEST(SmallVectorDeque, copy_and_move_preserve_values) {
  strobe::SmallVectorDeque<std::string, 2> inline_values;
  inline_values.push_back("first");
  inline_values.push_back("second");

  strobe::SmallVectorDeque<std::string, 2> inline_copy(inline_values);
  inline_copy.front() = "changed";
  EXPECT_EQ(inline_values.front(), "first");

  strobe::SmallVectorDeque<std::string, 2> inline_moved(
      std::move(inline_copy));
  EXPECT_EQ(inline_moved.front(), "changed");
  EXPECT_EQ(inline_moved.back(), "second");

  inline_values.push_back("third"); // Now uses heap storage.

  strobe::SmallVectorDeque<std::string, 2> heap_copy;
  heap_copy = inline_values;
  EXPECT_EQ(heap_copy.front(), "first");
  EXPECT_EQ(heap_copy.back(), "third");

  strobe::SmallVectorDeque<std::string, 2> heap_moved;
  heap_moved = std::move(heap_copy);
  ASSERT_EQ(heap_moved.size(), 3);
  EXPECT_EQ(heap_moved.front(), "first");
  EXPECT_EQ(heap_moved.back(), "third");
}

TEST(SmallVectorDeque, insertion_from_an_existing_element_survives_growth) {
  strobe::SmallVectorDeque<std::string, 1> values;
  values.push_back("original");

  values.push_back(values.front()); // Growth from inline storage.
  ASSERT_EQ(values.size(), 2);
  EXPECT_EQ(values.front(), "original");
  EXPECT_EQ(values.back(), "original");

  values.reserve(values.capacity() + 1);
  while (!values.full()) {
    values.push_back("filler");
  }

  values.push_front(values.back()); // Growth from heap storage.
  EXPECT_EQ(values.front(), "filler");
  EXPECT_EQ(values[1], "original");
}

TEST(SmallVectorDeque, removed_elements_are_destroyed) {
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
    strobe::SmallVectorDeque<Tracked, 2> values;
    values.emplace_back(live, 1);
    values.emplace_front(live, 2);
    EXPECT_EQ(live, 2);

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
