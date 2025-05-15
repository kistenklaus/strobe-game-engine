#include <gtest/gtest.h>

#include <strobe/core/sync/lockfree/mpsc_ring_buffer.hpp>


#include <gtest/gtest.h>
#include <atomic>
#include <thread>
#include <unordered_set>
#include <vector>

using namespace strobe::sync;


// Basic Initialization Test (Refactored)
TEST(LockFreeMPSCRingBuffer, BasicInitialization) {
  InplaceLockFreeMPSCRingBuffer<int, 8> buffer;
  ASSERT_EQ(buffer.capacity(), 8);

  // Attempt to dequeue from an empty buffer
  ASSERT_FALSE(buffer.dequeue().has_value());
}

// Enqueue and Dequeue Test (No Size, Full, or Empty)
TEST(LockFreeMPSCRingBuffer, EnqueueDequeueBasic) {
  InplaceLockFreeMPSCRingBuffer<int, 4> buffer;

  // Enqueue items up to capacity
  ASSERT_TRUE(buffer.enqueue(1));
  ASSERT_TRUE(buffer.enqueue(2));
  ASSERT_TRUE(buffer.enqueue(3));
  ASSERT_TRUE(buffer.enqueue(4));

  // Attempt to enqueue beyond capacity (should fail)
  ASSERT_FALSE(buffer.enqueue(5));

  // Dequeue and verify FIFO order
  ASSERT_EQ(buffer.dequeue().value(), 1);
  ASSERT_EQ(buffer.dequeue().value(), 2);
  ASSERT_EQ(buffer.dequeue().value(), 3);
  ASSERT_EQ(buffer.dequeue().value(), 4);

  // Attempt to dequeue from empty buffer
  ASSERT_FALSE(buffer.dequeue().has_value());

  // Reuse the buffer
  ASSERT_TRUE(buffer.enqueue(10));
  ASSERT_TRUE(buffer.enqueue(20));
  ASSERT_TRUE(buffer.enqueue(30));
  ASSERT_TRUE(buffer.enqueue(40));

  // Validate the FIFO order
  ASSERT_EQ(buffer.dequeue().value(), 10);
  ASSERT_EQ(buffer.dequeue().value(), 20);
  ASSERT_EQ(buffer.dequeue().value(), 30);
  ASSERT_EQ(buffer.dequeue().value(), 40);

  // Buffer should now be empty again
  ASSERT_FALSE(buffer.dequeue().has_value());
}


// Overwrite Test (Single-Threaded)
TEST(LockFreeMPSCRingBuffer, OverwriteTest) {
  InplaceLockFreeMPSCRingBuffer<int, 4> buffer;

  // Fill the buffer
  ASSERT_TRUE(buffer.enqueue(1));
  ASSERT_TRUE(buffer.enqueue(2));
  ASSERT_TRUE(buffer.enqueue(3));
  ASSERT_TRUE(buffer.enqueue(4));

  // Dequeue one and add another (should succeed)
  ASSERT_EQ(buffer.dequeue().value(), 1);
  ASSERT_TRUE(buffer.enqueue(5));

  // The buffer should now contain [2, 3, 4, 5]
  ASSERT_EQ(buffer.dequeue().value(), 2);
  ASSERT_EQ(buffer.dequeue().value(), 3);
  ASSERT_EQ(buffer.dequeue().value(), 4);
  ASSERT_EQ(buffer.dequeue().value(), 5);
  ASSERT_FALSE(buffer.dequeue().has_value());
}

// Clear Test (Single-Threaded)
TEST(LockFreeMPSCRingBuffer, ClearTest) {
  InplaceLockFreeMPSCRingBuffer<int, 4> buffer;

  // Enqueue several items
  ASSERT_TRUE(buffer.enqueue(1));
  ASSERT_TRUE(buffer.enqueue(2));
  ASSERT_TRUE(buffer.enqueue(3));

  // Clear the buffer
  buffer.clear();

  // The buffer should now be empty
  ASSERT_FALSE(buffer.dequeue().has_value());

  // Reuse the buffer
  ASSERT_TRUE(buffer.enqueue(10));
  ASSERT_TRUE(buffer.enqueue(20));
  ASSERT_TRUE(buffer.enqueue(30));

  // Validate FIFO after clear
  ASSERT_EQ(buffer.dequeue().value(), 10);
  ASSERT_EQ(buffer.dequeue().value(), 20);
  ASSERT_EQ(buffer.dequeue().value(), 30);
  ASSERT_FALSE(buffer.dequeue().has_value());
}

// Wrap-Around Test (Single-Threaded)
TEST(LockFreeMPSCRingBuffer, WrapAroundSingleThread) {
  InplaceLockFreeMPSCRingBuffer<int, 4> buffer;

  // Enqueue items to fill capacity
  ASSERT_TRUE(buffer.enqueue(1));
  ASSERT_TRUE(buffer.enqueue(2));
  ASSERT_TRUE(buffer.enqueue(3));
  ASSERT_TRUE(buffer.enqueue(4));

  // Dequeue two items
  ASSERT_EQ(buffer.dequeue().value(), 1);
  ASSERT_EQ(buffer.dequeue().value(), 2);

  // Enqueue two more items (should wrap around)
  ASSERT_TRUE(buffer.enqueue(5));
  ASSERT_TRUE(buffer.enqueue(6));

  // Verify the buffer contents
  ASSERT_EQ(buffer.dequeue().value(), 3);
  ASSERT_EQ(buffer.dequeue().value(), 4);
  ASSERT_EQ(buffer.dequeue().value(), 5);
  ASSERT_EQ(buffer.dequeue().value(), 6);
  ASSERT_FALSE(buffer.dequeue().has_value());
}

