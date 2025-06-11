#include <gtest/gtest.h>

#include <atomic>
#include <random>
#include <strobe/core/sync/spsc.hpp>
#include <thread>

// Namespace for SPSC Test
namespace sync_testing::spsc_test {
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
}  // namespace sync_testing::spsc_test

using namespace sync_testing::spsc_test;

// Basic Initialization Test
TEST(SPSCChannel, BasicInitialization) {
  auto [sender, receiver] = strobe::spsc::channel<int, 8, strobe::Mallocator>(8);
  ASSERT_FALSE(receiver.recv().has_value()); // Receiver should be empty initially
}

// Single-Threaded Send and Receive Test
TEST(SPSCChannel, SingleSendReceive) {
  auto [sender, receiver] = strobe::spsc::channel<int, 4, strobe::Mallocator>(4);

  ASSERT_TRUE(sender.send(1));
  ASSERT_TRUE(sender.send(2));

  ASSERT_EQ(receiver.recv().value(), 1);
  ASSERT_EQ(receiver.recv().value(), 2);
  ASSERT_FALSE(receiver.recv().has_value());
}

// Memory Safety Test
TEST(SPSCChannel, MemorySafety) {
  auto [sender, receiver] = strobe::spsc::channel<Tracked, 4, strobe::Mallocator>(4);

  sender.send(Tracked(1));
  sender.send(Tracked(2));
  sender.send(Tracked(3));

  ASSERT_EQ(Tracked::instance_count.load(), 3);

  receiver.recv();
  receiver.recv();
  receiver.recv();

  ASSERT_EQ(Tracked::instance_count.load(), 0);
}

// Multi-Threaded Test (Single Producer, Single Consumer)
TEST(SPSCChannel, MultiThreadedSendReceive) {
  auto [sender, receiver] = strobe::spsc::channel<int, 1024, strobe::Mallocator>(1024);
  std::atomic<size_t> produced_count{0};
  std::atomic<size_t> consumed_count{0};

  std::thread producer([&]() {
    for (int i = 0; i < 100'000; ++i) {
      while (!sender.send(i)) {}
      ++produced_count;
    }
  });

  std::thread consumer([&]() {
    while (consumed_count < 100'000) {
      if (auto value = receiver.recv(); value.has_value()) {
        ++consumed_count;
      }
    }
  });

  producer.join();
  consumer.join();

  ASSERT_EQ(produced_count.load(), 100'000);
  ASSERT_EQ(consumed_count.load(), 100'000);
}
