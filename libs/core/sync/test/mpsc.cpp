#include <chrono>
#include <gtest/gtest.h>

#include <atomic>
#include <random>
#include <strobe/core/memory/Mallocator.hpp>
#include <strobe/core/sync/mpsc.hpp>
#include <thread>
#include <unordered_set>

// Basic Initialization Test
TEST(MPSCChannel, BasicInitialization) {
  auto [sender, receiver] = strobe::mpsc::channel<int, 8, strobe::Mallocator>();
  ASSERT_FALSE(
      receiver.recv().has_value()); // Receiver should be empty initially
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
  Tracked(const Tracked &other) : value(other.value) { ++instance_count; }
  Tracked(Tracked &&other) noexcept : value(other.value) { ++instance_count; }
  ~Tracked() { --instance_count; }
};

// Initialize static member
std::atomic<int> Tracked::instance_count{0};
} // namespace sync_testing::mpsc_test

// Memory Safety Test (Destruction)
TEST(MPSCChannel, MemorySafety) {
  using namespace sync_testing::mpsc_test;
  {
    auto [sender, receiver] =
        strobe::mpsc::channel<Tracked, 4, strobe::Mallocator>();
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
  ASSERT_EQ(Tracked::instance_count.load(),
            0); // All instances should be destroyed
}

// Multi-Producer Single Consumer Test
TEST(MPSCChannel, MultiProducerSingleConsumer) {
  auto [sender, receiver] = strobe::mpsc::channel<int, 8, strobe::Mallocator>();
  std::atomic<size_t> produced_count{0};
  std::atomic<size_t> consumed_count{0};

  // Launch multiple producer threads
  std::vector<std::thread> producers;
  for (int t = 0; t < 4; ++t) {
    producers.emplace_back([&]() {
      for (int i = 0; i < 250'000; ++i) {
        while (!sender.send(i)) {
        } // Busy-wait until successfully sent
        produced_count.fetch_add(1, std::memory_order_relaxed);
      }
    });
  }

  std::atomic<bool> forceStop = false;

  // Single consumer thread
  std::thread consumer([&]() {
    std::unordered_set<int> consumed_values;
    while (consumed_count < 1'000'000 && !forceStop) {
      auto value = receiver.recv();
      if (value.has_value()) {
        consumed_values.insert(value.value());
        consumed_count.fetch_add(1, std::memory_order_relaxed);
      }
    }

    ASSERT_EQ(consumed_values.size(), 250'000);
  });

  for (auto &producer : producers) {
    producer.join();
  }
  ASSERT_EQ(produced_count.load(), 1'000'000);
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  forceStop.store(true);
  consumer.join();

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
  auto [sender, receiver] =
      strobe::mpsc::channel<int, 16384, strobe::Mallocator>();
  std::atomic<unsigned int> produced_count{0};
  std::atomic<unsigned int> consumed_count{0};
  std::vector<std::thread> producers;

  for (int t = 0; t < 16; ++t) {
    producers.emplace_back([&]() {
      std::uniform_int_distribution<> dist(1, 100);
      std::mt19937 rng(std::random_device{}());
      for (int i = 0; i < 50'000; ++i) {
        int value = dist(rng);
        while (!sender.send(value)) {
        }
        produced_count.fetch_add(1);
      }
    });
  }

  std::atomic<bool> forceStop = false;

  std::thread consumer([&]() {
    while (consumed_count < 800'000 && !forceStop) {
      if (auto value = receiver.recv(); value.has_value()) {
        auto x = consumed_count.fetch_add(1);
      }
    }
  });

  for (auto &producer : producers) {
    producer.join();
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  forceStop.store(true);
  consumer.join();

  ASSERT_EQ(produced_count.load(), 800'000);
  ASSERT_EQ(consumed_count.load(), 800'000);
}

// Stress Test with Many Producers
TEST(MPSCChannel, StressTestLotsProducers) {
  auto [sender, receiver] =
      strobe::mpsc::channel<int, 16384, strobe::Mallocator>();
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

  std::atomic<bool> forceStop = false;

  std::thread consumer([&]() {
    while (consumed_count < produced_count && !forceStop) {
      if (auto value = receiver.recv(); value.has_value()) {
        consumed_count.fetch_add(1, std::memory_order_relaxed);
      }
    }
  });

  for (auto &producer : producers) {
    producer.join();
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  forceStop.store(true);

  consumer.join();

  ASSERT_EQ(produced_count.load(), 12'800);
  ASSERT_EQ(consumed_count.load(), 12'800);
}

TEST(MPSCChannel, BlockingSendAndReceiveSingleThread) {
  {
    auto [sender, receiver] =
        strobe::mpsc::channel<int, 2, strobe::Mallocator>();

    // Fill the queue to capacity with blocking_send
    sender.blocking_send(1);
    sender.blocking_send(2);

    // Attempting a normal send should fail (queue is full)
    ASSERT_FALSE(sender.send(3));

    // Drain the queue with blocking_recv
    auto v1 = receiver.recv();
    ASSERT_TRUE(v1.has_value());
    ASSERT_EQ(*v1, 1);

    auto v2 = receiver.recv();
    ASSERT_TRUE(v2.has_value());
    ASSERT_EQ(*v2, 2);

    // Now it should be empty
    ASSERT_FALSE(receiver.recv().has_value());
  }
}

TEST(MPSCChannel, BlockingSendWaitsForSpace) {
  auto [sender, receiver] = strobe::mpsc::channel<int, 1, strobe::Mallocator>();

  // Fill queue
  sender.blocking_send(1);

  std::atomic<bool> sent_second{false};

  // Try to blocking_send another in a thread; should block until space
  std::thread producer([&]() {
    sender.blocking_send(2);
    sent_second.store(true, std::memory_order_release);
  });

  // Sleep a bit to ensure producer is likely waiting
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  ASSERT_FALSE(sent_second.load(std::memory_order_acquire));

  // Drain one value, unblocking producer
  auto v = receiver.recv();
  ASSERT_TRUE(v.has_value());
  ASSERT_EQ(*v, 1);

  // Wait for producer to finish
  producer.join();
  ASSERT_TRUE(sent_second.load(std::memory_order_acquire));

  // Receive second value
  auto v2 = receiver.recv();
  ASSERT_TRUE(v2.has_value());
  ASSERT_EQ(*v2, 2);
}

TEST(MPSCChannel, BlockingReceiveWaitsForData) {
  auto [sender, receiver] = strobe::mpsc::channel<int, 2, strobe::Mallocator>();

  std::atomic<bool> received{false};

  // Consumer thread blocks on blocking_recv()
  std::thread consumer([&]() {
    int v = receiver.blocking_recv();
    ASSERT_EQ(v, 42);
    received.store(true, std::memory_order_release);
  });

  // Sleep a bit to ensure consumer is likely waiting
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  ASSERT_FALSE(received.load(std::memory_order_acquire));

  // Producer sends
  sender.send(42);

  // Wait for consumer to finish
  consumer.join();
  ASSERT_TRUE(received.load(std::memory_order_acquire));
}

TEST(MPSCChannelTest, StressTestMultipleProducersSingleConsumer) {
  using namespace strobe::mpsc;

  constexpr std::size_t Capacity = 64;
  constexpr std::size_t NumProducers = 64;
  constexpr std::size_t NumMessagesPerProducer = 10000;

  // Use std::allocator for simplicity
  auto [sender, receiver] =
      strobe::mpsc::channel<int, Capacity, strobe::Mallocator>();

  std::atomic<std::size_t> messages_sent{0};
  std::atomic<std::size_t> messages_received{0};

  // Launch producers
  std::vector<std::thread> producer_threads;
  for (std::size_t i = 0; i < NumProducers; ++i) {
    producer_threads.emplace_back([&, i]() {
      for (std::size_t j = 0; j < NumMessagesPerProducer; ++j) {
        sender.blocking_send(static_cast<int>(i * NumMessagesPerProducer + j));
        messages_sent.fetch_add(1, std::memory_order_relaxed);

        // Optional: add a small random sleep to increase stress
        if (j % 500 == 0) {
          std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
      }
    });
  }

  // Launch single consumer
  std::thread consumer_thread([&]() {
    std::unordered_set<int> seen_values;
    while (messages_received.load(std::memory_order_relaxed) <
           NumProducers * NumMessagesPerProducer) {
      int value = receiver.blocking_recv();
      // Check for duplicates (for debugging)
      ASSERT_TRUE(seen_values.insert(value).second)
          << "Duplicate value received: " << value;
      messages_received.fetch_add(1, std::memory_order_relaxed);
    }
  });

  // Join all threads
  for (auto &t : producer_threads) {
    t.join();
  }
  consumer_thread.join();

  // Final sanity checks
  EXPECT_EQ(messages_sent.load(), NumProducers * NumMessagesPerProducer);
  EXPECT_EQ(messages_received.load(), NumProducers * NumMessagesPerProducer);
}
