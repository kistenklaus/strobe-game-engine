#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include "strobe/core/containers/inplace_linear_set.hpp"

namespace strobe {
namespace {

TEST(InplaceLinearSet, StartsEmpty) {
  InplaceLinearSet<int, 4> set;

  EXPECT_TRUE(set.empty());
  EXPECT_EQ(set.size(), 0);
  EXPECT_EQ(set.capacity(), 4);
  EXPECT_EQ(set.begin(), set.end());
  EXPECT_EQ(set.find(1), set.end());
  EXPECT_FALSE(set.contains(1));
  EXPECT_FALSE(set.erase(1));

  static_assert(std::is_const_v<
      std::remove_reference_t<decltype(*set.begin())>>);
}

TEST(InplaceLinearSet, SupportsZeroCapacity) {
  InplaceLinearSet<int, 0> set;

  EXPECT_TRUE(set.empty());
  EXPECT_EQ(set.capacity(), 0);
  EXPECT_EQ(set.begin(), set.end());
}

TEST(InplaceLinearSet, InsertsUpToCapacityWithoutDuplicates) {
  InplaceLinearSet<int, 3> set;

  auto first = set.insert(10);
  auto duplicate = set.insert(10);
  auto second = set.insert(20);
  set.insert(30);

  ASSERT_EQ(set.size(), 3);
  EXPECT_EQ(*first, 10);
  EXPECT_EQ(*duplicate, 10);
  EXPECT_EQ(*second, 20);
  EXPECT_TRUE(set.contains(10));
  EXPECT_TRUE(set.contains(20));
  EXPECT_TRUE(set.contains(30));
  EXPECT_FALSE(set.contains(40));
}

TEST(InplaceLinearSet, FindReturnsMatchingElement) {
  InplaceLinearSet<int, 4> set;
  set.insert(1);
  set.insert(2);

  auto found = set.find(2);

  ASSERT_NE(found, set.end());
  EXPECT_EQ(*found, 2);
  EXPECT_EQ(set.find(3), set.end());
}

TEST(InplaceLinearSet, SupportsHeterogeneousLookup) {
  InplaceLinearSet<std::string, 2> set;
  set.insert(std::string("hello"));

  const std::string_view existing = "hello";
  const std::string_view missing = "missing";

  EXPECT_TRUE(set.contains(existing));
  EXPECT_FALSE(set.contains(missing));
  EXPECT_NE(set.find(existing), set.end());
  EXPECT_TRUE(set.erase(existing));
  EXPECT_TRUE(set.empty());
}

TEST(InplaceLinearSet, IterationIsReadOnly) {
  InplaceLinearSet<int, 3> set;
  set.insert(1);
  set.insert(2);
  set.insert(3);

  int sum = 0;
  for (const int& value : set)
    sum += value;

  EXPECT_EQ(sum, 6);

  static_assert(std::is_same_v<
      decltype(*std::declval<InplaceLinearSet<int, 3>&>().begin()),
      const int&>);
}

TEST(InplaceLinearSet, ReverseIteration) {
  InplaceLinearSet<int, 3> set;
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

TEST(InplaceLinearSet, EraseMovesLastElementIntoRemovedSlot) {
  InplaceLinearSet<int, 3> set;
  set.insert(1);
  set.insert(2);
  set.insert(3);

  auto next = set.erase(set.find(2));

  ASSERT_EQ(set.size(), 2);
  ASSERT_NE(next, set.end());
  EXPECT_EQ(*next, 3);
  EXPECT_EQ(*set.begin(), 1);
  EXPECT_FALSE(set.contains(2));
  EXPECT_TRUE(set.contains(3));
  EXPECT_FALSE(set.erase(2));
}

TEST(InplaceLinearSet, ErasingLastReturnsEnd) {
  InplaceLinearSet<int, 2> set;
  set.insert(1);
  set.insert(2);

  auto next = set.erase(set.find(2));

  EXPECT_EQ(next, set.end());
  EXPECT_EQ(set.size(), 1);
  EXPECT_TRUE(set.contains(1));
}

TEST(InplaceLinearSet, EraseWhileIterating) {
  InplaceLinearSet<int, 10> set;

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

TEST(InplaceLinearSet, ClearAllowsReuse) {
  InplaceLinearSet<std::string, 2> set;
  set.insert("one");
  set.insert("two");

  set.clear();

  EXPECT_TRUE(set.empty());
  EXPECT_FALSE(set.contains("one"));

  set.insert("three");
  EXPECT_EQ(set.size(), 1);
  EXPECT_TRUE(set.contains("three"));
}

TEST(InplaceLinearSet, CopyIsIndependent) {
  InplaceLinearSet<std::string, 3> original;
  original.insert("one");
  original.insert("two");

  InplaceLinearSet<std::string, 3> copied(original);
  copied.erase("one");

  EXPECT_EQ(original.size(), 2);
  EXPECT_TRUE(original.contains("one"));
  EXPECT_EQ(copied.size(), 1);

  InplaceLinearSet<std::string, 3> assigned;
  assigned.insert("old");
  assigned = original;

  EXPECT_EQ(assigned.size(), 2);
  EXPECT_FALSE(assigned.contains("old"));
  EXPECT_TRUE(assigned.contains("two"));

  assigned = assigned;
  EXPECT_EQ(assigned.size(), 2);
}

TEST(InplaceLinearSet, MoveTransfersElements) {
  InplaceLinearSet<std::string, 3> original;
  original.insert("one");
  original.insert("two");

  InplaceLinearSet<std::string, 3> moved(std::move(original));

  EXPECT_TRUE(original.empty());
  EXPECT_EQ(moved.size(), 2);
  EXPECT_TRUE(moved.contains("one"));

  InplaceLinearSet<std::string, 3> assigned;
  assigned.insert("old");
  assigned = std::move(moved);

  EXPECT_TRUE(moved.empty());
  EXPECT_EQ(assigned.size(), 2);
  EXPECT_FALSE(assigned.contains("old"));
  EXPECT_TRUE(assigned.contains("two"));
}

TEST(InplaceLinearSet, SupportsMoveOnlyElements) {
  InplaceLinearSet<std::unique_ptr<int>, 2> set;
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

} // namespace
} // namespace strobe
