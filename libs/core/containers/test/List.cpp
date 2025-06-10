#include "strobe/core/containers/list.hpp"

#include <gtest/gtest.h>

#include <algorithm>

#include "strobe/core/memory/Mallocator.hpp"

namespace strobe::testing::list {}  // namespace strobe::testing::list

using namespace strobe;
using namespace strobe::testing::list;

TEST(List, BasicAssertions) {
  List<int, Mallocator> list;
  EXPECT_TRUE(list.empty());

  list.push_back(1);
  EXPECT_FALSE(list.empty());
  EXPECT_EQ(list.front(), 1);
  EXPECT_EQ(list.back(), 1);

  list.push_front(0);
  EXPECT_EQ(list.front(), 0);
  EXPECT_EQ(list.back(), 1);

  list.pop_back();
  EXPECT_EQ(list.front(), 0);
  EXPECT_EQ(list.back(), 0);

  list.pop_front();
  EXPECT_TRUE(list.empty());
}

TEST(List, PushBack) {
  List<int, Mallocator> list;

  std::vector<int> ref;
  for (int x = 0; x < 1; ++x) {
    list.push_back(x);
    ref.push_back(x);
  }

  auto refIt = ref.begin();
  for (auto it = list.begin(); it != list.end(); ++it) {
    int v = *it;
    int r = *refIt++;
    ASSERT_EQ(v, r);
  }

  ASSERT_TRUE(std::ranges::equal(ref, list));
}

TEST(List, InsertAndErase) {
  List<int, Mallocator> list;

  auto it = list.insert(list.end(), 10);
  EXPECT_EQ(list.front(), 10);
  EXPECT_EQ(list.back(), 10);

  list.insert(list.begin(), 5);
  EXPECT_EQ(list.front(), 5);
  EXPECT_EQ(list.back(), 10);

  list.insert(list.end(), 15);
  EXPECT_EQ(list.back(), 15);

  it = list.insert(++list.begin(), 7);
  EXPECT_EQ(*it, 7);
  EXPECT_EQ(list.front(), 5);
  EXPECT_EQ(list.back(), 15);

  it = list.erase(it);
  EXPECT_EQ(*it, 10);
  EXPECT_EQ(list.front(), 5);
  EXPECT_EQ(list.back(), 15);

  list.erase(list.begin());
  EXPECT_EQ(list.front(), 10);

  list.clear();
  EXPECT_TRUE(list.empty());
}

TEST(List, Iterators) {
  List<int, Mallocator> list;
  for (int i = 0; i < 5; ++i) {
    list.push_back(i);
  }

  int expected = 0;
  for (auto it = list.begin(); it != list.end(); ++it) {
    EXPECT_EQ(*it, expected++);
  }

  expected = 4;
  for (auto it = list.rbegin(); it != list.rend(); ++it) {
    EXPECT_EQ(*it, expected--);
  }
}

//
TEST(List, ConstIterators) {
  List<int, Mallocator> list;
  list.push_back(1);
  list.push_back(2);
  list.push_back(3);

  auto it = list.cbegin();
  EXPECT_EQ(*it, 1);
  ++it;
  EXPECT_EQ(*it, 2);
  ++it;
  EXPECT_EQ(*it, 3);

  auto rit = list.crbegin();
  EXPECT_EQ(*rit, 3);
  ++rit;
  EXPECT_EQ(*rit, 2);
  ++rit;
  EXPECT_EQ(*rit, 1);
}

TEST(List, FrontAndBack) {
  List<int, Mallocator> list;
  list.push_back(1);
  list.push_back(2);
  list.push_back(3);

  EXPECT_EQ(list.front(), 1);
  EXPECT_EQ(list.back(), 3);

  list.front() = 10;
  list.back() = 30;

  EXPECT_EQ(list.front(), 10);
  EXPECT_EQ(list.back(), 30);
}

TEST(List, Splice) {
  List<int, Mallocator> list1;
  List<int, Mallocator> list2;

  list1.push_back(1);
  list1.push_back(2);
  list1.push_back(3);

  list2.push_back(4);
  list2.push_back(5);

  auto it = list1.begin();
  ++it;

  list1.insert(it, std::move(*list2.begin()));
  list2.pop_front();
  list1.insert(it, std::move(*list2.begin()));
  list2.pop_front();

  EXPECT_EQ(list1.front(), 1);
  EXPECT_EQ(list1.back(), 3);

  EXPECT_EQ(list1.front(), 1);
  EXPECT_EQ(*++list1.begin(), 4);
  EXPECT_EQ(*++ ++list1.begin(), 5);
}

TEST(List, ClearAndEmpty) {
  List<int, Mallocator> list;
  EXPECT_TRUE(list.empty());

  list.push_back(1);
  list.push_back(2);
  list.push_back(3);
  EXPECT_FALSE(list.empty());

  list.clear();
  EXPECT_TRUE(list.empty());
}

TEST(List, StressTest) {
  List<int, Mallocator> list;
  constexpr int N = 100000;

  for (int i = 0; i < N; ++i) {
    list.push_back(i);
  }

  EXPECT_EQ(list.front(), 0);
  EXPECT_EQ(list.back(), N - 1);
  EXPECT_FALSE(list.empty());

  for (int i = 0; i < N; ++i) {
    list.pop_front();
  }

  EXPECT_TRUE(list.empty());
}

namespace strobe::testing::list {

struct DestructibleObject {
  static inline int destruction_count = 0;
  static void reset() { destruction_count = 0; }

  ~DestructibleObject() { ++destruction_count; }
};

}  // namespace strobe::testing::list

using namespace strobe;
using namespace strobe::testing::list;

TEST(List, DestructorTest) {
  DestructibleObject::reset();
  constexpr int count = 100;
  {
    List<DestructibleObject, Mallocator> list;

    for (int i = 0; i < count; ++i) {
      list.push_back(DestructibleObject());
    }

    EXPECT_EQ(DestructibleObject::destruction_count, 100);

    list.clear();
    EXPECT_EQ(DestructibleObject::destruction_count, 100 + count);
  }

  // Once list goes out of scope, all remaining objects should be destroyed
  EXPECT_EQ(DestructibleObject::destruction_count, 100 + count);
}
