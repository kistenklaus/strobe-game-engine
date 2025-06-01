#include <gtest/gtest.h>
#include <random>
#include <strobe/core/memory/monotonic_pool_allocator.hpp>
#include <strobe/core/memory/Mallocator.hpp>
#include <unordered_set>

using namespace strobe;

// Simple structure to test object construction
struct TestObject {
    int value;
    TestObject(int v) : value(v) {}
};

// Basic allocation and deallocation
TEST(MonotonicPoolResource, BasicAllocation) {
    MonotonicPoolResource<sizeof(int), alignof(int), Mallocator> resource;

    void* ptr1 = resource.allocate(sizeof(int), alignof(int));
    void* ptr2 = resource.allocate(sizeof(int), alignof(int));
    
    ASSERT_NE(ptr1, nullptr);
    ASSERT_NE(ptr2, nullptr);
    ASSERT_NE(ptr1, ptr2);

    resource.deallocate(ptr1, sizeof(int), alignof(int));
    resource.deallocate(ptr2, sizeof(int), alignof(int));
}

// Reuse of freed memory (free list)
TEST(MonotonicPoolResource, ReuseFreedMemory) {
    MonotonicPoolResource<sizeof(int), alignof(int), Mallocator> resource;

    void* ptr1 = resource.allocate(sizeof(int), alignof(int));
    resource.deallocate(ptr1, sizeof(int), alignof(int));
    
    void* ptr2 = resource.allocate(sizeof(int), alignof(int));
    ASSERT_EQ(ptr1, ptr2); // Memory should be reused
}

// Growth Test (checking pool growth logic)
TEST(MonotonicPoolResource, Growth) {
    MonotonicPoolResource<sizeof(int), alignof(int), Mallocator> resource;

    // Allocate a few blocks to trigger growth
    for (int i = 0; i < 100; ++i) {
        void* ptr = resource.allocate(sizeof(int), alignof(int));
        ASSERT_NE(ptr, nullptr);
    }
}

// Alignment Test (ensuring correct alignment)
TEST(MonotonicPoolResource, Alignment) {
    constexpr std::size_t alignment = alignof(std::max_align_t);
    MonotonicPoolResource<sizeof(int), alignment, Mallocator> resource;

    void* ptr = resource.allocate(sizeof(int), alignment);
    ASSERT_EQ(reinterpret_cast<std::uintptr_t>(ptr) % alignment, 0);
}

// Object Construction Test (constructing objects in place)
TEST(MonotonicPoolResource, ObjectConstruction) {
    MonotonicPoolResource<sizeof(TestObject), alignof(TestObject), Mallocator> resource;

    TestObject* obj = static_cast<TestObject*>(resource.allocate(sizeof(TestObject), alignof(TestObject)));
    new (obj) TestObject(42);
    ASSERT_EQ(obj->value, 42);

    obj->~TestObject();
    resource.deallocate(obj, sizeof(TestObject), alignof(TestObject));
}

// Large Allocation Test (testing growth factor)
TEST(MonotonicPoolResource, LargeAllocation) {
    MonotonicPoolResource<1024, 64, Mallocator> resource;

    void* ptr1 = resource.allocate(1024, 64);
    void* ptr2 = resource.allocate(1024, 64);
    void* ptr3 = resource.allocate(1024, 64);

    ASSERT_NE(ptr1, nullptr);
    ASSERT_NE(ptr2, nullptr);
    ASSERT_NE(ptr3, nullptr);
    ASSERT_NE(ptr1, ptr2);
    ASSERT_NE(ptr2, ptr3);
}

// Clear Test (checking memory reset)
TEST(MonotonicPoolResource, Clear) {
    MonotonicPoolResource<sizeof(int), alignof(int), Mallocator> resource;

    void* ptr1 = resource.allocate(sizeof(int), alignof(int));
    void* ptr2 = resource.allocate(sizeof(int), alignof(int));
    
    resource.deallocate(ptr1, sizeof(int), alignof(int));
    resource.deallocate(ptr2, sizeof(int), alignof(int));

    void* ptr3 = resource.allocate(sizeof(int), alignof(int));
    void* ptr4 = resource.allocate(sizeof(int), alignof(int));

    ASSERT_EQ(ptr3, ptr2);
    ASSERT_EQ(ptr4, ptr1);
}

// Core allocator integrity test
TEST(MonotonicPoolResource, AllocatorIntegrity) {
    constexpr std::size_t allocation_count = 1000;
    MonotonicPoolResource<sizeof(int), alignof(int), Mallocator> resource;

    // Generate random values
    std::vector<int> random_values(allocation_count);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(0, 1000000);

    for (auto& value : random_values) {
        value = dist(gen);
    }

    std::unordered_map<void*, int> memory_map;

    // Allocate and write random values
    for (std::size_t i = 0; i < allocation_count; ++i) {
        int* ptr = static_cast<int*>(resource.allocate(sizeof(int), alignof(int)));
        *ptr = random_values[i];
        ASSERT_FALSE(memory_map.contains(ptr)) << "Returned the same address twice";
        memory_map[ptr] = random_values[i];
    }

    // Collect all pointers and shuffle them
    std::vector<void*> pointers;
    for (auto& [ptr, value] : memory_map) {
        pointers.push_back(ptr);
    }

    // Randomly shuffle the pointers
    std::shuffle(pointers.begin(), pointers.end(), gen);

    // Deallocate in random order
    for (void* ptr : pointers) {
        auto expected = memory_map[ptr];
        auto observed = *static_cast<int*>(ptr);
        ASSERT_EQ(observed, expected);

        resource.deallocate(ptr, sizeof(int), alignof(int));
    }
}
