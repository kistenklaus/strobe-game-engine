#include <gtest/gtest.h>

#include <atomic>
#include <random>
#include <strobe/core/sync/mpsc.hpp>
#include <thread>
#include <unordered_set>
#include <strobe/core/memory/Mallocator.hpp>

// Basic Initialization Test
TEST(MPSCChannel, BasicInitialization) {
  auto [sender, receiver] = strobe::mpsc::channel<int, 8, strobe::Mallocator>();
  ASSERT_FALSE(receiver.recv().has_value()); // Receiver should be empty initially
}

// Basic Send and Receive Test (Single-Threaded)
TEST(MPSCChannel, SingleSend) {
  auto [sender, receiver] = strobe::mpsc::channel<int, 4, strobe::Mallocator>();
  ASSERT_TRUE(sender.send(1));
}

// Basic Send and Receive Test (Single-Threaded)
TEST(MPSCChannel, SendReceiveBasic) {
  auto [sender, receiver] = strobe::mpsc::channel<int, 4, strobe::Mallocator>();

  ASSERT_TRUE(sender.send(1));
  ASSERT_TRUE(sender.send(2));
  ASSERT_TRUE(sender.send(3));
  ASSERT_TRUE(sender.send(4));

  // Should not allow sending beyond capacity
  ASSERT_FALSE(sender.send(5));

  // Receive in FIFO order
  ASSERT_EQ(receiver.recv().value(), 1);
  ASSERT_EQ(receiver.recv().value(), 2);
  ASSERT_EQ(receiver.recv().value(), 3);
  ASSERT_EQ(receiver.recv().value(), 4);

  // Receiver should now be empty
  ASSERT_FALSE(receiver.recv().has_value());
}

// Memory Safety Test (Destruction)
namespace sync_testing::mpsc_test {
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
}  // namespace mpsc_test

// Memory Safety Test (Destruction)
TEST(MPSCChannel, MemorySafety) {
  using namespace sync_testing::mpsc_test;
  {
    auto [sender, receiver] = strobe::mpsc::channel<Tracked, 4, strobe::Mallocator>();
    sender.send(Tracked(1));
    sender.send(Tracked(2));
    sender.send(Tracked(3));
    sender.send(Tracked(4));
    ASSERT_EQ(Tracked::instance_count.load(), 4);

    // Clear the channel
    receiver.recv();
    receiver.recv();
    receiver.recv();
    receiver.recv();
    ASSERT_EQ(Tracked::instance_count.load(), 0);
  }
  ASSERT_EQ(Tracked::instance_count.load(), 0); // All instances should be destroyed
}

// Multi-Producer Single Consumer Test
TEST(MPSCChannel, MultiProducerSingleConsumer) {
  auto [sender, receiver] = strobe::mpsc::channel<int, 1024, strobe::Mallocator>();
  std::atomic<size_t> produced_count{0};
  std::atomic<size_t> consumed_count{0};

  // Launch multiple producer threads
  std::vector<std::thread> producers;
  for (int t = 0; t < 4; ++t) {
    producers.emplace_back([&]() {
      for (int i = 0; i < 250'000; ++i) {
        while (!sender.send(i)) {} // Busy-wait until successfully sent
        produced_count.fetch_add(1, std::memory_order_relaxed);
      }
    });
  }

  // Single consumer thread
  std::thread consumer([&]() {
    std::unordered_set<int> consumed_values;
    while (consumed_count < 1'000'000) {
      auto value = receiver.recv();
      if (value.has_value()) {
        consumed_values.insert(value.value());
        consumed_count.fetch_add(1, std::memory_order_relaxed);
      }
    }

    ASSERT_EQ(consumed_values.size(), 250'000);
  });

  for (auto& producer : producers) {
    producer.join();
  }
  consumer.join();

  ASSERT_EQ(produced_count.load(), 1'000'000);
  ASSERT_EQ(consumed_count.load(), 1'000'000);
}

// Overwrite Test (Single Consumer)
TEST(MPSCChannel, OverwriteTest) {
  auto [sender, receiver] = strobe::mpsc::channel<int, 4, strobe::Mallocator>();

  // Fill the buffer
  ASSERT_TRUE(sender.send(1));
  ASSERT_TRUE(sender.send(2));
  ASSERT_TRUE(sender.send(3));
  ASSERT_TRUE(sender.send(4));

  // Attempt to send another (should fail)
  ASSERT_FALSE(sender.send(5));

  // Dequeue and validate FIFO
  ASSERT_EQ(receiver.recv().value(), 1);
  ASSERT_EQ(receiver.recv().value(), 2);
  ASSERT_EQ(receiver.recv().value(), 3);
  ASSERT_EQ(receiver.recv().value(), 4);

  // Should be empty now
  ASSERT_FALSE(receiver.recv().has_value());
}

// Stress Test with Random Producers
TEST(MPSCChannel, StressTestRandomProducers) {
  auto [sender, receiver] = strobe::mpsc::channel<int, 16384, strobe::Mallocator>();
  std::atomic<size_t> produced_count{0};
  std::atomic<size_t> consumed_count{0};

  std::vector<std::thread> producers;
  std::mt19937 rng(std::random_device{}());

  for (int t = 0; t < 16; ++t) {
    producers.emplace_back([&]() {
      std::uniform_int_distribution<> dist(1, 100);
      for (int i = 0; i < 500'000; ++i) {
        int value = dist(rng);
        while (!sender.send(value)) {}
        produced_count.fetch_add(1, std::memory_order_relaxed);
      }
    });
  }

  std::thread consumer([&]() {
    while (consumed_count < 8'000'000) {
      if (auto value = receiver.recv(); value.has_value()) {
        consumed_count.fetch_add(1, std::memory_order_relaxed);
      }
    }
  });

  for (auto& producer : producers) {
    producer.join();
  }

  consumer.join();

  ASSERT_EQ(produced_count.load(), 8'000'000);
  ASSERT_EQ(consumed_count.load(), 8'000'000);
}


// Stress Test with Many Producers
TEST(MPSCChannel, StressTestLotsProducers) {
  auto [sender, receiver] = strobe::mpsc::channel<int, 16384, strobe::Mallocator>();
  std::atomic<size_t> produced_count{0};
  std::atomic<size_t> consumed_count{0};

  std::vector<std::thread> producers;
  for (int t = 0; t < 128; ++t) {
    producers.emplace_back([&]() {
      for (int i = 0; i < 100; ++i) {
        while (!sender.send(i)) {
        }
        produced_count.fetch_add(1, std::memory_order_relaxed);
      }
    });
  }

  std::thread consumer([&]() {
    while (consumed_count < produced_count) {
      if (auto value = receiver.recv(); value.has_value()) {
        consumed_count.fetch_add(1, std::memory_order_relaxed);
      }
    }
  });

  for (auto& producer : producers) {
    producer.join();
  }

  consumer.join();

  ASSERT_EQ(produced_count.load(), 12'800);
  ASSERT_EQ(consumed_count.load(), 12'800);
}
