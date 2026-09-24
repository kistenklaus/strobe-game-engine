#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include "strobe/core/containers/linear_set.hpp"

namespace strobe {
namespace {

TEST(LinearSet, StartsEmpty) {
  LinearSet<int> set;

  EXPECT_TRUE(set.empty());
  EXPECT_EQ(set.size(), 0);
  EXPECT_EQ(set.begin(), set.end());
  EXPECT_EQ(set.find(1), set.end());
  EXPECT_FALSE(set.contains(1));
  EXPECT_FALSE(set.erase(1));

  static_assert(std::is_same_v<LinearSet<int>::iterator, const int *>);
}

TEST(LinearSet, InsertDoesNotAddDuplicates) {
  LinearSet<int> set;
  set.reserve(2);

  auto first = set.insert(10);
  auto duplicate = set.insert(10);
  set.insert(20);

  ASSERT_EQ(set.size(), 2);
  EXPECT_EQ(*first, 10);
  EXPECT_EQ(*duplicate, 10);
  EXPECT_TRUE(set.contains(10));
  EXPECT_TRUE(set.contains(20));
  EXPECT_FALSE(set.contains(30));
}

// Verify returned iterators before an insertion that might reallocate.
TEST(LinearSet, InsertReturnsExistingOrInsertedElement) {
  LinearSet<int> set;
  set.reserve(3);

  auto first = set.insert(1);
  EXPECT_EQ(first, set.begin());

  auto second = set.insert(2);
  EXPECT_EQ(second, set.begin() + 1);

  auto existing = set.insert(1);
  EXPECT_EQ(existing, set.begin());
  EXPECT_EQ(set.size(), 2);
}

TEST(LinearSet, GrowsAndPreservesElements) {
  LinearSet<int> set;
  set.reserve(4);

  EXPECT_GE(set.capacity(), 4);

  for (int i = 0; i < 100; ++i)
    set.insert(i);

  ASSERT_EQ(set.size(), 100);

  for (int i = 0; i < 100; ++i)
    EXPECT_TRUE(set.contains(i));
}

TEST(LinearSet, IterationIsReadOnly) {
  LinearSet<int> set;
  set.insert(1);
  set.insert(2);

  int sum = 0;
  for (const int &value : set)
    sum += value;

  EXPECT_EQ(sum, 3);

  static_assert(
      std::is_const_v<std::remove_reference_t<decltype(*set.begin())>>);
  static_assert(
      std::is_same_v<decltype(*std::declval<const LinearSet<int> &>().begin()),
                     const int &>);
}

TEST(LinearSet, SupportsHeterogeneousLookup) {
  LinearSet<std::string> set;
  set.insert(std::string("hello"));

  const std::string_view existing = "hello";
  const std::string_view missing = "missing";

  EXPECT_NE(set.find(existing), set.end());
  EXPECT_TRUE(set.contains(existing));
  EXPECT_FALSE(set.contains(missing));
  EXPECT_TRUE(set.erase(existing));
  EXPECT_TRUE(set.empty());
}

TEST(LinearSet, EraseMovesLastElementIntoRemovedSlot) {
  LinearSet<int> set;
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

TEST(LinearSet, ErasingLastReturnsEnd) {
  LinearSet<int> set;
  set.insert(1);
  set.insert(2);

  auto next = set.erase(set.find(2));

  EXPECT_EQ(next, set.end());
  EXPECT_EQ(set.size(), 1);
  EXPECT_TRUE(set.contains(1));
}

TEST(LinearSet, EraseWhileIterating) {
  LinearSet<int> set;

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

TEST(LinearSet, ClearAllowsReuse) {
  LinearSet<std::string> set;
  set.insert("one");
  set.insert("two");

  set.clear();

  EXPECT_TRUE(set.empty());
  EXPECT_FALSE(set.contains("one"));

  set.insert("three");
  EXPECT_EQ(set.size(), 1);
  EXPECT_TRUE(set.contains("three"));
}

TEST(LinearSet, CopyIsIndependent) {
  LinearSet<std::string> original;
  original.insert("one");
  original.insert("two");

  LinearSet<std::string> copied(original);
  copied.erase("one");

  EXPECT_EQ(original.size(), 2);
  EXPECT_TRUE(original.contains("one"));
  EXPECT_EQ(copied.size(), 1);

  LinearSet<std::string> assigned;
  assigned.insert("old");
  assigned = original;

  EXPECT_EQ(assigned.size(), 2);
  EXPECT_FALSE(assigned.contains("old"));
  EXPECT_TRUE(assigned.contains("two"));

  assigned = assigned;
  EXPECT_EQ(assigned.size(), 2);
}

TEST(LinearSet, MoveTransfersElements) {
  LinearSet<std::string> original;
  original.insert("one");
  original.insert("two");

  LinearSet<std::string> moved(std::move(original));

  EXPECT_TRUE(original.empty());
  EXPECT_EQ(moved.size(), 2);
  EXPECT_TRUE(moved.contains("one"));

  LinearSet<std::string> assigned;
  assigned.insert("old");
  assigned = std::move(moved);

  EXPECT_TRUE(moved.empty());
  EXPECT_EQ(assigned.size(), 2);
  EXPECT_FALSE(assigned.contains("old"));
  EXPECT_TRUE(assigned.contains("two"));
}

TEST(LinearSet, InsertAliasedValueDuringGrowth) {
  LinearSet<std::string> set;
  set.insert("one");

  const std::string &existing = *set.begin();
  set.insert(existing + " more");

  EXPECT_TRUE(set.contains("one"));
  EXPECT_TRUE(set.contains("one more"));
}

TEST(LinearSet, SupportsMoveOnlyElements) {
  LinearSet<std::unique_ptr<int>> set;

  set.insert(std::make_unique<int>(10));
  set.insert(std::make_unique<int>(20));

  ASSERT_EQ(set.size(), 2);
  EXPECT_EQ(**set.begin(), 10);
  EXPECT_EQ(*(set.begin()[1]), 20);

  auto it = set.erase(set.begin());

  ASSERT_EQ(set.size(), 1);
  ASSERT_NE(it, set.end());
  EXPECT_EQ(**it, 20);
}

} // namespace
} // namespace strobe
