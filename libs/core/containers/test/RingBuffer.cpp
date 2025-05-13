#include <gtest/gtest.h>

#include <strobe/core/containers/ring_buffer.hpp>
#include "strobe/core/memory/Mallocator.hpp"

// Basic Initialization Test
TEST(RingBuffer, BasicInitialization) {
  strobe::InplaceRingBuffer<int, 8> buffer;
  ASSERT_TRUE(buffer.empty());
  ASSERT_FALSE(buffer.full());
  ASSERT_EQ(buffer.size(), 0);
  ASSERT_EQ(buffer.capacity(), 8);
}

// Push and Pop Operations
TEST(RingBuffer, PushBackPopFront) {
  strobe::InplaceRingBuffer<int, 8> buffer;
  buffer.push_back(1);
  buffer.push_back(2);
  buffer.push_back(3);

  ASSERT_EQ(buffer.front(), 1);
  ASSERT_EQ(buffer.back(), 3);
  ASSERT_EQ(buffer.size(), 3);

  buffer.pop_front();
  ASSERT_EQ(buffer.front(), 2);
  ASSERT_EQ(buffer.size(), 2);

  buffer.pop_front();
  buffer.pop_front();
  ASSERT_TRUE(buffer.empty());
}

TEST(RingBuffer, PushBackPopFrontDynamic) {
  strobe::RingBuffer<int, strobe::Mallocator> buffer{8};
  buffer.push_back(1);
  buffer.push_back(2);
  buffer.push_back(3);

  ASSERT_EQ(buffer.front(), 1);
  ASSERT_EQ(buffer.back(), 3);
  ASSERT_EQ(buffer.size(), 3);

  buffer.pop_front();
  ASSERT_EQ(buffer.front(), 2);
  ASSERT_EQ(buffer.size(), 2);

  buffer.pop_front();
  buffer.pop_front();
  ASSERT_TRUE(buffer.empty());
}

TEST(RingBuffer, PushFrontPopBack) {
  strobe::InplaceRingBuffer<int, 8> buffer;
  buffer.push_front(1);
  buffer.push_front(2);
  buffer.push_front(3);

  ASSERT_EQ(buffer.front(), 3);
  ASSERT_EQ(buffer.back(), 1);
  ASSERT_EQ(buffer.size(), 3);

  buffer.pop_back();
  ASSERT_EQ(buffer.back(), 2);
  ASSERT_EQ(buffer.size(), 2);

  buffer.pop_back();
  buffer.pop_back();
  ASSERT_TRUE(buffer.empty());
}

// Queue-Like Operations (Enqueue/Dequeue)
TEST(RingBuffer, EnqueueDequeue) {
  strobe::InplaceRingBuffer<int, 8> buffer;
  ASSERT_TRUE(buffer.enqueue(1));
  ASSERT_TRUE(buffer.enqueue(2));
  ASSERT_TRUE(buffer.enqueue(3));

  ASSERT_EQ(buffer.size(), 3);
  ASSERT_EQ(buffer.dequeue().value(), 1);
  ASSERT_EQ(buffer.dequeue().value(), 2);
  ASSERT_EQ(buffer.dequeue().value(), 3);
  ASSERT_TRUE(buffer.empty());
}

// Full and Empty Conditions
TEST(RingBuffer, FullAndEmptyConditions) {
  strobe::InplaceRingBuffer<int, 3> buffer;
  ASSERT_TRUE(buffer.empty());
  ASSERT_FALSE(buffer.full());

  buffer.push_back(1);
  buffer.push_back(2);
  buffer.push_back(3);

  ASSERT_TRUE(buffer.full());
  ASSERT_FALSE(buffer.empty());
  ASSERT_EQ(buffer.size(), 3);

  ASSERT_FALSE(buffer.enqueue(4)); // Should fail because full
  buffer.pop_front();
  ASSERT_TRUE(buffer.enqueue(4));
}