// Alternate Enqueue and Dequeue (Single-Threaded)
TEST(LockFreeMPSCRingBuffer, AlternatingEnqueueDequeue) {
  InplaceLockFreeMPSCRingBuffer<int, 4> buffer;

  // Alternate between enqueue and dequeue
  ASSERT_TRUE(buffer.enqueue(1));
  ASSERT_EQ(buffer.dequeue().value(), 1);
  ASSERT_TRUE(buffer.enqueue(2));
  ASSERT_TRUE(buffer.enqueue(3));
  ASSERT_EQ(buffer.dequeue().value(), 2);
  ASSERT_TRUE(buffer.enqueue(4));
  ASSERT_EQ(buffer.dequeue().value(), 3);
  ASSERT_EQ(buffer.dequeue().value(), 4);
  ASSERT_FALSE(buffer.dequeue().has_value());
}

// Mixed Enqueue and Dequeue Test (Single-Threaded)
TEST(LockFreeMPSCRingBuffer, MixedEnqueueDequeue) {
  InplaceLockFreeMPSCRingBuffer<int, 4> buffer;

  // Enqueue and immediately dequeue
  ASSERT_TRUE(buffer.enqueue(1));
  ASSERT_EQ(buffer.dequeue().value(), 1);

  // Enqueue two and dequeue one
  ASSERT_TRUE(buffer.enqueue(2));
  ASSERT_TRUE(buffer.enqueue(3));
  ASSERT_EQ(buffer.dequeue().value(), 2);

  // Enqueue another to force wrap-around
  ASSERT_TRUE(buffer.enqueue(4));
  ASSERT_TRUE(buffer.enqueue(5));

  // Verify the remaining items
  ASSERT_EQ(buffer.dequeue().value(), 3);
  ASSERT_EQ(buffer.dequeue().value(), 4);
  ASSERT_EQ(buffer.dequeue().value(), 5);
  ASSERT_FALSE(buffer.dequeue().has_value());
}

// Bulk Enqueue and Dequeue Test (Single-Threaded)
TEST(LockFreeMPSCRingBuffer, BulkEnqueueDequeue) {
  InplaceLockFreeMPSCRingBuffer<int, 8> buffer;

  // Enqueue multiple items
  for (int i = 0; i < 8; ++i) {
    ASSERT_TRUE(buffer.enqueue(i));
  }

  // Buffer should now be full
  ASSERT_FALSE(buffer.enqueue(100)); // Should fail

  // Dequeue all items and verify order
  for (int i = 0; i < 8; ++i) {
    ASSERT_EQ(buffer.dequeue().value(), i);
  }

  // Buffer should now be empty
  ASSERT_FALSE(buffer.dequeue().has_value());

  // Refill with different values
  for (int i = 10; i < 18; ++i) {
    ASSERT_TRUE(buffer.enqueue(i));
  }

  // Verify the new values
  for (int i = 10; i < 18; ++i) {
    ASSERT_EQ(buffer.dequeue().value(), i);
  }

  // Finally, the buffer should be empty
  ASSERT_FALSE(buffer.dequeue().has_value());
}

// Namespace for SPSC Test
namespace mpsc_test {
struct Tracked {
  static std::atomic<int> instance_count;
  int value;

  Tracked(int v) : value(v) { ++instance_count; }
  Tracked(const Tracked& other) : value(other.value) { ++instance_count; }
  Tracked(Tracked&& other) noexcept : value(other.value) { ++instance_count; }
  ~Tracked() { --instance_count; }
};

// Initialize static member
std::atomic<int> Tracked::instance_count{0};

}  // namespace spsc_test

// Memory Safety Test (Destruction)
TEST(LockFreeMPSCRingBuffer, MemorySafety) {
  using namespace mpsc_test;
  {
    InplaceLockFreeMPSCRingBuffer<Tracked, 4> buffer;
    buffer.enqueue(Tracked(1));
    buffer.enqueue(Tracked(2));
    buffer.enqueue(Tracked(3));
    buffer.enqueue(Tracked(4));
    ASSERT_EQ(Tracked::instance_count.load(), 4);

    // Dequeue and clear
    buffer.clear();
    ASSERT_EQ(Tracked::instance_count.load(), 0);
  }

  ASSERT_EQ(Tracked::instance_count.load(), 0); // All instances should be destroyed
}


// High-Frequency Enqueue/Dequeue Test (Single-Threaded)
TEST(LockFreeMPSCRingBuffer, HighFrequencyEnqueueDequeue) {
  InplaceLockFreeMPSCRingBuffer<int, 1024> buffer;

  // Rapid enqueue and dequeue
  for (int i = 0; i < 1000000; ++i) {
    ASSERT_TRUE(buffer.enqueue(i));
    ASSERT_EQ(buffer.dequeue().value(), i);
  }

  // At the end, the buffer should be empty
  ASSERT_FALSE(buffer.dequeue().has_value());
}

