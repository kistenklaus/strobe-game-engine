#include <gtest/gtest.h>

#include <cstdint>
#include <strobe/core/memory/allocator_ref.hpp>
#include <strobe/core/memory/allocator_traits.hpp>
#include <strobe/core/memory/mallocator.hpp>

namespace {

using strobe::AllocatorTraits;
using strobe::Mallocator;

static_assert(strobe::Allocator<Mallocator>);
static_assert(strobe::StatelessAllocator<Mallocator>);
static_assert(strobe::SizeIndependentAllocator<Mallocator>);
static_assert(AllocatorTraits<Mallocator>::is_stateless);
static_assert(AllocatorTraits<Mallocator>::is_always_equal);

TEST(Mallocator, AllocatesAndReleasesAlignedStorage) {
  Mallocator allocator;

  constexpr std::size_t size = 192;
  constexpr std::size_t alignment = 64;
  void *memory = allocator.allocate(size, alignment);

  ASSERT_NE(memory, nullptr);
  EXPECT_EQ(reinterpret_cast<std::uintptr_t>(memory) % alignment, 0U);

  allocator.deallocate(memory, size, alignment);
}

TEST(Mallocator, TraitsForwardTypedAllocation) {
  Mallocator allocator;

  int *value = AllocatorTraits<Mallocator>::template allocate<int>(allocator);
  ASSERT_NE(value, nullptr);
  *value = 42;
  EXPECT_EQ(*value, 42);

  AllocatorTraits<Mallocator>::deallocate(allocator, value);
}

TEST(Mallocator, StatelessReferenceCanBeDefaultConstructed) {
  strobe::AllocatorReference<Mallocator> allocator;

  void *memory = allocator.allocate(sizeof(std::uint64_t), alignof(std::uint64_t));
  ASSERT_NE(memory, nullptr);

  allocator.deallocate(memory, sizeof(std::uint64_t), alignof(std::uint64_t));
}

} // namespace
