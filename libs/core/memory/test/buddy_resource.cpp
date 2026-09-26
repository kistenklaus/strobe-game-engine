#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <new>
#include <unordered_set>
#include <vector>

#include <strobe/core/memory/allocator_traits.hpp>
#include <strobe/core/memory/buddy_resource.hpp>

namespace {

struct TrackingAllocator {
  std::shared_ptr<std::size_t> allocations = std::make_shared<std::size_t>();
  std::shared_ptr<std::size_t> deallocations = std::make_shared<std::size_t>();

  void *allocate(std::size_t size, std::size_t alignment) {
    ++*allocations;
    return ::operator new(size, std::align_val_t(alignment));
  }

  void deallocate(void *pointer, std::size_t, std::size_t alignment) noexcept {
    ++*deallocations;
    ::operator delete(pointer, std::align_val_t(alignment));
  }
};

struct MinimallyAlignedAllocator {
  void *allocate(std::size_t size, std::size_t alignment) {
    EXPECT_EQ(alignment, alignof(std::max_align_t));
    auto *raw = static_cast<std::byte *>(
        ::operator new(size + 64, std::align_val_t(64)));
    return raw + 16;
  }

  void deallocate(void *pointer, std::size_t, std::size_t alignment) noexcept {
    EXPECT_EQ(alignment, alignof(std::max_align_t));
    auto *raw = static_cast<std::byte *>(pointer) - 16;
    ::operator delete(raw, std::align_val_t(64));
  }
};

static_assert(strobe::Allocator<TrackingAllocator>);

using Resource = strobe::BuddyResource<1024, 16, TrackingAllocator>;
static_assert(strobe::Allocator<Resource>);
static_assert(strobe::OverAllocator<Resource>);

TEST(BuddyResource, ObtainsOneBackingAllocation) {
  TrackingAllocator upstream;
  {
    Resource resource(upstream);
    EXPECT_EQ(*upstream.allocations, 1U);
    EXPECT_EQ(*upstream.deallocations, 0U);
  }
  EXPECT_EQ(*upstream.deallocations, 1U);
}

TEST(BuddyResource, AllocatesWithinCapacityAndReportsOwnership) {
  Resource resource;

  void *first = resource.allocate(16, 16);
  void *second = resource.allocate(16, 16);

  ASSERT_NE(first, nullptr);
  ASSERT_NE(second, nullptr);
  EXPECT_NE(first, second);
  EXPECT_TRUE(resource.owns(first));
  EXPECT_TRUE(resource.owns(second));
  EXPECT_FALSE(resource.owns(nullptr));
  EXPECT_FALSE(resource.owns(static_cast<std::byte *>(first) + 1024));

  resource.deallocate(first, 16, 16);
  resource.deallocate(second, 16, 16);
}

TEST(BuddyResource, AllocationsHonorRequestedAlignment) {
  strobe::BuddyResource<1024, 16, MinimallyAlignedAllocator> resource;

  auto [first, first_size] = resource.allocate_at_least(64, 64);
  void *second = resource.allocate(128, 128);

  ASSERT_NE(first, nullptr);
  ASSERT_NE(second, nullptr);
  EXPECT_EQ(first_size, 64U);
  EXPECT_EQ(reinterpret_cast<std::uintptr_t>(first) % 64U, 0U);
  EXPECT_EQ(reinterpret_cast<std::uintptr_t>(second) % 128U, 0U);

  resource.deallocate(first, 64, 64);
  resource.deallocate(second, 128, 128);

  void *full = resource.allocate(1024, 16);
  ASSERT_NE(full, nullptr);
  resource.deallocate(full, 1024, 16);
}

TEST(BuddyResource, ExhaustsAndReusesAllMinimumBlocks) {
  Resource resource;
  std::vector<void *> allocations;

  for (;;) {
    void *memory = resource.allocate(16, 16);
    if (memory == nullptr) {
      break;
    }
    allocations.push_back(memory);
  }

  ASSERT_EQ(allocations.size(), 1024U / 16U);
  EXPECT_EQ(resource.allocate(16, 16), nullptr);

  std::unordered_set<void *> unique(allocations.begin(), allocations.end());
  EXPECT_EQ(unique.size(), allocations.size());

  for (void *memory : allocations) {
    resource.deallocate(memory, 16, 16);
  }

  void *full = resource.allocate(1024, 16);
  ASSERT_NE(full, nullptr);
  resource.deallocate(full, 1024, 16);
  void *reused = resource.allocate(1024, 16);
  ASSERT_NE(reused, nullptr);
  resource.deallocate(reused, 1024, 16);
}

TEST(BuddyResource, RoundsRequestsToPowerOfTwoBlockSizes) {
  Resource resource;

  void *small = resource.allocate(17, 16);
  void *large = resource.allocate(65, 16);

  ASSERT_NE(small, nullptr);
  ASSERT_NE(large, nullptr);
  EXPECT_NE(small, large);

  resource.deallocate(small, 17, 16);
  resource.deallocate(large, 65, 16);
}

TEST(BuddyResource, AllocateAtLeastReportsRoundedBlockSize) {
  Resource resource;

  auto [memory, actualSize] = resource.allocate_at_least(17, 16);

  ASSERT_NE(memory, nullptr);
  EXPECT_EQ(actualSize, 32U);
  resource.deallocate(memory, 17, 16);
}

TEST(BuddyResource, RoundsRequestsBelowTheMinimumBlockSize) {
  Resource resource;
  std::array<void *, 1024 / 16> allocations{};

  for (void *&allocation : allocations) {
    auto [pointer, available] = resource.allocate_at_least(1, 1);
    ASSERT_NE(pointer, nullptr);
    EXPECT_EQ(available, 16U);
    allocation = pointer;
  }

  EXPECT_EQ(resource.allocate(1, 1), nullptr);

  for (void *allocation : allocations) {
    resource.deallocate(allocation, 1, 1);
  }

  void *full = resource.allocate(1024, 1);
  ASSERT_NE(full, nullptr);
  resource.deallocate(full, 1024, 1);
}

TEST(BuddyResource, SupportsPageAllocatorWithLargerMinimumBlocks) {
  strobe::BuddyResource<64 * 1024, 16 * 1024> resource;

  auto [pointer, available] = resource.allocate_at_least(1, 1);

  ASSERT_NE(pointer, nullptr);
  EXPECT_EQ(available, 16U * 1024U);
  resource.deallocate(pointer, 1, 1);

  void *full = resource.allocate(64 * 1024, 16);
  ASSERT_NE(full, nullptr);
  resource.deallocate(full, 64 * 1024, 16);
}

TEST(BuddyResource, CoalescesAfterShuffledDeallocation) {
  Resource resource;
  std::array<void *, 1024 / 16> allocations{};

  for (void *&allocation : allocations) {
    allocation = resource.allocate(16, 16);
    ASSERT_NE(allocation, nullptr);
  }

  for (std::size_t index = 0; index < allocations.size(); ++index) {
    const std::size_t shuffled = (index * 37U) % allocations.size();
    resource.deallocate(allocations[shuffled], 16, 16);
  }

  void *full = resource.allocate(1024, 16);
  ASSERT_NE(full, nullptr);
  resource.deallocate(full, 1024, 16);
}

TEST(BuddyResource, RandomizedOperationsPreserveDisjointStorage) {
  struct Allocation {
    void *pointer;
    std::size_t requested;
    std::size_t available;
  };

  Resource resource;
  void *whole = resource.allocate(1024, 16);
  ASSERT_NE(whole, nullptr);
  const auto begin = reinterpret_cast<std::uintptr_t>(whole);
  resource.deallocate(whole, 1024, 16);

  std::vector<Allocation> live;
  std::uint64_t random = 0x9e3779b97f4a7c15ULL;
  const auto next_random = [&random] {
    random ^= random << 13;
    random ^= random >> 7;
    random ^= random << 17;
    return random;
  };

  for (std::size_t iteration = 0; iteration < 20'000; ++iteration) {
    const std::uint64_t value = next_random();
    if (live.empty() || (value & 1U) == 0) {
      const std::size_t requested = 1 + ((value >> 1U) % 256U);
      auto [pointer, available] = resource.allocate_at_least(requested, 1);
      if (pointer == nullptr) {
        continue;
      }

      EXPECT_EQ(available, std::max<std::size_t>(16, std::bit_ceil(requested)));
      const auto allocation_begin = reinterpret_cast<std::uintptr_t>(pointer);
      const auto allocation_end = allocation_begin + available;
      EXPECT_GE(allocation_begin, begin);
      EXPECT_LE(allocation_end, begin + 1024);

      for (const Allocation &allocation : live) {
        const auto other_begin =
            reinterpret_cast<std::uintptr_t>(allocation.pointer);
        const auto other_end = other_begin + allocation.available;
        EXPECT_TRUE(allocation_end <= other_begin ||
                    other_end <= allocation_begin);
      }

      live.push_back({pointer, requested, available});
    } else {
      const std::size_t index = (value >> 1U) % live.size();
      resource.deallocate(live[index].pointer, live[index].requested, 1);
      live[index] = live.back();
      live.pop_back();
    }
  }

  for (const Allocation &allocation : live) {
    resource.deallocate(allocation.pointer, allocation.requested, 1);
  }

  whole = resource.allocate(1024, 16);
  ASSERT_NE(whole, nullptr);
  resource.deallocate(whole, 1024, 16);
}

TEST(BuddyResource, AllocateAtLeastReportsFailureWhenCapacityIsExhausted) {
  Resource resource;
  void *full = resource.allocate(1024, 16);
  ASSERT_NE(full, nullptr);

  auto [memory, actualSize] = resource.allocate_at_least(16, 16);

  EXPECT_EQ(memory, nullptr);
  EXPECT_EQ(actualSize, 0U);
  resource.deallocate(full, 1024, 16);
}

TEST(BuddyResource, OversizedAllocationReturnsNull) {
  Resource resource;

  EXPECT_EQ(resource.allocate(1025, 16), nullptr);
}

} // namespace