// Front and Back Access
TEST(RingBuffer, FrontAndBackAccess) {
  strobe::InplaceRingBuffer<int, 8> buffer;
  buffer.push_back(10);
  buffer.push_back(20);
  buffer.push_back(30);

  ASSERT_EQ(buffer.front(), 10);
  ASSERT_EQ(buffer.back(), 30);

  buffer.push_front(5);
  ASSERT_EQ(buffer.front(), 5);
  ASSERT_EQ(buffer.back(), 30);
}

// Clear Operation
TEST(RingBuffer, ClearOperation) {
  strobe::InplaceRingBuffer<int, 8> buffer;
  buffer.push_back(10);
  buffer.push_back(20);
  buffer.push_back(30);
  ASSERT_EQ(buffer.size(), 3);

  buffer.clear();
  ASSERT_TRUE(buffer.empty());
  ASSERT_EQ(buffer.size(), 0);

  // Should still work after clearing
  buffer.push_back(40);
  ASSERT_EQ(buffer.front(), 40);
}



// Namespace for RingBuffer Memory Safety Test
namespace ringbuffer_test {
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
}  // namespace ringbuffer_test

// Memory Safety Test for RingBuffer
TEST(RingBuffer, MemorySafety) {
  using namespace ringbuffer_test;
  
  {
    strobe::InplaceRingBuffer<Tracked, 4> buffer;

    // Enqueue some items
    buffer.push_back(Tracked(1));
    buffer.push_back(Tracked(2));
    buffer.push_back(Tracked(3));
    ASSERT_EQ(Tracked::instance_count.load(), 3);


    // Dequeue some items
    buffer.pop_front();
    ASSERT_EQ(Tracked::instance_count.load(), 2);


    // Enqueue again to trigger wrapping
    buffer.push_back(Tracked(4));
    buffer.push_back(Tracked(5));
    ASSERT_EQ(Tracked::instance_count.load(), 4);

    // Clear the buffer
    buffer.clear();
    ASSERT_EQ(Tracked::instance_count.load(), 0);

  }



  // After scope ends, all instances should be destroyed
  ASSERT_EQ(Tracked::instance_count.load(), 0);
}

// Memory Safety - Clear and Reuse Test for RingBuffer
TEST(RingBuffer, MemorySafetyClearReuse) {
  using namespace ringbuffer_test;

  {
    strobe::InplaceRingBuffer<Tracked, 4> buffer;

    // Enqueue some items
    buffer.push_back(Tracked(1));
    buffer.push_back(Tracked(2));
    buffer.push_back(Tracked(3));
    ASSERT_EQ(Tracked::instance_count.load(), 3);

    // Clear the buffer
    buffer.clear();
    ASSERT_EQ(Tracked::instance_count.load(), 0);

    // Reuse the buffer
    buffer.push_back(Tracked(4));
    buffer.push_back(Tracked(5));
    ASSERT_EQ(Tracked::instance_count.load(), 2);

    // Clear again
    buffer.clear();
    ASSERT_EQ(Tracked::instance_count.load(), 0);
  }

  // All instances should be destroyed after scope
  ASSERT_EQ(Tracked::instance_count.load(), 0);
}

// Memory Safety - Partial Destruction Test for RingBuffer
TEST(RingBuffer, MemorySafetyPartialDestruction) {
  using namespace ringbuffer_test;

  {
    strobe::InplaceRingBuffer<Tracked, 4> buffer;

    // Enqueue some items
    buffer.push_back(Tracked(1));
    buffer.push_back(Tracked(2));
    buffer.push_back(Tracked(3));
    ASSERT_EQ(Tracked::instance_count.load(), 3);

    // Partially dequeue
    buffer.pop_front();
    ASSERT_EQ(Tracked::instance_count.load(), 2);

    // Enqueue more (wrap-around)
    buffer.push_back(Tracked(4));
    buffer.push_back(Tracked(5));
    ASSERT_EQ(Tracked::instance_count.load(), 4);

    // Only partially clear (manually)
    buffer.pop_front();
    buffer.pop_front();
    ASSERT_EQ(Tracked::instance_count.load(), 2);
  }

  // All instances should be destroyed after scope
  ASSERT_EQ(Tracked::instance_count.load(), 0);
}
