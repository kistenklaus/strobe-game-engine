#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <type_traits>
#include <utility>

#include "strobe/core/containers/linear_map.hpp"

namespace strobe {
namespace {

TEST(LinearMap, StartsEmpty) {
  LinearMap<int, std::string> map;

  EXPECT_TRUE(map.empty());
  EXPECT_EQ(map.size(), 0);
  EXPECT_EQ(map.begin(), map.end());
  EXPECT_EQ(map.find(1), map.end());
  EXPECT_FALSE(map.contains(1));
  EXPECT_FALSE(map.containsKey(1));
  EXPECT_EQ(map.erase(1), 0);
  EXPECT_TRUE(map.keys().empty());
  EXPECT_TRUE(map.values().empty());
}

TEST(LinearMap, InsertReturnsIteratorAndInsertedFlag) {
  LinearMap<int, std::string> map;

  auto [first, insertedFirst] = map.insert(1, "one");

  ASSERT_TRUE(insertedFirst);
  ASSERT_NE(first, map.end());
  EXPECT_EQ((*first).first, 1);
  EXPECT_EQ((*first).second, "one");
  EXPECT_EQ(map.size(), 1);

  auto [duplicate, insertedDuplicate] =
      map.insert(1, "replacement");

  EXPECT_FALSE(insertedDuplicate);
  ASSERT_NE(duplicate, map.end());
  EXPECT_EQ((*duplicate).second, "one");
  EXPECT_EQ(map.size(), 1);

  // The first iterator may have been invalidated by growth; don't use it.
}

TEST(LinearMap, DuplicateDoesNotGrowOrReplaceValue) {
  LinearMap<int, std::string> map;
  map.reserve(2);
  map.insert(1, "one");
  map.insert(2, "two");

  const auto capacity = map.capacity();
  const auto* keys = map.keys().data();
  const auto* values = map.values().data();

  auto [it, inserted] = map.insert(1, "replacement");

  EXPECT_FALSE(inserted);
  EXPECT_EQ((*it).second, "one");
  EXPECT_EQ(map.size(), 2);
  EXPECT_EQ(map.capacity(), capacity);
  EXPECT_EQ(map.keys().data(), keys);
  EXPECT_EQ(map.values().data(), values);
}

TEST(LinearMap, DuplicateDoesNotMoveRvalueArgument) {
  LinearMap<int, std::unique_ptr<int>> map;
  map.insert(1, std::make_unique<int>(10));

  auto candidate = std::make_unique<int>(20);
  auto [it, inserted] = map.insert(1, std::move(candidate));

  EXPECT_FALSE(inserted);
  ASSERT_NE(candidate, nullptr);
  EXPECT_EQ(*candidate, 20);
  EXPECT_EQ(*(*it).second, 10);
}

TEST(LinearMap, FindAndContains) {
  LinearMap<int, std::string> map;
  map.insert(10, "ten");
  map.insert(20, "twenty");

  auto found = map.find(20);
  ASSERT_NE(found, map.end());
  EXPECT_EQ((*found).first, 20);
  EXPECT_EQ((*found).second, "twenty");

  EXPECT_EQ(map.find(30), map.end());
  EXPECT_TRUE(map.contains(10));
  EXPECT_TRUE(map.containsKey(10));
  EXPECT_FALSE(map.contains(30));

  const auto& constMap = map;
  auto constFound = constMap.find(10);
  ASSERT_NE(constFound, constMap.end());
  EXPECT_EQ((*constFound).second, "ten");
}

TEST(LinearMap, SupportsRvalueKeys) {
  LinearMap<std::string, int> map;

  std::string key = "first";
  auto [it, inserted] = map.insert(std::move(key), 10);

  EXPECT_TRUE(inserted);
  EXPECT_EQ((*it).first, "first");
  EXPECT_EQ((*it).second, 10);
  EXPECT_TRUE(map.contains("first"));
}

TEST(LinearMap, TryEmplaceDoesNotConstructDuplicateValue) {
  struct Value {
    explicit Value(int& constructions) : constructions(&constructions) {
      ++constructions;
    }

    int* constructions;
  };

  LinearMap<int, Value> map;
  int constructions = 0;

  auto [first, insertedFirst] =
      map.try_emplace(1, constructions);

  EXPECT_TRUE(insertedFirst);
  EXPECT_EQ(constructions, 1);

  auto [duplicate, insertedDuplicate] =
      map.try_emplace(1, constructions);

  EXPECT_FALSE(insertedDuplicate);
  EXPECT_EQ(constructions, 1);
  EXPECT_EQ((*duplicate).second.constructions, &constructions);

  // `first` could have been invalidated by a later insertion in general.
  (void)first;
}

TEST(LinearMap, InsertOrAssignDistinguishesInsertFromUpdate) {
  LinearMap<int, std::string> map;

  auto [first, inserted] =
      map.insert_or_assign(1, "one");

  EXPECT_TRUE(inserted);
  EXPECT_EQ((*first).second, "one");

  auto [updated, insertedAgain] =
      map.insert_or_assign(1, "ONE");

  EXPECT_FALSE(insertedAgain);
  EXPECT_EQ((*updated).second, "ONE");
  EXPECT_EQ(map.size(), 1);
}

TEST(LinearMap, OperatorBracketDefaultConstructsOnlyOnMiss) {
  LinearMap<int, std::string> map;

  EXPECT_TRUE(map[4].empty());
  EXPECT_EQ(map.size(), 1);

  map[4] = "four";
  EXPECT_EQ(map[4], "four");
  EXPECT_EQ(map.size(), 1);

  map[5] = "five";
  EXPECT_EQ(map.size(), 2);
  EXPECT_EQ((*map.find(5)).second, "five");
}

TEST(LinearMap, IterationPairsReadOnlyKeysWithMutableValues) {
  LinearMap<int, std::string> map;
  map.insert(1, "one");
  map.insert(2, "two");

  for (auto [key, value] : map)
    value += std::to_string(key);

  EXPECT_EQ((*map.find(1)).second, "one1");
  EXPECT_EQ((*map.find(2)).second, "two2");

  static_assert(std::is_same_v<
      decltype((*map.begin()).first), const int&>);
  static_assert(std::is_same_v<
      decltype((*map.begin()).second), std::string&>);
  static_assert(std::is_same_v<
      decltype(map.keys()), span<const int>>);
  static_assert(std::is_same_v<
      decltype(map.values()), span<std::string>>);
}

TEST(LinearMap, ConstIterationExposesConstValues) {
  LinearMap<int, std::string> map;
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

TEST(LinearMap, ReserveAndGrowthPreserveEntries) {
  LinearMap<int, int> map;
  map.reserve(32);

  EXPECT_GE(map.capacity(), 32);

  for (int i = 0; i < 100; ++i)
    EXPECT_TRUE(map.insert(i, i * 10).second);

  ASSERT_EQ(map.size(), 100);

  for (int i = 0; i < 100; ++i) {
    EXPECT_EQ(map.keys()[i], i);
    EXPECT_EQ(map.values()[i], i * 10);
  }
}

TEST(LinearMap, EraseByKeyMovesLastEntryIntoSlot) {
  LinearMap<int, std::string> map;
  map.insert(1, "one");
  map.insert(2, "two");
  map.insert(3, "three");

  EXPECT_EQ(map.erase(2), 1);
  EXPECT_EQ(map.erase(2), 0);

  ASSERT_EQ(map.size(), 2);
  EXPECT_EQ(map.keys()[0], 1);
  EXPECT_EQ(map.values()[0], "one");
  EXPECT_EQ(map.keys()[1], 3);
  EXPECT_EQ(map.values()[1], "three");
}

TEST(LinearMap, EraseIteratorReturnsMovedEntry) {
  LinearMap<int, std::string> map;
  map.insert(1, "one");
  map.insert(2, "two");
  map.insert(3, "three");

  auto next = map.erase(map.find(2));

  ASSERT_NE(next, map.end());
  EXPECT_EQ((*next).first, 3);
  EXPECT_EQ((*next).second, "three");
  EXPECT_FALSE(map.contains(2));
  EXPECT_EQ(map.size(), 2);

  auto afterLast = map.erase(map.find(3));
  EXPECT_EQ(afterLast, map.end());
  EXPECT_EQ(map.size(), 1);
  EXPECT_TRUE(map.contains(1));
}

TEST(LinearMap, EraseWhileIterating) {
  LinearMap<int, int> map;

  for (int i = 0; i < 10; ++i)
    map.insert(i, i);

  for (auto it = map.begin(); it != map.end();) {
    if ((*it).first % 2 == 0)
      it = map.erase(it);
    else
      ++it;
  }

  EXPECT_EQ(map.size(), 5);

  for (int i = 0; i < 10; ++i)
    EXPECT_EQ(map.contains(i), i % 2 != 0);
}

TEST(LinearMap, ClearAllowsReuse) {
  LinearMap<int, std::string> map;
  map.insert(1, "one");
  map.insert(2, "two");

  const auto capacity = map.capacity();
  map.clear();

  EXPECT_TRUE(map.empty());
  EXPECT_EQ(map.capacity(), capacity);
  EXPECT_FALSE(map.contains(1));

  map.insert(3, "three");
  EXPECT_EQ((*map.find(3)).second, "three");
}

TEST(LinearMap, CopyIsIndependent) {
  LinearMap<int, std::string> original;
  original.insert(1, "one");
  original.insert(2, "two");

  LinearMap<int, std::string> copied(original);
  copied.values()[0] = "changed";
  copied.erase(2);

  EXPECT_EQ(original.size(), 2);
  EXPECT_EQ(original.values()[0], "one");
  EXPECT_TRUE(original.contains(2));

  LinearMap<int, std::string> assigned;
  assigned.insert(9, "old");
  assigned = original;

  EXPECT_EQ(assigned.size(), 2);
  EXPECT_FALSE(assigned.contains(9));
  EXPECT_EQ((*assigned.find(2)).second, "two");

  assigned = assigned;
  EXPECT_EQ(assigned.size(), 2);
}

TEST(LinearMap, MoveTransfersEntries) {
  LinearMap<int, std::string> original;
  original.insert(1, "one");
  original.insert(2, "two");

  LinearMap<int, std::string> moved(std::move(original));

  EXPECT_TRUE(original.empty());
  EXPECT_EQ(moved.size(), 2);
  EXPECT_EQ((*moved.find(1)).second, "one");

  LinearMap<int, std::string> assigned;
  assigned.insert(9, "old");
  assigned = std::move(moved);

  EXPECT_TRUE(moved.empty());
  EXPECT_EQ(assigned.size(), 2);
  EXPECT_FALSE(assigned.contains(9));
  EXPECT_EQ((*assigned.find(2)).second, "two");
}

TEST(LinearMap, InsertAliasedValueDuringGrowth) {
  LinearMap<int, std::string> map;
  map.insert(1, "one");

  // Force a growth regardless of allocate_at_least's current capacity.
  while (map.size() < map.capacity()) {
    const int key = static_cast<int>(map.size()) + 1;
    map.insert(key, "filler");
  }

  const std::string expected = map.values()[0];
  const int newKey = static_cast<int>(map.size()) + 1;

  auto [it, inserted] =
      map.insert(newKey, map.values()[0]);

  ASSERT_TRUE(inserted);
  EXPECT_EQ((*it).second, expected);
  EXPECT_EQ(map.values()[0], expected);
}

TEST(LinearMap, SupportsMoveOnlyValues) {
  LinearMap<int, std::unique_ptr<int>> map;
  map.insert(1, std::make_unique<int>(10));
  map.insert(2, std::make_unique<int>(20));

  ASSERT_EQ(map.size(), 2);
  EXPECT_EQ(*(*map.find(1)).second, 10);
  EXPECT_EQ(*(*map.find(2)).second, 20);

  EXPECT_EQ(map.erase(1), 1);
  EXPECT_FALSE(map.contains(1));
  EXPECT_EQ(*(*map.find(2)).second, 20);
}

} // namespace
} // namespace strobe
