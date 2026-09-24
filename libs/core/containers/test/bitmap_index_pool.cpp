#include "strobe/core/containers/invalid_index.hpp"
#include <strobe/core/containers/bitmap_index_pool.hpp>

#include <gtest/gtest.h>
#include <vector>

TEST(BitmapIndexPool, single_allocations_are_unique_until_exhausted) {
  constexpr size_t size = 17;
  strobe::BitmapIndexPool pool(size);
  std::vector<bool> occupied(size, false);

  for (size_t n = 0; n < size; ++n) {
    const size_t index = pool.alloc();
    ASSERT_NE(index, strobe::INVALID_INDEX);
    ASSERT_LT(index, size);
    EXPECT_FALSE(occupied[index]);
    occupied[index] = true;
  }

  EXPECT_EQ(pool.alloc(), strobe::INVALID_INDEX);
}

TEST(BitmapIndexPool, ranges_are_contiguous_and_do_not_overlap) {
  constexpr size_t size = 19;
  strobe::BitmapIndexPool pool(size);
  std::vector<bool> occupied(size, false);

  for (uint32_t count : {3u, 5u, 2u, 9u}) {
    const size_t first = pool.alloc(count);
    ASSERT_NE(first, strobe::INVALID_INDEX);
    ASSERT_LE(first, size);
    ASSERT_LE(count, size - first);

    for (size_t i = first; i < first + count; ++i) {
      EXPECT_FALSE(occupied[i]);
      occupied[i] = true;
    }
  }

  EXPECT_EQ(pool.alloc(), strobe::INVALID_INDEX);
  EXPECT_EQ(pool.alloc(2), strobe::INVALID_INDEX);
}

TEST(BitmapIndexPool, freed_range_can_be_allocated_again) {
  constexpr size_t size = 12;
  strobe::BitmapIndexPool pool(size);

  const size_t first = pool.alloc(size);
  ASSERT_NE(first, strobe::INVALID_INDEX);
  ASSERT_LE(first, size);
  ASSERT_LE(size, size - first);

  const size_t released = first + 4;
  pool.free(released, 3);

  const size_t reused = pool.alloc(3);
  ASSERT_NE(reused, strobe::INVALID_INDEX);
  ASSERT_LE(reused, size);
  ASSERT_LE(3u, size - reused);

  // All other indices remain allocated, so this is the only valid range.
  EXPECT_EQ(reused, released);
  EXPECT_EQ(pool.alloc(), strobe::INVALID_INDEX);
}

TEST(BitmapIndexPool, growth_preserves_allocations_and_adds_capacity) {
  constexpr size_t oldSize = 7;
  constexpr size_t newSize = 15;
  strobe::BitmapIndexPool pool(oldSize);
  std::vector<bool> occupied(newSize, false);

  for (size_t n = 0; n < oldSize; ++n) {
    const size_t index = pool.alloc();
    ASSERT_NE(index, strobe::INVALID_INDEX);
    ASSERT_LT(index, oldSize);
    ASSERT_FALSE(occupied[index]);
    occupied[index] = true;
  }
  EXPECT_EQ(pool.alloc(), strobe::INVALID_INDEX);

  pool.resize(newSize);
  for (size_t n = oldSize; n < newSize; ++n) {
    const size_t index = pool.alloc();
    ASSERT_NE(index, strobe::INVALID_INDEX);
    ASSERT_LT(index, newSize);
    EXPECT_FALSE(occupied[index]);
    occupied[index] = true;
  }
  EXPECT_EQ(pool.alloc(), strobe::INVALID_INDEX);
}
