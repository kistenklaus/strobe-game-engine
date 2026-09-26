#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <mutex>
#include <new>
#include <thread>
#include <unordered_set>
#include <vector>

#include <strobe/core/memory/allocator_traits.hpp>
#include <strobe/core/memory/sync_resource.hpp>

namespace {

struct RichAllocator {
  void *allocate(std::size_t size, std::size_t alignment) {
    void *pointer = ::operator new(size, std::align_val_t(alignment));
    std::lock_guard lock{mutex};
    pointers.insert(pointer);
    return pointer;
  }

  void deallocate(void *pointer, std::size_t, std::size_t alignment) {
    {
      std::lock_guard lock{mutex};
      pointers.erase(pointer);
    }
    ::operator delete(pointer, std::align_val_t(alignment));
  }

  void deallocate(void *pointer) { deallocate(pointer, 0, alignof(std::max_align_t)); }

  void *reallocate(void *pointer, std::size_t old_size,
                   std::size_t new_size, std::size_t alignment) {
    void *replacement = allocate(new_size, alignment);
    std::memcpy(replacement, pointer, std::min(old_size, new_size));
    deallocate(pointer, old_size, alignment);
    return replacement;
  }

  std::pair<void *, std::size_t> allocate_at_least(std::size_t size,
                                                   std::size_t alignment) {
    return {allocate(size, alignment), size + 8};
  }

  bool owns(const void *pointer) const {
    std::lock_guard lock{mutex};
    return pointers.contains(const_cast<void *>(pointer));
  }

  mutable std::mutex mutex;
  std::unordered_set<void *> pointers;
};

using Resource = strobe::SyncResource<RichAllocator>;
static_assert(strobe::Allocator<Resource>);
static_assert(strobe::OverAllocator<Resource>);
static_assert(strobe::ReAllocator<Resource>);
static_assert(strobe::OwningAllocator<Resource>);
static_assert(strobe::SizeIndependentAllocator<Resource>);

TEST(SyncResource, ForwardsAllocationAndOwnershipOperations) {
  Resource resource;
  void *pointer = resource.allocate(16, 16);

  ASSERT_NE(pointer, nullptr);
  EXPECT_TRUE(resource.owns(pointer));
  int outside = 0;
  EXPECT_FALSE(resource.owns(&outside));

  resource.deallocate(pointer, 16, 16);
  EXPECT_FALSE(resource.owns(pointer));
}

TEST(SyncResource, ForwardsOptionalAllocatorOperations) {
  Resource resource;
  auto [pointer, available] = resource.allocate_at_least(16, 16);
  ASSERT_NE(pointer, nullptr);
  EXPECT_EQ(available, 24U);

  void *replacement = resource.reallocate(pointer, 16, 32, 16);
  ASSERT_NE(replacement, nullptr);
  EXPECT_TRUE(resource.owns(replacement));
  resource.deallocate(replacement);
  EXPECT_FALSE(resource.owns(replacement));
}

TEST(SyncResource, LockedCallbackProvidesProtectedUpstreamAccess) {
  Resource resource;
  EXPECT_EQ(resource.locked([](RichAllocator &allocator) {
               return allocator.pointers.size();
             }),
            0U);

  const Resource &constant = resource;
  EXPECT_EQ(constant.locked([](const RichAllocator &allocator) {
               return allocator.pointers.size();
             }),
            0U);
}

TEST(SyncResource, SerializesConcurrentOperations) {
  Resource resource;
  constexpr std::size_t thread_count = 8;
  constexpr std::size_t operations_per_thread = 64;
  std::vector<std::thread> threads;
  threads.reserve(thread_count);

  for (std::size_t thread = 0; thread < thread_count; ++thread) {
    threads.emplace_back([&] {
      for (std::size_t i = 0; i < operations_per_thread; ++i) {
        void *pointer = resource.allocate(8, 8);
        ASSERT_NE(pointer, nullptr);
        resource.deallocate(pointer, 8, 8);
      }
    });
  }
  for (auto &thread : threads) {
    thread.join();
  }
}

} // namespace
