#include <gtest/gtest.h>

#include <cstddef>
#include <memory>
#include <new>
#include <unordered_set>
#include <vector>

#include <strobe/core/memory/allocator_traits.hpp>
#include <strobe/core/memory/BuddyResource.hpp>

namespace {

struct TrackingAllocator {
  std::shared_ptr<std::size_t> allocations = std::make_shared<std::size_t>();
  std::shared_ptr<std::size_t> deallocations =
      std::make_shared<std::size_t>();

  void *allocate(std::size_t size, std::size_t alignment) {
    ++*allocations;
    return ::operator new(size, std::align_val_t(alignment));
  }

  void deallocate(void *pointer, std::size_t,
                 std::size_t alignment) noexcept {
    ++*deallocations;
    ::operator delete(pointer, std::align_val_t(alignment));
  }
};

static_assert(strobe::Allocator<TrackingAllocator>);

using Resource = strobe::BuddyResource<1024, 16, TrackingAllocator>;

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

  EXPECT_NE(resource.allocate(1024, 16), nullptr);
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

TEST(BuddyResource, OversizedAllocationReturnsNull) {
  Resource resource;

  EXPECT_EQ(resource.allocate(1025, 16), nullptr);
}

} // namespace
