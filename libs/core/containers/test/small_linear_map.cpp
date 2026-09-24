#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <type_traits>
#include <utility>

#include "strobe/core/containers/small_linear_map.hpp"

namespace strobe {
namespace {

TEST(SmallLinearMap, StartsWithInlineCapacity) {
  SmallLinearMap<int, std::string, 4> map;

  EXPECT_TRUE(map.empty());
  EXPECT_EQ(map.size(), 0);
  EXPECT_EQ(map.capacity(), 4);
  EXPECT_EQ(map.begin(), map.end());
  EXPECT_TRUE(map.keys().empty());
  EXPECT_TRUE(map.values().empty());
}

TEST(SmallLinearMap, InsertReturnsIteratorAndInsertedFlag) {
  SmallLinearMap<int, std::string, 2> map;

  auto [first, insertedFirst] = map.insert(1, "one");
  ASSERT_TRUE(insertedFirst);
  ASSERT_NE(first, map.end());
  EXPECT_EQ((*first).first, 1);
  EXPECT_EQ((*first).second, "one");
  EXPECT_EQ(map.size(), 1);

  auto [duplicate, insertedDuplicate] = map.insert(1, "replacement");
  EXPECT_FALSE(insertedDuplicate);
  ASSERT_NE(duplicate, map.end());
  EXPECT_EQ((*duplicate).first, 1);
  EXPECT_EQ((*duplicate).second, "one");
  EXPECT_EQ(map.size(), 1);
}

TEST(SmallLinearMap, DuplicateAtInlineCapacityDoesNotGrow) {
  SmallLinearMap<int, std::string, 2> map;
  map.insert(1, "one");
  map.insert(2, "two");

  ASSERT_EQ(map.size(), map.capacity());
  const auto* keys = map.keys().data();
  const auto* values = map.values().data();

  auto [it, inserted] = map.insert(1, "replacement");

  EXPECT_FALSE(inserted);
  EXPECT_EQ((*it).second, "one");
  EXPECT_EQ(map.size(), 2);
  EXPECT_EQ(map.capacity(), 2);
  EXPECT_EQ(map.keys().data(), keys);
  EXPECT_EQ(map.values().data(), values);
}

TEST(SmallLinearMap, DuplicateInHeapStoragePreservesValue) {
  SmallLinearMap<int, std::string, 1> map;
  map.insert(1, "one");
  map.insert(2, "two");

  const auto capacity = map.capacity();
  auto [it, inserted] = map.insert(2, "replacement");

  EXPECT_FALSE(inserted);
  EXPECT_EQ((*it).second, "two");
  EXPECT_EQ(map.size(), 2);
  EXPECT_EQ(map.capacity(), capacity);
}

TEST(SmallLinearMap, DuplicateDoesNotMoveRvalueArgument) {
  SmallLinearMap<int, std::unique_ptr<int>, 1> map;
  map.insert(1, std::make_unique<int>(10));

  auto candidate = std::make_unique<int>(20);
  auto [it, inserted] = map.insert(1, std::move(candidate));

  EXPECT_FALSE(inserted);
  ASSERT_NE(candidate, nullptr);
  EXPECT_EQ(*candidate, 20);
  EXPECT_EQ(*(*it).second, 10);
}

TEST(SmallLinearMap, FillsInlineStorageAndGrows) {
  SmallLinearMap<int, std::string, 2> map;
  map.insert(1, "one");
  map.insert(2, "two");

  EXPECT_EQ(map.size(), 2);
  EXPECT_EQ(map.capacity(), 2);

  auto [it, inserted] = map.insert(3, "three");

  ASSERT_TRUE(inserted);
  ASSERT_EQ(map.size(), 3);
  EXPECT_GE(map.capacity(), 3);
  EXPECT_EQ((*it).first, 3);
  EXPECT_EQ((*it).second, "three");

  EXPECT_EQ(map.keys()[0], 1);
  EXPECT_EQ(map.keys()[1], 2);
  EXPECT_EQ(map.keys()[2], 3);
  EXPECT_EQ(map.values()[0], "one");
  EXPECT_EQ(map.values()[1], "two");
  EXPECT_EQ(map.values()[2], "three");
}

TEST(SmallLinearMap, ReservePreservesEntries) {
  SmallLinearMap<int, int, 2> map;
  map.insert(1, 10);
  map.insert(2, 20);

  map.reserve(32);

  EXPECT_GE(map.capacity(), 32);
  ASSERT_EQ(map.size(), 2);
  EXPECT_EQ(map.keys()[0], 1);
  EXPECT_EQ(map.values()[0], 10);
  EXPECT_EQ(map.keys()[1], 2);
  EXPECT_EQ(map.values()[1], 20);
}

TEST(SmallLinearMap, IterationPairsReadOnlyKeysWithMutableValues) {
  SmallLinearMap<int, std::string, 2> map;
  map.insert(1, "one");
  map.insert(2, "two");
  map.insert(3, "three");

  for (auto [key, value] : map)
    value += std::to_string(key);

  EXPECT_EQ(map.values()[0], "one1");
  EXPECT_EQ(map.values()[1], "two2");
  EXPECT_EQ(map.values()[2], "three3");

  static_assert(std::is_same_v<
      decltype((*map.begin()).first), const int&>);
  static_assert(std::is_same_v<
      decltype((*map.begin()).second), std::string&>);
  static_assert(std::is_same_v<
      decltype(map.keys()), span<const int>>);
  static_assert(std::is_same_v<
      decltype(map.values()), span<std::string>>);
}

TEST(SmallLinearMap, ConstIterationExposesConstValues) {
  SmallLinearMap<int, std::string, 2> map;
  map.insert(7, "seven");

  const auto& constMap = map;
  auto [key, value] = *constMap.begin();

  EXPECT_EQ(key, 7);
  EXPECT_EQ(value, "seven");

  static_assert(std::is_same_v<decltype(key), const int&>);
  static_assert(std::is_same_v<decltype(value), const std::string&>);
  static_assert(std::is_same_v<
      decltype(constMap.values()), span<const std::string>>);
}

TEST(SmallLinearMap, EraseReplacesRemovedEntryWithLast) {
  SmallLinearMap<int, std::string, 2> map;
  map.insert(1, "one");
  map.insert(2, "two");
  map.insert(3, "three");

  EXPECT_TRUE(map.erase(2));

  ASSERT_EQ(map.size(), 2);
  EXPECT_FALSE(map.containsKey(2));
  EXPECT_EQ(map.keys()[0], 1);
  EXPECT_EQ(map.values()[0], "one");
  EXPECT_EQ(map.keys()[1], 3);
  EXPECT_EQ(map.values()[1], "three");

  EXPECT_FALSE(map.erase(2));
  EXPECT_TRUE(map.erase(3));
  EXPECT_TRUE(map.erase(1));
  EXPECT_TRUE(map.empty());
}

TEST(SmallLinearMap, ClearAllowsReuse) {
  SmallLinearMap<int, std::string, 2> map;
  map.insert(1, "one");
  map.insert(2, "two");
  map.insert(3, "three");

  map.clear();

  EXPECT_TRUE(map.empty());
  EXPECT_FALSE(map.containsKey(1));

  map.insert(4, "four");
  ASSERT_EQ(map.size(), 1);
  EXPECT_EQ(map.keys()[0], 4);
  EXPECT_EQ(map.values()[0], "four");
}

TEST(SmallLinearMap, CopyIsIndependent) {
  SmallLinearMap<int, std::string, 2> original;
  original.insert(1, "one");
  original.insert(2, "two");
  original.insert(3, "three");

  SmallLinearMap<int, std::string, 2> copied(original);
  copied.values()[0] = "changed";
  copied.erase(2);

  EXPECT_EQ(original.size(), 3);
  EXPECT_EQ(original.values()[0], "one");
  EXPECT_TRUE(original.containsKey(2));

  SmallLinearMap<int, std::string, 2> assigned;
  assigned.insert(9, "old");
  assigned = original;

  ASSERT_EQ(assigned.size(), 3);
  EXPECT_FALSE(assigned.containsKey(9));
  EXPECT_EQ(assigned.values()[2], "three");

  assigned = assigned;
  EXPECT_EQ(assigned.size(), 3);
}

TEST(SmallLinearMap, MoveFromInlineStorage) {
  SmallLinearMap<int, std::string, 4> original;
  original.insert(1, "one");
  original.insert(2, "two");

  SmallLinearMap<int, std::string, 4> moved(std::move(original));

  ASSERT_EQ(moved.size(), 2);
  EXPECT_EQ(moved.capacity(), 4);
  EXPECT_EQ(moved.values()[0], "one");
  EXPECT_EQ(moved.values()[1], "two");
  EXPECT_TRUE(original.empty());

  SmallLinearMap<int, std::string, 4> assigned;
  assigned.insert(9, "old");
  assigned = std::move(moved);

  ASSERT_EQ(assigned.size(), 2);
  EXPECT_TRUE(assigned.containsKey(1));
  EXPECT_FALSE(assigned.containsKey(9));
  EXPECT_TRUE(moved.empty());
}

TEST(SmallLinearMap, MoveFromHeapStorage) {
  SmallLinearMap<int, std::string, 1> original;
  original.insert(1, "one");
  original.insert(2, "two");

  const auto* keysBeforeMove = original.keys().data();
  const auto* valuesBeforeMove = original.values().data();

  SmallLinearMap<int, std::string, 1> moved(std::move(original));

  ASSERT_EQ(moved.size(), 2);
  EXPECT_EQ(moved.keys().data(), keysBeforeMove);
  EXPECT_EQ(moved.values().data(), valuesBeforeMove);
  EXPECT_TRUE(original.empty());

  SmallLinearMap<int, std::string, 1> assigned;
  assigned = std::move(moved);

  ASSERT_EQ(assigned.size(), 2);
  EXPECT_EQ(assigned.keys().data(), keysBeforeMove);
  EXPECT_EQ(assigned.values().data(), valuesBeforeMove);
  EXPECT_TRUE(moved.empty());
}

TEST(SmallLinearMap, InsertAliasedValueDuringGrowth) {
  SmallLinearMap<int, std::string, 1> map;
  map.insert(1, "one");

  auto [it, inserted] = map.insert(2, map.values()[0]);

  ASSERT_TRUE(inserted);
  ASSERT_EQ(map.size(), 2);
  EXPECT_EQ((*it).second, "one");
  EXPECT_EQ(map.values()[0], "one");
  EXPECT_EQ(map.values()[1], "one");
}

TEST(SmallLinearMap, SupportsMoveOnlyValues) {
  SmallLinearMap<int, std::unique_ptr<int>, 1> map;
  map.insert(1, std::make_unique<int>(10));
  map.insert(2, std::make_unique<int>(20));

  ASSERT_EQ(map.size(), 2);
  EXPECT_EQ(*map.values()[0], 10);
  EXPECT_EQ(*map.values()[1], 20);

  EXPECT_TRUE(map.erase(1));

  ASSERT_EQ(map.size(), 1);
  EXPECT_EQ(map.keys()[0], 2);
  EXPECT_EQ(*map.values()[0], 20);
}

TEST(SmallLinearMap, ZeroInlineCapacityGrowsOnFirstInsert) {
  SmallLinearMap<int, int, 0> map;

  EXPECT_EQ(map.capacity(), 0);

  auto [it, inserted] = map.insert(1, 10);

  ASSERT_TRUE(inserted);
  ASSERT_EQ(map.size(), 1);
  EXPECT_GE(map.capacity(), 1);
  EXPECT_EQ((*it).first, 1);
  EXPECT_EQ((*it).second, 10);
}

} // namespace
} // namespace strobe
