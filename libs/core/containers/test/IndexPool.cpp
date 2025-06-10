#include <gtest/gtest.h>
#include <unordered_set>
#include <strobe/core/containers/index_pool.hpp>
#include "strobe/core/memory/Mallocator.hpp"

namespace strobe::memory::testing::index_pool {

// Basic Initialization Test
TEST(IndexPool, BasicInitialization) {
  strobe::IndexPool<unsigned int, int, strobe::Mallocator> indexPool{};

  unsigned int a = indexPool.insert();
  unsigned int b = indexPool.insert();
  ASSERT_NE(a, b);
  indexPool[a] = 42;
  indexPool[b] = 99;

  ASSERT_EQ(indexPool[a], 42);
  ASSERT_EQ(indexPool[b], 99);
}

// Basic Initialization with Complex Type
TEST(IndexPool, BasicInitializationComplexType) {
  strobe::IndexPool<unsigned int, std::string, strobe::Mallocator> indexPool{};

  unsigned int a = indexPool.insert();
  unsigned int b = indexPool.insert();
  ASSERT_NE(a, b);
  indexPool[a] = "Hello";
  indexPool[b] = "World";

  ASSERT_EQ(indexPool[a], "Hello");
  ASSERT_EQ(indexPool[b], "World");
}

// Insertion and Erasure Test
TEST(IndexPool, InsertEraseTest) {
  strobe::IndexPool<unsigned int, int, strobe::Mallocator> indexPool{};
  std::unordered_set<unsigned int> ids;

  for (int i = 0; i < 10; ++i) {
    unsigned int id = indexPool.insert(i);
    ids.insert(id);
    ASSERT_EQ(indexPool[id], i);
  }

  for (unsigned int id : ids) {
    indexPool.erase(id);
  }

  // Ensure all IDs are freed, and we can insert again
  for (int i = 0; i < 10; ++i) {
    unsigned int id = indexPool.insert(i * 2);
    ASSERT_EQ(indexPool[id], i * 2);
  }
}

// Clear Test
TEST(IndexPool, ClearTest) {
  strobe::IndexPool<unsigned int, std::string, strobe::Mallocator> indexPool{};

  unsigned int a = indexPool.insert("Hello");
  unsigned int b = indexPool.insert("World");

  ASSERT_EQ(indexPool[a], "Hello");
  ASSERT_EQ(indexPool[b], "World");

  indexPool.clear();
  
  // After clear, all IDs should be invalid
#ifndef NDEBUG
  ASSERT_DEATH(indexPool[a], ".*");
  ASSERT_DEATH(indexPool[b], ".*");
#endif
}

// Reuse of Erased Indices
TEST(IndexPool, ReuseOfErasedIndices) {
  strobe::IndexPool<unsigned int, int, strobe::Mallocator> indexPool{};
  std::unordered_set<unsigned int> ids;

  for (int i = 0; i < 100; ++i) {
    ids.insert(indexPool.insert(i));
  }

  // Erase half of them
  int count = 0;
  for (unsigned int id : ids) {
    if (count++ % 2 == 0) {
      indexPool.erase(id);
    }
  }

  // Insert again, ensuring all remaining IDs are valid
  for (int i = 0; i < 50; ++i) {
    unsigned int id = indexPool.insert(i * 3);
    ASSERT_EQ(indexPool[id], i * 3);
  }
}

// Double Erase (Should Assert)
TEST(IndexPool, DoubleErase) {
  strobe::IndexPool<unsigned int, int, strobe::Mallocator> indexPool{};
  unsigned int a = indexPool.insert(42);
  indexPool.erase(a);

  // This should trigger an assertion in debug mode
#ifndef NDEBUG
  ASSERT_DEATH(indexPool.erase(a), ".*");
#endif
}

// Access Invalid Index (Should Assert)
TEST(IndexPool, AccessInvalidIndex) {
  strobe::IndexPool<unsigned int, int, strobe::Mallocator> indexPool{};

  // Accessing an index that was never created
#ifndef NDEBUG
  ASSERT_DEATH(indexPool[999], ".*");
#endif
}

// Complex Type with Destructor Side Effects
struct DestructionCounter {
  static int destroyedCount;
  DestructionCounter() = default;
  ~DestructionCounter() { destroyedCount++; }
};
int DestructionCounter::destroyedCount = 0;

TEST(IndexPool, ComplexTypeDestruction) {
  DestructionCounter::destroyedCount = 0;
  {
    strobe::IndexPool<unsigned int, DestructionCounter, strobe::Mallocator> indexPool{};
    unsigned int a = indexPool.insert();
    indexPool.insert();
    indexPool.erase(a);
    ASSERT_EQ(DestructionCounter::destroyedCount, 1);
  } // Destructor should trigger destruction of b

  ASSERT_EQ(DestructionCounter::destroyedCount, 2);
}

// Stress Test with Many Entries
TEST(IndexPool, LargeNumberOfEntries) {
  strobe::IndexPool<unsigned int, int, strobe::Mallocator> indexPool{};
  const int large_count = 1000000;

  for (int i = 0; i < large_count; ++i) {
    unsigned int id = indexPool.insert(i);
    ASSERT_EQ(indexPool[id], i);
  }
}

// Rapid Clear and Reuse Test
TEST(IndexPool, RapidClearReuse) {
  strobe::IndexPool<unsigned int, int, strobe::Mallocator> indexPool{};
  
  for (int cycle = 0; cycle < 1000; ++cycle) {
    std::unordered_set<unsigned int> ids;
    for (int i = 0; i < 100; ++i) {
      unsigned int id = indexPool.insert(i);
      ASSERT_EQ(indexPool[id], i);
      ids.insert(id);
    }
    for (unsigned int id : ids) {
      indexPool.erase(id);
    }
  }
}

} // namespace strobe::memory::testing::index_pool
