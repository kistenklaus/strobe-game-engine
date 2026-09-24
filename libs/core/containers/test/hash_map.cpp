#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <type_traits>
#include <utility>

#include "strobe/core/containers/hash_map.hpp"

namespace strobe {
namespace {

struct ConstantHash {
  std::size_t operator()(int) const noexcept {
    return 0;
  }
};

TEST(HashMap, StartsEmpty) {
  HashMap<int, std::string> map;

  EXPECT_TRUE(map.empty());
  EXPECT_EQ(map.size(), 0);
  EXPECT_EQ(map.capacity(), 0);
  EXPECT_EQ(map.begin(), map.end());
  EXPECT_EQ(map.find(1), map.end());
  EXPECT_FALSE(map.contains(1));
  EXPECT_FALSE(map.erase(1));
}

TEST(HashMap, InsertsAndFindsValues) {
  HashMap<int, std::string> map;

  auto [first, insertedFirst] = map.insert(10, "ten");
  ASSERT_TRUE(insertedFirst);
  EXPECT_EQ((*first).first, 10);
  EXPECT_EQ((*first).second, "ten");

  map.insert(20, "twenty");

  ASSERT_EQ(map.size(), 2);
  EXPECT_TRUE(map.contains(10));
  EXPECT_TRUE(map.contains(20));
  EXPECT_FALSE(map.contains(30));

  auto found = map.find(20);
  ASSERT_NE(found, map.end());
  EXPECT_EQ((*found).first, 20);
  EXPECT_EQ((*found).second, "twenty");
}

TEST(HashMap, DuplicateInsertionPreservesOriginalValue) {
  HashMap<int, std::string> map;
  map.insert(1, "original");

  auto [it, inserted] = map.insert(1, "replacement");

  EXPECT_FALSE(inserted);
  EXPECT_EQ(map.size(), 1);
  ASSERT_NE(it, map.end());
  EXPECT_EQ((*it).second, "original");
}

TEST(HashMap, TryEmplaceDoesNotConstructDuplicateValue) {
  struct Value {
    explicit Value(int& constructions) : constructions(&constructions) {
      ++constructions;
    }

    int* constructions;
  };

  HashMap<int, Value> map;
  int constructions = 0;

  auto [first, insertedFirst] = map.try_emplace(1, constructions);
  EXPECT_TRUE(insertedFirst);
  EXPECT_EQ(constructions, 1);

  auto [duplicate, insertedDuplicate] =
      map.try_emplace(1, constructions);

  EXPECT_FALSE(insertedDuplicate);
  EXPECT_EQ(constructions, 1);
  EXPECT_EQ(first, duplicate);
}

TEST(HashMap, OperatorBracketInsertsDefaultValue) {
  HashMap<int, std::string> map;

  EXPECT_TRUE(map[4].empty());
  map[4] = "four";

  EXPECT_EQ(map.size(), 1);
  EXPECT_EQ(map[4], "four");
  EXPECT_EQ(map.size(), 1);
}

TEST(HashMap, IterationExposesReadOnlyKeysAndMutableValues) {
  HashMap<int, std::string> map;
  map.insert(1, "one");
  map.insert(2, "two");

  int count = 0;
  for (auto [key, value] : map) {
    value += std::to_string(key);
    ++count;
  }

  EXPECT_EQ(count, 2);
  EXPECT_EQ((*map.find(1)).second, "one1");
  EXPECT_EQ((*map.find(2)).second, "two2");

  static_assert(std::is_same_v<
      decltype((*map.begin()).first), const int&>);
  static_assert(std::is_same_v<
      decltype((*map.begin()).second), std::string&>);
}

TEST(HashMap, ConstIterationExposesConstValues) {
  HashMap<int, std::string> map;
  map.insert(7, "seven");

  const auto& constMap = map;
  auto [key, value] = *constMap.begin();

  EXPECT_EQ(key, 7);
  EXPECT_EQ(value, "seven");

  static_assert(std::is_same_v<decltype(key), const int&>);
  static_assert(std::is_same_v<decltype(value), const std::string&>);
}

TEST(HashMap, GrowsAndPreservesEntries) {
  HashMap<int, int> map;

  for (int i = 0; i < 512; ++i)
    EXPECT_TRUE(map.try_emplace(i, i * 10).second);

  ASSERT_EQ(map.size(), 512);

  for (int i = 0; i < 512; ++i) {
    auto it = map.find(i);
    ASSERT_NE(it, map.end()) << i;
    EXPECT_EQ((*it).second, i * 10);
  }
}

TEST(HashMap, ReservePreventsGrowthWithinRequestedCount) {
  HashMap<int, int> map;
  map.reserve(100);

  const auto capacity = map.capacity();
  EXPECT_GE(capacity - capacity / 4, 100);

  for (int i = 0; i < 100; ++i)
    map.insert(i, i);

  EXPECT_EQ(map.capacity(), capacity);
  EXPECT_EQ(map.size(), 100);
}

TEST(HashMap, EraseDoesNotBreakCollisionChain) {
  HashMap<int, int, ConstantHash> map;
  map.insert(1, 10);
  map.insert(2, 20);
  map.insert(3, 30);
  map.insert(4, 40);

  EXPECT_TRUE(map.erase(2));
  EXPECT_FALSE(map.contains(2));

  // These keys were placed after the erased slot.
  EXPECT_EQ((*map.find(3)).second, 30);
  EXPECT_EQ((*map.find(4)).second, 40);

  EXPECT_TRUE(map.try_emplace(5, 50).second);
  EXPECT_EQ((*map.find(5)).second, 50);
  EXPECT_EQ(map.size(), 4);
}

TEST(HashMap, RehashClearsTombstonesWithoutLosingEntries) {
  HashMap<int, int, ConstantHash> map;
  map.reserve(16);

  for (int i = 0; i < 12; ++i)
    map.insert(i, i * 10);

  for (int i = 0; i < 8; ++i)
    EXPECT_TRUE(map.erase(i));

  for (int i = 12; i < 24; ++i)
    map.insert(i, i * 10);

  EXPECT_EQ(map.size(), 16);

  for (int i = 8; i < 24; ++i) {
    auto it = map.find(i);
    ASSERT_NE(it, map.end()) << i;
    EXPECT_EQ((*it).second, i * 10);
  }
}

TEST(HashMap, EraseIteratorReturnsNextOccupiedSlot) {
  HashMap<int, int> map;
  map.insert(1, 10);
  map.insert(2, 20);
  map.insert(3, 30);

  auto it = map.begin();
  ASSERT_NE(it, map.end());

  const int erasedKey = (*it).first;
  auto next = map.erase(it);

  EXPECT_FALSE(map.contains(erasedKey));
  EXPECT_EQ(map.size(), 2);

  // Iteration can continue from the returned iterator.
  int remaining = 0;
  for (; next != map.end(); ++next)
    ++remaining;

  EXPECT_LE(remaining, 2);
}

TEST(HashMap, EraseWhileIterating) {
  HashMap<int, int> map;

  for (int i = 0; i < 100; ++i)
    map.insert(i, i);

  for (auto it = map.begin(); it != map.end();) {
    if ((*it).first % 2 == 0)
      it = map.erase(it);
    else
      ++it;
  }

  EXPECT_EQ(map.size(), 50);

  for (int i = 0; i < 100; ++i)
    EXPECT_EQ(map.contains(i), i % 2 != 0);
}

TEST(HashMap, ClearAllowsReuse) {
  HashMap<int, std::string> map;
  map.insert(1, "one");
  map.insert(2, "two");
  map.erase(1);

  const auto capacity = map.capacity();
  map.clear();

  EXPECT_TRUE(map.empty());
  EXPECT_EQ(map.capacity(), capacity);
  EXPECT_EQ(map.begin(), map.end());
  EXPECT_FALSE(map.contains(2));

  map.insert(3, "three");
  EXPECT_EQ((*map.find(3)).second, "three");
}

TEST(HashMap, CopyIsIndependent) {
  HashMap<int, std::string> original;
  original.insert(1, "one");
  original.insert(2, "two");

  HashMap<int, std::string> copied(original);
  (*copied.find(1)).second = "changed";
  copied.erase(2);

  EXPECT_EQ(original.size(), 2);
  EXPECT_EQ((*original.find(1)).second, "one");
  EXPECT_TRUE(original.contains(2));

  HashMap<int, std::string> assigned;
  assigned.insert(9, "old");
  assigned = original;

  EXPECT_EQ(assigned.size(), 2);
  EXPECT_FALSE(assigned.contains(9));
  EXPECT_EQ((*assigned.find(2)).second, "two");

  assigned = assigned;
  EXPECT_EQ(assigned.size(), 2);
}

TEST(HashMap, MoveTransfersEntries) {
  HashMap<int, std::string> original;
  original.insert(1, "one");
  original.insert(2, "two");

  HashMap<int, std::string> moved(std::move(original));

  EXPECT_TRUE(original.empty());
  EXPECT_EQ(moved.size(), 2);
  EXPECT_EQ((*moved.find(1)).second, "one");

  HashMap<int, std::string> assigned;
  assigned.insert(9, "old");
  assigned = std::move(moved);

  EXPECT_TRUE(moved.empty());
  EXPECT_EQ(assigned.size(), 2);
  EXPECT_FALSE(assigned.contains(9));
  EXPECT_EQ((*assigned.find(2)).second, "two");
}

TEST(HashMap, SupportsMoveOnlyValues) {
  HashMap<int, std::unique_ptr<int>> map;
  map.try_emplace(1, std::make_unique<int>(10));
  map.try_emplace(2, std::make_unique<int>(20));

  ASSERT_EQ(map.size(), 2);
  EXPECT_EQ(*(*map.find(1)).second, 10);
  EXPECT_EQ(*(*map.find(2)).second, 20);

  EXPECT_TRUE(map.erase(1));
  EXPECT_FALSE(map.contains(1));
  EXPECT_EQ(*(*map.find(2)).second, 20);
}

TEST(HashMap, RepeatedInsertEraseMatchesExpectedKeys) {
  HashMap<int, int> map;

  for (int round = 0; round < 10; ++round) {
    for (int i = 0; i < 100; ++i)
      map[i] = round;

    for (int i = 0; i < 100; i += 2)
      EXPECT_TRUE(map.erase(i));

    EXPECT_EQ(map.size(), 50);

    for (int i = 0; i < 100; ++i) {
      EXPECT_EQ(map.contains(i), i % 2 != 0);
      if (i % 2 != 0){
        EXPECT_EQ((*map.find(i)).second, round);
      }
    }
  }
}

TEST(HashMap, DuplicateInsertPreservesValueAndCapacity) {
  HashMap<int, std::string> map;
  map.reserve(6);

  for (int i = 0; i < 6; ++i)
    map.insert(i, std::to_string(i));

  const auto capacity = map.capacity();
  auto [it, inserted] = map.insert(3, "replacement");

  EXPECT_FALSE(inserted);
  EXPECT_EQ((*it).second, "3");
  EXPECT_EQ(map.size(), 6);
  EXPECT_EQ(map.capacity(), capacity);
}

TEST(HashMap, DuplicateInsertDoesNotMoveValue) {
  HashMap<int, std::unique_ptr<int>> map;
  map.insert(1, std::make_unique<int>(10));

  auto candidate = std::make_unique<int>(20);
  auto [it, inserted] = map.insert(1, std::move(candidate));

  EXPECT_FALSE(inserted);
  ASSERT_NE(candidate, nullptr);
  EXPECT_EQ(*candidate, 20);
  EXPECT_EQ(*(*it).second, 10);
}

TEST(HashMap, DuplicateTryEmplaceDoesNotMoveKeyOrValue) {
  HashMap<std::string, std::unique_ptr<int>> map;
  map.try_emplace("key", std::make_unique<int>(10));

  std::string key = "key";
  auto value = std::make_unique<int>(20);

  auto [it, inserted] =
      map.try_emplace(std::move(key), std::move(value));

  EXPECT_FALSE(inserted);
  EXPECT_EQ(key, "key");
  ASSERT_NE(value, nullptr);
  EXPECT_EQ(*value, 20);
  EXPECT_EQ(*(*it).second, 10);
}

TEST(HashMap, OperatorBracketPreservesExistingValue) {
  HashMap<int, std::string> map;
  map.insert(1, "one");

  std::string& existing = map[1];

  EXPECT_EQ(existing, "one");
  EXPECT_EQ(map.size(), 1);

  existing = "ONE";
  EXPECT_EQ((*map.find(1)).second, "ONE");
}

TEST(HashMap, ConstFindAndIterationExposeConstValues) {
  HashMap<int, std::string> map;
  map.insert(1, "one");

  const auto& constMap = map;
  auto found = constMap.find(1);

  ASSERT_NE(found, constMap.end());
  EXPECT_EQ((*found).first, 1);
  EXPECT_EQ((*found).second, "one");

  static_assert(std::is_same_v<
      decltype((*constMap.begin()).first), const int&>);
  static_assert(std::is_same_v<
      decltype((*constMap.begin()).second), const std::string&>);
}

struct CaseInsensitiveHash {
  std::size_t operator()(const std::string& value) const {
    std::size_t hash = 0;

    for (unsigned char c : value) {
      const unsigned char lower =
          c >= 'A' && c <= 'Z'
              ? static_cast<unsigned char>(c - 'A' + 'a')
              : c;

      hash = hash * 131 + lower;
    }

    return hash;
  }
};

struct CaseInsensitiveEqual {
  bool operator()(const std::string& a,
                  const std::string& b) const {
    if (a.size() != b.size())
      return false;

    for (std::size_t i = 0; i < a.size(); ++i) {
      auto lower = [](unsigned char c) {
        return c >= 'A' && c <= 'Z'
            ? static_cast<unsigned char>(c - 'A' + 'a')
            : c;
      };

      if (lower(static_cast<unsigned char>(a[i])) !=
          lower(static_cast<unsigned char>(b[i])))
        return false;
    }

    return true;
  }
};

TEST(HashMap, UsesConfiguredHashAndEqualityForUniqueness) {
  HashMap<std::string, int,
          CaseInsensitiveHash, CaseInsensitiveEqual> map;

  auto [first, insertedFirst] = map.insert("Hello", 10);
  EXPECT_TRUE(insertedFirst);
  EXPECT_EQ((*first).second, 10);

  auto [duplicate, insertedDuplicate] = map.insert("HELLO", 20);
  EXPECT_FALSE(insertedDuplicate);
  EXPECT_EQ((*duplicate).second, 10);
  EXPECT_EQ(map.size(), 1);

  EXPECT_TRUE(map.contains("hello"));
  EXPECT_EQ((*map.find("hElLo")).second, 10);
  EXPECT_TRUE(map.erase("HELLO"));
  EXPECT_TRUE(map.empty());
}

} // namespace
} // namespace strobe
