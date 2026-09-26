#include <gtest/gtest.h>

#include <cstddef>
#include <memory>
#include <new>

#include <strobe/core/memory/allocator_traits.hpp>
#include <strobe/core/memory/poly_allocator.hpp>

namespace {

struct AllocationStats {
  std::size_t allocations = 0;
  std::size_t deallocations = 0;
  std::size_t last_size = 0;
  std::size_t last_alignment = 0;
};

struct RecordingAllocator {
  std::shared_ptr<AllocationStats> stats = std::make_shared<AllocationStats>();

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
};

static_assert(strobe::Allocator<RecordingAllocator>);
using ConcreteResource = strobe::MemoryResource<RecordingAllocator>;
static_assert(strobe::Allocator<ConcreteResource>);
static_assert(strobe::Allocator<strobe::PolyMemoryResource>);
static_assert(strobe::Allocator<strobe::PolyResourceReference>);

TEST(PolyMemoryResource, ConcreteResourceForwardsAllocationOperations) {
  RecordingAllocator allocator;
  ConcreteResource resource(allocator);

  void *memory = resource.allocate(37, 32);

  ASSERT_NE(memory, nullptr);
  EXPECT_EQ(allocator.stats->allocations, 1U);
  EXPECT_EQ(allocator.stats->last_size, 37U);
  EXPECT_EQ(allocator.stats->last_alignment, 32U);

  resource.deallocate(memory, 37, 32);
  EXPECT_EQ(allocator.stats->deallocations, 1U);
}

TEST(PolyMemoryResource, DispatchesThroughTheBaseInterface) {
  RecordingAllocator allocator;
  ConcreteResource concrete(allocator);
  strobe::PolyMemoryResource &resource = concrete;

  void *memory = resource.allocate(24, 16);

  ASSERT_NE(memory, nullptr);
  EXPECT_EQ(allocator.stats->allocations, 1U);
  resource.deallocate(memory, 24, 16);
  EXPECT_EQ(allocator.stats->deallocations, 1U);
}

TEST(PolyMemoryResource, ReferenceForwardsToReferencedResource) {
  RecordingAllocator allocator;
  ConcreteResource concrete(allocator);
  strobe::PolyResourceReference reference(&concrete);

  void *memory = reference.allocate(48, 64);

  ASSERT_NE(memory, nullptr);
  EXPECT_EQ(allocator.stats->allocations, 1U);
  reference.deallocate(memory, 48, 64);
  EXPECT_EQ(allocator.stats->deallocations, 1U);
}

} // namespace
