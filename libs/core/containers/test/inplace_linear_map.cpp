#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <type_traits>
#include <utility>

#include "strobe/core/containers/inplace_linear_map.hpp"

namespace strobe {
namespace {

TEST(InplaceLinearMap, StartsEmpty) {
  InplaceLinearMap<int, std::string, 4> map;

  EXPECT_TRUE(map.empty());
  EXPECT_EQ(map.size(), 0);
  EXPECT_EQ(map.capacity(), 4);
  EXPECT_EQ(map.begin(), map.end());
  EXPECT_TRUE(map.keys().empty());
  EXPECT_TRUE(map.values().empty());
  EXPECT_FALSE(map.containsKey(1));
  EXPECT_FALSE(map.erase(1));
}

TEST(InplaceLinearMap, SupportsZeroCapacity) {
  InplaceLinearMap<int, std::string, 0> map;

  EXPECT_TRUE(map.empty());
  EXPECT_EQ(map.capacity(), 0);
  EXPECT_EQ(map.begin(), map.end());
}

TEST(InplaceLinearMap, InsertReturnsIteratorAndInsertedFlag) {
  InplaceLinearMap<int, std::string, 3> map;

  auto [first, insertedFirst] = map.insert(10, "ten");

  ASSERT_TRUE(insertedFirst);
  ASSERT_NE(first, map.end());
  EXPECT_EQ((*first).first, 10);
  EXPECT_EQ((*first).second, "ten");
  EXPECT_EQ(map.size(), 1);

  auto [duplicate, insertedDuplicate] = map.insert(10, "replacement");

  EXPECT_FALSE(insertedDuplicate);
  ASSERT_NE(duplicate, map.end());
  EXPECT_EQ((*duplicate).first, 10);
  EXPECT_EQ((*duplicate).second, "ten");
  EXPECT_EQ(map.size(), 1);
}

TEST(InplaceLinearMap, InsertUpToCapacity) {
  InplaceLinearMap<int, std::string, 3> map;
  map.insert(10, "ten");
  map.insert(20, "twenty");
  auto [last, inserted] = map.insert(30, "thirty");

  EXPECT_TRUE(inserted);
  ASSERT_EQ(map.size(), map.capacity());
  EXPECT_EQ((*last).first, 30);
  EXPECT_EQ((*last).second, "thirty");

  EXPECT_TRUE(map.containsKey(10));
  EXPECT_TRUE(map.containsKey(20));
  EXPECT_TRUE(map.containsKey(30));
  EXPECT_FALSE(map.containsKey(40));

  EXPECT_EQ(map.keys()[0], 10);
  EXPECT_EQ(map.keys()[1], 20);
  EXPECT_EQ(map.keys()[2], 30);
  EXPECT_EQ(map.values()[0], "ten");
  EXPECT_EQ(map.values()[1], "twenty");
  EXPECT_EQ(map.values()[2], "thirty");
}

TEST(InplaceLinearMap, DuplicateAtCapacityDoesNotAppend) {
  InplaceLinearMap<int, std::string, 2> map;
  map.insert(1, "one");
  map.insert(2, "two");

  ASSERT_EQ(map.size(), map.capacity());

  auto [it, inserted] = map.insert(1, "replacement");

  EXPECT_FALSE(inserted);
  ASSERT_NE(it, map.end());
  EXPECT_EQ((*it).second, "one");
  EXPECT_EQ(map.size(), 2);
  EXPECT_EQ(map.values()[1], "two");
}

TEST(InplaceLinearMap, DuplicateDoesNotMoveRvalueArgument) {
  InplaceLinearMap<int, std::unique_ptr<int>, 1> map;
  map.insert(1, std::make_unique<int>(10));

  auto candidate = std::make_unique<int>(20);
  auto [it, inserted] = map.insert(1, std::move(candidate));

  EXPECT_FALSE(inserted);
  ASSERT_NE(candidate, nullptr);
  EXPECT_EQ(*candidate, 20);
  EXPECT_EQ(*(*it).second, 10);
  EXPECT_EQ(map.size(), 1);
}

TEST(InplaceLinearMap, SupportsRvalueKeys) {
  InplaceLinearMap<std::string, int, 2> map;

  std::string key = "first";
  auto [it, inserted] = map.insert(std::move(key), 10);

  EXPECT_TRUE(inserted);
  EXPECT_EQ((*it).first, "first");
  EXPECT_EQ((*it).second, 10);
  EXPECT_TRUE(map.containsKey("first"));
}

TEST(InplaceLinearMap, IterationPairsReadOnlyKeysWithMutableValues) {
  InplaceLinearMap<int, std::string, 4> map;
  map.insert(1, "one");
  map.insert(2, "two");

  for (auto [key, value] : map)
    value += std::to_string(key);

  EXPECT_EQ(map.values()[0], "one1");
  EXPECT_EQ(map.values()[1], "two2");

  static_assert(std::is_same_v<decltype((*map.begin()).first), const int &>);
  static_assert(std::is_same_v<decltype((*map.begin()).second), std::string &>);
  static_assert(std::is_same_v<decltype(map.keys()), span<const int>>);
  static_assert(std::is_same_v<decltype(map.values()), span<std::string>>);
}

TEST(InplaceLinearMap, ConstIterationExposesConstValues) {
  InplaceLinearMap<int, std::string, 2> map;
  map.insert(7, "seven");

  const auto &constMap = map;
  auto [key, value] = *constMap.begin();

  EXPECT_EQ(key, 7);
  EXPECT_EQ(value, "seven");

  static_assert(std::is_same_v<decltype(key), const int &>);
  static_assert(std::is_same_v<decltype(value), const std::string &>);
  static_assert(
      std::is_same_v<decltype(constMap.values()), span<const std::string>>);
}

TEST(InplaceLinearMap, EraseReplacesRemovedEntryWithLast) {
  InplaceLinearMap<int, std::string, 4> map;
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

TEST(InplaceLinearMap, ClearAllowsReuse) {
  InplaceLinearMap<int, std::string, 2> map;
  map.insert(1, "one");
  map.insert(2, "two");

  map.clear();

  EXPECT_TRUE(map.empty());
  EXPECT_FALSE(map.containsKey(1));

  auto [it, inserted] = map.insert(3, "three");

  ASSERT_TRUE(inserted);
  ASSERT_EQ(map.size(), 1);
  EXPECT_EQ((*it).first, 3);
  EXPECT_EQ((*it).second, "three");
}

TEST(InplaceLinearMap, CopyIsIndependent) {
  InplaceLinearMap<int, std::string, 3> original;
  original.insert(1, "one");
  original.insert(2, "two");

  InplaceLinearMap<int, std::string, 3> copied(original);
  copied.values()[0] = "changed";
  copied.erase(2);

  EXPECT_EQ(original.size(), 2);
  EXPECT_EQ(original.values()[0], "one");
  EXPECT_TRUE(original.containsKey(2));

  InplaceLinearMap<int, std::string, 3> assigned;
  assigned.insert(9, "old");
  assigned = original;

  ASSERT_EQ(assigned.size(), 2);
  EXPECT_FALSE(assigned.containsKey(9));
  EXPECT_EQ(assigned.values()[1], "two");

  assigned = assigned;
  EXPECT_EQ(assigned.size(), 2);
}

TEST(InplaceLinearMap, MoveTransfersEntries) {
  InplaceLinearMap<int, std::string, 3> original;
  original.insert(1, "one");

  InplaceLinearMap<int, std::string, 3> moved(std::move(original));

  ASSERT_EQ(moved.size(), 1);
  EXPECT_EQ(moved.values()[0], "one");
  EXPECT_TRUE(original.empty());

  InplaceLinearMap<int, std::string, 3> assigned;
  assigned.insert(9, "old");
  assigned = std::move(moved);

  ASSERT_EQ(assigned.size(), 1);
  EXPECT_TRUE(assigned.containsKey(1));
  EXPECT_FALSE(assigned.containsKey(9));
  EXPECT_TRUE(moved.empty());
}

TEST(InplaceLinearMap, InsertAliasedValue) {
  InplaceLinearMap<int, std::string, 2> map;
  map.insert(1, "one");

  auto [it, inserted] = map.insert(2, map.values()[0]);

  ASSERT_TRUE(inserted);
  ASSERT_EQ(map.size(), 2);
  EXPECT_EQ((*it).second, "one");
  EXPECT_EQ(map.values()[0], "one");
  EXPECT_EQ(map.values()[1], "one");
}

TEST(InplaceLinearMap, SupportsMoveOnlyValues) {
  InplaceLinearMap<int, std::unique_ptr<int>, 2> map;
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

struct TrackedValue {
  explicit TrackedValue(int &live) : live(&live) { ++*this->live; }

  TrackedValue(const TrackedValue &other) : live(other.live) { ++*live; }

  TrackedValue(TrackedValue &&other) noexcept : live(other.live) { ++*live; }

  TrackedValue &operator=(const TrackedValue &) = default;
  TrackedValue &operator=(TrackedValue &&) = default;

  ~TrackedValue() { --*live; }

  int *live;
};

TEST(InplaceLinearMap, DestroysOnlyOccupiedElements) {
  int live = 0;

  {
    InplaceLinearMap<int, TrackedValue, 8> map;
    EXPECT_EQ(live, 0);

    map.insert(1, TrackedValue(live));
    map.insert(2, TrackedValue(live));
    EXPECT_EQ(live, 2);

    map.erase(1);
    EXPECT_EQ(live, 1);

    map.clear();
    EXPECT_EQ(live, 0);
  }

  EXPECT_EQ(live, 0);
}

} // namespace
} // namespace strobe
