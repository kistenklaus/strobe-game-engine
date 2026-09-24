#include <strobe/core/containers/vector.hpp>

#include <gtest/gtest.h>

#include <array>
#include <iterator>
#include <ranges>
#include <string>
#include <utility>

TEST(Vector, default_and_size_construction) {
  strobe::Vector<int> empty;
  EXPECT_TRUE(empty.empty());
  EXPECT_EQ(empty.size(), 0);
  EXPECT_EQ(empty.begin(), empty.end());

  strobe::Vector<int> values(3);
  EXPECT_TRUE(std::ranges::equal(values, std::array{0, 0, 0}));

  strobe::Vector<int> filled(3, 7);
  EXPECT_TRUE(std::ranges::equal(filled, std::array{7, 7, 7}));
}

TEST(Vector, push_emplace_and_pop) {
  strobe::Vector<std::string> values;

  values.push_back("one");
  values.emplace_back(3, 'x');
  values.push_back(std::string{"three"});

  ASSERT_EQ(values.size(), 3);
  EXPECT_EQ(values.front(), "one");
  EXPECT_EQ(values[1], "xxx");
  EXPECT_EQ(values.back(), "three");

  values.pop_back();
  EXPECT_EQ(values.size(), 2);
  EXPECT_EQ(values.back(), "xxx");

  values.clear();
  EXPECT_TRUE(values.empty());
}

TEST(Vector, reserve_preserves_elements) {
  strobe::Vector<int> values(std::array{1, 2, 3});
  values.reserve(100);

  EXPECT_GE(values.capacity(), 100);
  EXPECT_TRUE(std::ranges::equal(values, std::array{1, 2, 3}));

  values.push_back(4);
  EXPECT_TRUE(std::ranges::equal(values, std::array{1, 2, 3, 4}));
}

TEST(Vector, resize_grows_and_shrinks) {
  strobe::Vector<int> values(std::array{1, 2});

  values.resize(4, 9);
  EXPECT_TRUE(std::ranges::equal(values, std::array{1, 2, 9, 9}));

  values.resize(6);
  EXPECT_TRUE(std::ranges::equal(values, std::array{1, 2, 9, 9, 0, 0}));

  values.resize(1);
  EXPECT_TRUE(std::ranges::equal(values, std::array{1}));
}

TEST(Vector, copy_is_independent) {
  strobe::Vector<std::string> source(std::array<std::string, 3>{"a", "b", "c"});

  strobe::Vector<std::string> copy(source);
  EXPECT_TRUE(std::ranges::equal(copy, source));
  EXPECT_NE(copy.data(), source.data());

  copy[0] = "changed";
  EXPECT_EQ(source[0], "a");

  strobe::Vector<std::string> assigned;
  assigned = source;
  EXPECT_TRUE(std::ranges::equal(assigned, source));
  assigned = assigned;
  EXPECT_TRUE(std::ranges::equal(assigned, source));
}

TEST(Vector, move_transfers_allocation) {
  strobe::Vector<int> source(std::array{1, 2, 3});
  const int *allocation = source.data();

  strobe::Vector<int> moved(std::move(source));
  EXPECT_TRUE(source.empty());
  EXPECT_EQ(moved.data(), allocation);
  EXPECT_TRUE(std::ranges::equal(moved, std::array{1, 2, 3}));

  strobe::Vector<int> assigned(std::array{9});
  assigned = std::move(moved);
  EXPECT_TRUE(moved.empty());
  EXPECT_EQ(assigned.data(), allocation);
  EXPECT_TRUE(std::ranges::equal(assigned, std::array{1, 2, 3}));

  assigned = std::move(assigned);
  EXPECT_TRUE(std::ranges::equal(assigned, std::array{1, 2, 3}));
}

