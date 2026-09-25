#include <gtest/gtest.h>

#include <strobe/core/memory/allocator_ref.hpp>
#include <strobe/core/memory/mallocator.hpp>

namespace {

struct StatefulAllocator {
  int allocations = 0;
  int sized_deallocations = 0;
  int size_independent_deallocations = 0;

  void *allocate(std::size_t size, std::size_t align) {
    ++allocations;
    return strobe::Mallocator{}.allocate(size, align);
  }

  void deallocate(void *ptr, std::size_t size, std::size_t align) {
    ++sized_deallocations;
    strobe::Mallocator{}.deallocate(ptr, size, align);
  }

  void deallocate(void *ptr) {
    ++size_independent_deallocations;
    strobe::Mallocator{}.deallocate(ptr);
  }

  bool owns(void *ptr) const { return ptr != nullptr; }
};

static_assert(strobe::Allocator<StatefulAllocator>);
static_assert(strobe::SizeIndependentAllocator<StatefulAllocator>);
static_assert(strobe::OwningAllocator<StatefulAllocator>);
static_assert(!strobe::StatelessAllocator<StatefulAllocator>);
static_assert(!std::is_default_constructible_v<
              strobe::AllocatorReference<StatefulAllocator>>);

TEST(AllocatorReference, ForwardsStatefulOperations) {
  StatefulAllocator resource;
  strobe::AllocatorReference<StatefulAllocator> reference(&resource);

  void *memory = reference.allocate(sizeof(int), alignof(int));
  ASSERT_NE(memory, nullptr);
  EXPECT_EQ(resource.allocations, 1);
  EXPECT_TRUE(reference.owns(memory));

  reference.deallocate(memory);
  EXPECT_EQ(resource.size_independent_deallocations, 1);
  EXPECT_EQ(resource.sized_deallocations, 0);
}

TEST(AllocatorReference, ForwardsSizedDeallocation) {
  StatefulAllocator resource;
  strobe::AllocatorReference<StatefulAllocator> reference(&resource);

  void *memory = reference.allocate(sizeof(long), alignof(long));
  ASSERT_NE(memory, nullptr);

  reference.deallocate(memory, sizeof(long), alignof(long));
  EXPECT_EQ(resource.sized_deallocations, 1);
  EXPECT_EQ(resource.size_independent_deallocations, 0);
}

TEST(AllocatorReference, StatelessResourceNeedsNoPointer) {
  strobe::AllocatorReference<strobe::Mallocator> reference;

  void *memory = reference.allocate(sizeof(double), alignof(double));
  ASSERT_NE(memory, nullptr);

  reference.deallocate(memory, sizeof(double), alignof(double));
}

} // namespace
