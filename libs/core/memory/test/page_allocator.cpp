#include <gtest/gtest.h>

#include <cstdint>
#include <strobe/core/memory/allocator_traits.hpp>
#include <strobe/core/memory/page_allocator.hpp>
#include <strobe/core/memory/pages.hpp>

namespace {

using strobe::AllocatorTraits;
using strobe::PageAllocator;

static_assert(strobe::Allocator<PageAllocator>);
static_assert(strobe::OverAllocator<PageAllocator>);
static_assert(strobe::StatelessAllocator<PageAllocator>);
static_assert(AllocatorTraits<PageAllocator>::is_stateless);

TEST(PageAllocator, AllocatesPageAlignedStorage) {
  PageAllocator allocator;
  const std::size_t page = strobe::page_size();
  constexpr std::size_t size = 137;

  void *memory = allocator.allocate(size, page);

  ASSERT_NE(memory, nullptr);
  EXPECT_EQ(reinterpret_cast<std::uintptr_t>(memory) % page, 0U);

  allocator.deallocate(memory, size, page);
}

TEST(PageAllocator, ZeroSizeAllocationReturnsNull) {
  PageAllocator allocator;

  EXPECT_EQ(allocator.allocate(0, strobe::page_size()), nullptr);
}

TEST(PageAllocator, AllocateAtLeastReportsPageRoundedSize) {
  PageAllocator allocator;
  const std::size_t page = strobe::page_size();
  constexpr std::size_t size = 137;

  auto [memory, actualSize] = allocator.allocate_at_least(size, page);

  ASSERT_NE(memory, nullptr);
  EXPECT_GE(actualSize, size);
  EXPECT_EQ(actualSize % page, 0U);

  allocator.deallocate(memory, actualSize, page);
}

TEST(PageAllocator, AllocateAtLeastZeroSizeReturnsNullAndZero) {
  PageAllocator allocator;
  const std::size_t page = strobe::page_size();

  auto [memory, actualSize] = allocator.allocate_at_least(0, page);

  EXPECT_EQ(memory, nullptr);
  EXPECT_EQ(actualSize, 0U);
}

TEST(PageAllocator, AllocateAtLeastRoundsAcrossPageBoundary) {
  PageAllocator allocator;
  const std::size_t page = strobe::page_size();

  auto [memory, actualSize] = allocator.allocate_at_least(page + 1, page);

  ASSERT_NE(memory, nullptr);
  EXPECT_EQ(actualSize, page * 2);

  allocator.deallocate(memory, actualSize, page);
}

TEST(PageAllocator, NullDeallocationIsNoOp) {
  PageAllocator allocator;

  EXPECT_NO_FATAL_FAILURE(
      allocator.deallocate(nullptr, 0, strobe::page_size()));
}

TEST(PageAllocator, DefaultConstructedInstancesAreInterchangeable) {
  PageAllocator first;
  PageAllocator second;
  const std::size_t page = strobe::page_size();

  void *memory = first.allocate(sizeof(std::uint64_t), page);
  ASSERT_NE(memory, nullptr);

  second.deallocate(memory, sizeof(std::uint64_t), page);
}

} // namespace