TEST(Vector, insert_and_erase_preserve_order) {
  strobe::Vector<int> values(std::array{1, 3, 4});

  auto inserted = values.insert(values.begin() + 1, 2);
  ASSERT_EQ(*inserted, 2);
  EXPECT_TRUE(std::ranges::equal(values, std::array{1, 2, 3, 4}));

  auto next = values.erase(values.begin() + 2);
  ASSERT_NE(next, values.end());
  EXPECT_EQ(*next, 4);
  EXPECT_TRUE(std::ranges::equal(values, std::array{1, 2, 4}));

  values.push_front(0);
  values.pop_front();
  EXPECT_TRUE(std::ranges::equal(values, std::array{1, 2, 4}));
}

TEST(Vector, range_insert_handles_both_capacity_paths) {
  const std::array inserted{7, 8, 9};

  strobe::Vector<int> withCapacity(std::array{1, 2, 3});
  withCapacity.reserve(20);
  withCapacity.insert(withCapacity.begin() + 1, inserted);
  EXPECT_TRUE(std::ranges::equal(withCapacity, std::array{1, 7, 8, 9, 2, 3}));

  strobe::Vector<int> needsGrowth(std::array{1, 2, 3});
  needsGrowth.insert(needsGrowth.begin() + 1, inserted);
  EXPECT_TRUE(std::ranges::equal(needsGrowth, std::array{1, 7, 8, 9, 2, 3}));
}

TEST(Vector, self_append_works_with_and_without_reallocation) {
  strobe::Vector<int> withCapacity(std::array{1, 2, 3});
  withCapacity.reserve(10);
  withCapacity.append(withCapacity);
  EXPECT_TRUE(std::ranges::equal(withCapacity, std::array{1, 2, 3, 1, 2, 3}));

  strobe::Vector<int> needsGrowth(std::array{4, 5, 6});
  needsGrowth.append(needsGrowth);
  EXPECT_TRUE(std::ranges::equal(needsGrowth, std::array{4, 5, 6, 4, 5, 6}));
}

TEST(Vector, self_insert_and_self_assign_preserve_source_values) {
  strobe::Vector<std::string> values(std::array<std::string, 3>{"a", "b", "c"});

  values.insert(values.begin() + 1, values);
  EXPECT_TRUE(std::ranges::equal(
      values, std::array<std::string, 6>{"a", "a", "b", "c", "b", "c"}));

  values.assign(values.begin() + 2, values.begin() + 5);
  EXPECT_TRUE(
      std::ranges::equal(values, std::array<std::string, 3>{"b", "c", "b"}));
}

TEST(Vector, accepts_range_with_distinct_sentinel) {
  int source[]{1, 2, 3};
  auto range = std::ranges::subrange(std::counted_iterator(source, 3),
                                     std::default_sentinel);

  strobe::Vector<int> values(range);
  EXPECT_TRUE(std::ranges::equal(values, std::array{1, 2, 3}));

  values.append(range);
  EXPECT_TRUE(std::ranges::equal(values, std::array{1, 2, 3, 1, 2, 3}));
}

TEST(Vector, removed_elements_are_destroyed) {
  struct Tracked {
    int *live;
    int value;

    Tracked(int &count, int value) noexcept : live(&count), value(value) {
      ++*live;
    }
    Tracked(const Tracked &other) noexcept
        : live(other.live), value(other.value) {
      ++*live;
    }
    Tracked(Tracked &&other) noexcept : live(other.live), value(other.value) {
      ++*live;
    }

    Tracked &operator=(const Tracked &other) noexcept {
      value = other.value;
      return *this;
    }

    Tracked &operator=(Tracked &&other) noexcept {
      value = other.value;
      return *this;
    }

    ~Tracked() { --*live; }
  };

  int live = 0;
  {
    strobe::Vector<Tracked> values;
    values.emplace_back(live, 1);
    values.emplace_back(live, 2);
    values.emplace_back(live, 3);
    EXPECT_EQ(live, 3);

    values.erase(values.begin() + 1);
    EXPECT_EQ(live, 2);
    EXPECT_EQ(values[1].value, 3);

    values.clear();
    EXPECT_EQ(live, 0);
  }
  EXPECT_EQ(live, 0);
}
