#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include "strobe/core/containers/small_linear_set.hpp"

namespace strobe {
namespace {

TEST(SmallLinearSet, StartsWithInlineCapacity) {
  SmallLinearSet<int, 4> set;

  EXPECT_TRUE(set.empty());
  EXPECT_EQ(set.size(), 0);
  EXPECT_EQ(set.capacity(), 4);
  EXPECT_EQ(set.begin(), set.end());
  EXPECT_FALSE(set.contains(1));

  static_assert(
      std::is_const_v<std::remove_reference_t<decltype(*set.begin())>>);
}

TEST(SmallLinearSet, InsertsWithoutDuplicates) {
  SmallLinearSet<int, 2> set;

  auto first = set.insert(10);
  auto duplicate = set.insert(10);

  EXPECT_EQ(*first, 10);
  EXPECT_EQ(*duplicate, 10);
  EXPECT_EQ(set.size(), 1);

  set.insert(20);
  EXPECT_EQ(set.capacity(), 2);

  // Do not use first or duplicate after the next insertion: it may grow.
  set.insert(30);

  EXPECT_EQ(set.size(), 3);
  EXPECT_GE(set.capacity(), 3);
  EXPECT_TRUE(set.contains(10));
  EXPECT_TRUE(set.contains(20));
  EXPECT_TRUE(set.contains(30));
  EXPECT_FALSE(set.contains(40));
}

TEST(SmallLinearSet, ReservePreservesElements) {
  SmallLinearSet<int, 2> set;
  set.insert(1);
  set.insert(2);

  set.reserve(32);

  EXPECT_GE(set.capacity(), 32);
  EXPECT_EQ(set.size(), 2);
  EXPECT_TRUE(set.contains(1));
  EXPECT_TRUE(set.contains(2));
}

TEST(SmallLinearSet, SupportsHeterogeneousLookup) {
  SmallLinearSet<std::string, 2> set;
  set.insert(std::string("hello"));

  const std::string_view existing = "hello";
  const std::string_view missing = "missing";

  EXPECT_NE(set.find(existing), set.end());
  EXPECT_TRUE(set.contains(existing));
  EXPECT_FALSE(set.contains(missing));
  EXPECT_TRUE(set.erase(existing));
  EXPECT_TRUE(set.empty());
}

TEST(SmallLinearSet, IterationIsReadOnly) {
  SmallLinearSet<int, 2> set;
  set.insert(1);
  set.insert(2);
  set.insert(3);

  int sum = 0;
  for (const int &value : set)
    sum += value;

  EXPECT_EQ(sum, 6);

  static_assert(std::is_same_v<
                decltype(*std::declval<SmallLinearSet<int, 2> &>().begin()),
                const int &>);
}

TEST(SmallLinearSet, ReverseIteration) {
  SmallLinearSet<int, 3> set;
  set.insert(1);
  set.insert(2);
  set.insert(3);

  auto it = set.rbegin();
  ASSERT_NE(it, set.rend());
  EXPECT_EQ(*it++, 3);
  EXPECT_EQ(*it++, 2);
  EXPECT_EQ(*it++, 1);
  EXPECT_EQ(it, set.rend());
}

TEST(SmallLinearSet, EraseMovesLastElementIntoRemovedSlot) {
  SmallLinearSet<int, 2> set;
  set.insert(1);
  set.insert(2);
  set.insert(3);

  auto next = set.erase(set.find(2));

  ASSERT_EQ(set.size(), 2);
  ASSERT_NE(next, set.end());
  EXPECT_EQ(*next, 3);
  EXPECT_EQ(*set.begin(), 1);
  EXPECT_FALSE(set.contains(2));
  EXPECT_FALSE(set.erase(2));
}

TEST(SmallLinearSet, ErasingLastReturnsEnd) {
  SmallLinearSet<int, 2> set;
  set.insert(1);
  set.insert(2);

  auto next = set.erase(set.find(2));

  EXPECT_EQ(next, set.end());
  EXPECT_EQ(set.size(), 1);
  EXPECT_TRUE(set.contains(1));
}

TEST(SmallLinearSet, EraseWhileIterating) {
  SmallLinearSet<int, 2> set;

  for (int i = 0; i < 10; ++i)
    set.insert(i);

  for (auto it = set.begin(); it != set.end();) {
    if (*it % 2 == 0)
      it = set.erase(it);
    else
      ++it;
  }

  EXPECT_EQ(set.size(), 5);

  for (int i = 0; i < 10; ++i)
    EXPECT_EQ(set.contains(i), i % 2 != 0);
}

TEST(SmallLinearSet, ClearAllowsReuse) {
  SmallLinearSet<std::string, 1> set;
  set.insert("one");
  set.insert("two");

  set.clear();

  EXPECT_TRUE(set.empty());
  EXPECT_FALSE(set.contains("one"));

  set.insert("three");
  EXPECT_EQ(set.size(), 1);
  EXPECT_TRUE(set.contains("three"));
}

TEST(SmallLinearSet, CopyIsIndependent) {
  SmallLinearSet<std::string, 1> original;
  original.insert("one");
  original.insert("two");

  SmallLinearSet<std::string, 1> copied(original);
  copied.erase("one");

  EXPECT_EQ(original.size(), 2);
  EXPECT_TRUE(original.contains("one"));
  EXPECT_EQ(copied.size(), 1);

  SmallLinearSet<std::string, 1> assigned;
  assigned.insert("old");
  assigned = original;

  EXPECT_EQ(assigned.size(), 2);
  EXPECT_FALSE(assigned.contains("old"));
  EXPECT_TRUE(assigned.contains("two"));

  assigned = assigned;
  EXPECT_EQ(assigned.size(), 2);
}

TEST(SmallLinearSet, MovesInlineElements) {
  SmallLinearSet<std::string, 4> original;
  original.insert("one");
  original.insert("two");

  SmallLinearSet<std::string, 4> moved(std::move(original));

  EXPECT_TRUE(original.empty());
  EXPECT_EQ(moved.size(), 2);
  EXPECT_EQ(moved.capacity(), 4);
  EXPECT_TRUE(moved.contains("one"));

  SmallLinearSet<std::string, 4> assigned;
  assigned.insert("old");
  assigned = std::move(moved);

  EXPECT_TRUE(moved.empty());
  EXPECT_EQ(assigned.size(), 2);
  EXPECT_FALSE(assigned.contains("old"));
  EXPECT_TRUE(assigned.contains("two"));
}

TEST(SmallLinearSet, TransfersHeapStorageWhenMoved) {
  SmallLinearSet<std::string, 1> original;
  original.insert("one");
  original.insert("two");

  const auto *storage = original.begin();

  SmallLinearSet<std::string, 1> moved(std::move(original));

  EXPECT_EQ(moved.begin(), storage);
  EXPECT_TRUE(original.empty());

  SmallLinearSet<std::string, 1> assigned;
  assigned = std::move(moved);

  EXPECT_EQ(assigned.begin(), storage);
  EXPECT_TRUE(moved.empty());
  EXPECT_TRUE(assigned.contains("one"));
  EXPECT_TRUE(assigned.contains("two"));
}

TEST(SmallLinearSet, SupportsMoveOnlyElements) {
  SmallLinearSet<std::unique_ptr<int>, 1> set;
  set.insert(std::make_unique<int>(10));
  set.insert(std::make_unique<int>(20));

  ASSERT_EQ(set.size(), 2);
  EXPECT_EQ(**set.begin(), 10);
  EXPECT_EQ(*(set.begin()[1]), 20);

  auto next = set.erase(set.begin());

  ASSERT_EQ(set.size(), 1);
  ASSERT_NE(next, set.end());
  EXPECT_EQ(**next, 20);
}

TEST(SmallLinearSet, ZeroInlineCapacityGrowsOnFirstInsert) {
  SmallLinearSet<int, 0> set;

  EXPECT_EQ(set.capacity(), 0);

  set.insert(42);

  EXPECT_EQ(set.size(), 1);
  EXPECT_GE(set.capacity(), 1);
  EXPECT_TRUE(set.contains(42));
}

} // namespace
} // namespace strobe
