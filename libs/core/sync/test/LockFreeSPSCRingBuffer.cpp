#include <gtest/gtest.h>

#include <atomic>
#include <random>
#include <strobe/core/sync/lockfree/spsc_ring_buffer.hpp>
#include <thread>

using namespace strobe::sync;

// Basic Enqueue and Dequeue Test
TEST(LockFreeSPSCRingBuffer, EnqueueDequeue) {
  InplaceLockFreeSPSCRingBuffer<int, 8> buffer;
  ASSERT_TRUE(buffer.enqueue(1));
  ASSERT_TRUE(buffer.enqueue(2));
  ASSERT_TRUE(buffer.enqueue(3));

  ASSERT_EQ(buffer.dequeue().value(), 1);
  ASSERT_EQ(buffer.dequeue().value(), 2);
  ASSERT_EQ(buffer.dequeue().value(), 3);
  ASSERT_FALSE(buffer.dequeue().has_value());
}

TEST(LockFreeSPSCRingBuffer, EnqueueDequeueDynamic) {
  LockFreeSPSCRingBuffer<int, strobe::Mallocator> buffer{8, strobe::Mallocator{}};
  ASSERT_TRUE(buffer.enqueue(1));
  ASSERT_TRUE(buffer.enqueue(2));
  ASSERT_TRUE(buffer.enqueue(3));
  ASSERT_EQ(buffer.dequeue().value(), 1);
  ASSERT_EQ(buffer.dequeue().value(), 2);
  ASSERT_EQ(buffer.dequeue().value(), 3);
  ASSERT_FALSE(buffer.dequeue().has_value());
}



// True Wrap-Around Test (Corrected)
TEST(LockFreeSPSCRingBuffer, WrapAroundTest) {
  InplaceLockFreeSPSCRingBuffer<int, 4> buffer;

  // Fill the buffer to full capacity (4 items)
  ASSERT_TRUE(buffer.enqueue(1));
  ASSERT_TRUE(buffer.enqueue(2));
  ASSERT_TRUE(buffer.enqueue(3));
  ASSERT_TRUE(buffer.enqueue(4));
  ASSERT_FALSE(buffer.enqueue(5));  // This should fail because it's full

  // Remove one to create space
  ASSERT_EQ(buffer.dequeue().value(), 1);

  // Add one more to trigger a wrap-around
  ASSERT_TRUE(buffer.enqueue(5));

  // Validate wrap-around behavior (FIFO)
  ASSERT_EQ(buffer.dequeue().value(), 2);
  ASSERT_EQ(buffer.dequeue().value(), 3);
  ASSERT_EQ(buffer.dequeue().value(), 4);
  ASSERT_EQ(buffer.dequeue().value(), 5);
  ASSERT_FALSE(buffer.dequeue().has_value());
}

// Multi-Threaded SPSC Test (Basic)
TEST(LockFreeSPSCRingBuffer, MultiThreadedProducerConsumer) {
  InplaceLockFreeSPSCRingBuffer<int, 1024 * 16> buffer;
  std::atomic<size_t> produced_count{0};
  std::atomic<size_t> consumed_count{0};
  std::atomic<bool> stop_flag{false};

  // Producer thread
  std::thread producer([&]() {
    for (int i = 0; i < 100'000; ++i) {
      while (!buffer.enqueue(i)) {
      }  // Spin without any sleep
      produced_count.fetch_add(1, std::memory_order_relaxed);
    }
    stop_flag.store(true, std::memory_order_release);
  });

  // Consumer thread
  std::thread consumer([&]() {
    int last_value = -1;
    while (!stop_flag.load(std::memory_order_acquire) ||
           consumed_count < 100'000) {
      auto value = buffer.dequeue();
      if (value.has_value()) {
        ASSERT_GT(value.value(), last_value);
        last_value = value.value();
        consumed_count.fetch_add(1, std::memory_order_relaxed);
      }
    }
  });

  producer.join();
  consumer.join();

  ASSERT_EQ(produced_count.load(), 100'000);
  ASSERT_EQ(consumed_count.load(), 100'000);
}

// Balanced Throughput Test
TEST(LockFreeSPSCRingBuffer, BalancedThroughput) {
  InplaceLockFreeSPSCRingBuffer<int, 1024> buffer;
  std::atomic<size_t> produced_count{0};
  std::atomic<size_t> consumed_count{0};
  std::atomic<bool> stop_flag{false};

  std::thread producer([&]() {
    for (int i = 0; i < 100'000; ++i) {
      while (!buffer.enqueue(i)) {
      }
      produced_count.fetch_add(1, std::memory_order_relaxed);
    }
    stop_flag.store(true, std::memory_order_release);
  });

  std::thread consumer([&]() {
    while (!stop_flag.load(std::memory_order_acquire) ||
           consumed_count < 100'000) {
      auto value = buffer.dequeue();
      if (value.has_value()) {
        consumed_count.fetch_add(1, std::memory_order_relaxed);
      }
    }
  });

  producer.join();
  consumer.join();
  ASSERT_EQ(produced_count.load(), 100'000);
  ASSERT_EQ(consumed_count.load(), 100'000);
}

// High Contention Test
TEST(LockFreeSPSCRingBuffer, HighContentionTest) {
  InplaceLockFreeSPSCRingBuffer<int, 1024> buffer;
  std::atomic<size_t> produced_count{0};
  std::atomic<size_t> consumed_count{0};
  std::atomic<bool> stop_flag{false};

  std::thread producer([&]() {
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<> dist(1, 3);  // Random increments (1 to 3)
    for (int i = 0; i < 100'000; ++i) {
      while (!buffer.enqueue(i)) {
      }
      produced_count.fetch_add(1, std::memory_order_relaxed);
    }
    stop_flag.store(true, std::memory_order_release);
  });

  std::thread consumer([&]() {
    while (!stop_flag.load(std::memory_order_acquire) ||
           consumed_count < 100'000) {
      auto value = buffer.dequeue();
      if (value.has_value()) {
        consumed_count.fetch_add(1, std::memory_order_relaxed);
      }
    }
  });

  producer.join();
  consumer.join();
  ASSERT_EQ(produced_count.load(), 100'000);
  ASSERT_EQ(consumed_count.load(), 100'000);
}

// Namespace for SPSC Test
namespace spsc_test {
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
TEST(LockFreeSPSCRingBuffer, MemorySafety) {
  using namespace spsc_test;
  {
    InplaceLockFreeSPSCRingBuffer<spsc_test::Tracked, 4> buffer;
    ASSERT_EQ(Tracked::instance_count.load(), 0);

    buffer.enqueue(Tracked(1));
    buffer.enqueue(Tracked(2));
    buffer.enqueue(Tracked(3));
    buffer.enqueue(Tracked(4));
    ASSERT_EQ(Tracked::instance_count.load(), 4);

    // Dequeue and clear
    buffer.clear();
    ASSERT_EQ(Tracked::instance_count.load(), 0);
  }

  // All instances should be destroyed
  ASSERT_EQ(Tracked::instance_count.load(), 0);
}
