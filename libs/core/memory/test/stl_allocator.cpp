#include <gtest/gtest.h>

#include <cstddef>
#include <memory>
#include <new>
#include <vector>

#include <strobe/core/memory/allocator_traits.hpp>
#include <strobe/core/memory/stl_allocator.hpp>

namespace {

struct AllocationStats {
  std::size_t allocations = 0;
  std::size_t deallocations = 0;
  std::size_t last_size = 0;
  std::size_t last_alignment = 0;
};

struct RecordingAllocator {
  AllocationStats *stats = nullptr;

  void *allocate(std::size_t size, std::size_t alignment) {
    ++stats->allocations;
    stats->last_size = size;
    stats->last_alignment = alignment;
    return ::operator new(size, std::align_val_t(alignment));
  }

  void deallocate(void *pointer, std::size_t size,
                 std::size_t alignment) noexcept {
    ++stats->deallocations;
    stats->last_size = size;
    stats->last_alignment = alignment;
    ::operator delete(pointer, std::align_val_t(alignment));
  }

  friend bool operator==(const RecordingAllocator &lhs,
                         const RecordingAllocator &rhs) noexcept {
    return lhs.stats == rhs.stats;
  }
};

static_assert(strobe::Allocator<RecordingAllocator>);
using IntAllocator = strobe::StlAllocator<int, RecordingAllocator>;
static_assert(std::same_as<std::allocator_traits<IntAllocator>::value_type,
                           int>);
static_assert(std::same_as<std::allocator_traits<IntAllocator>::rebind_alloc<
                               double>,
                           strobe::StlAllocator<double, RecordingAllocator>>);

TEST(StlAllocator, ConvertsObjectCountToBytesAndPreservesAlignment) {
  AllocationStats stats;
  IntAllocator allocator(RecordingAllocator{&stats});

  int *values = allocator.allocate(3);

  ASSERT_NE(values, nullptr);
  EXPECT_EQ(stats.allocations, 1U);
  EXPECT_EQ(stats.last_size, 3U * sizeof(int));
  EXPECT_EQ(stats.last_alignment, alignof(int));

  allocator.deallocate(values, 3);
  EXPECT_EQ(stats.deallocations, 1U);
  EXPECT_EQ(stats.last_size, 3U * sizeof(int));
  EXPECT_EQ(stats.last_alignment, alignof(int));
}

TEST(StlAllocator, RebindsAndComparesAcrossObjectTypes) {
  AllocationStats stats;
  IntAllocator integers(RecordingAllocator{&stats});
  strobe::StlAllocator<double, RecordingAllocator> doubles(integers);

  EXPECT_TRUE(integers == doubles);
}

TEST(StlAllocator, WorksWithStandardVector) {
  AllocationStats stats;
  {
    std::vector<int, IntAllocator> values{
        IntAllocator{RecordingAllocator{&stats}}};

    values.push_back(7);
    values.push_back(11);

    ASSERT_EQ(values.size(), 2U);
    EXPECT_EQ(values[0], 7);
    EXPECT_EQ(values[1], 11);
    EXPECT_GE(stats.allocations, 1U);
  }

  EXPECT_EQ(stats.deallocations, stats.allocations);
}

} // namespace
