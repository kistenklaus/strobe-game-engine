#include "strobe/core/memory/Mallocator.hpp"
#include <atomic>
#include <cassert>
#include <gtest/gtest.h>

#include <strobe/core/sync/lockfree/lock_free_bounded_ms_queue.hpp>
#include <thread>

using namespace strobe::sync;

// Basic Enqueue and Dequeue Test
TEST(LockFreeBoundedMSQueue, initalization) {
  LockFreeBoundedMSQueue<int, strobe::Mallocator> queue(2);

  ASSERT_TRUE(queue.enqueue(1));
  ASSERT_TRUE(queue.enqueue(2));

  auto x = queue.dequeue();
  auto y = queue.dequeue();
  auto z = queue.dequeue();

  ASSERT_TRUE(x.has_value());
  ASSERT_EQ(*x, 1);
  ASSERT_TRUE(y.has_value());
  ASSERT_EQ(*y, 2);
  ASSERT_FALSE(z.has_value());
}

TEST(LockFreeBoundedMSQueue, BasicEnqueueDequeue) {
  LockFreeBoundedMSQueue<int, strobe::Mallocator> queue(2);
  ASSERT_TRUE(queue.enqueue(1));
  ASSERT_TRUE(queue.enqueue(2));
  ASSERT_EQ(queue.dequeue(), 1);
  ASSERT_EQ(queue.dequeue(), 2);
  ASSERT_FALSE(queue.dequeue().has_value());
}

TEST(LockFreeBoundedMSQueue, FIFOOrder) {
  LockFreeBoundedMSQueue<int, strobe::Mallocator> queue(4);
  for (int i = 1; i <= 4; ++i) {
    ASSERT_TRUE(queue.enqueue(i));
  }
  for (int i = 1; i <= 4; ++i) {
    auto value = queue.dequeue();
    ASSERT_TRUE(value.has_value());
    ASSERT_EQ(*value, i);
  }
}

TEST(LockFreeBoundedMSQueue, CapacityLimit) {
  LockFreeBoundedMSQueue<int, strobe::Mallocator> queue(2);
  ASSERT_TRUE(queue.enqueue(1));
  ASSERT_TRUE(queue.enqueue(2));
  // Next enqueue should fail because pool is exhausted
  ASSERT_FALSE(queue.enqueue(3));
}

TEST(LockFreeBoundedMSQueue, EmptyQueue) {
  LockFreeBoundedMSQueue<int, strobe::Mallocator> queue(2);
  auto value = queue.dequeue();
  ASSERT_FALSE(value.has_value());
}

TEST(LockFreeBoundedMSQueue, RepeatedCycles) {
  LockFreeBoundedMSQueue<int, strobe::Mallocator> queue(2);
  for (int cycle = 0; cycle < 10; ++cycle) {
    ASSERT_TRUE(queue.enqueue(cycle));
    ASSERT_TRUE(queue.enqueue(cycle + 100));
    auto x = queue.dequeue();
    auto y = queue.dequeue();
    ASSERT_TRUE(x.has_value());
    ASSERT_TRUE(y.has_value());
    ASSERT_FALSE(queue.dequeue().has_value());
  }
}

TEST(LockFreeBoundedMSQueue, SingleProducerStress) {
  LockFreeBoundedMSQueue<int, strobe::Mallocator> queue(32);
  constexpr int iterations = 1000;
  for (int i = 0; i < iterations; ++i) {
    ASSERT_TRUE(queue.enqueue(i));
    auto value = queue.dequeue();
    ASSERT_TRUE(value.has_value());
    ASSERT_EQ(*value, i);
  }
}

TEST(LockFreeBoundedMSQueue, LargeObjectSupport) {

  struct LargeObject {
    alignas(64) char data[128];
    int id;
  };
  LockFreeBoundedMSQueue<LargeObject, strobe::Mallocator> queue(4);
  for (int i = 0; i < 4; ++i) {
    LargeObject obj{};
    obj.id = i;
    ASSERT_TRUE(queue.enqueue(obj));
  }
  for (int i = 0; i < 4; ++i) {
    auto value = queue.dequeue();
    ASSERT_TRUE(value.has_value());
    ASSERT_EQ(value->id, i);
  }
}

TEST(LockFreeBoundedMSQueue, DequeueFromEmptyQueue) {

  LockFreeBoundedMSQueue<int, strobe::Mallocator, false> queue(4);
  auto val = queue.dequeue();
  ASSERT_FALSE(val.has_value());
}

TEST(LockFreeBoundedMSQueue, InterleavedEnqueueDequeue) {
  LockFreeBoundedMSQueue<int, strobe::Mallocator, false> queue(4);
  for (int i = 0; i < 10; ++i) {
    ASSERT_TRUE(queue.enqueue(i));
    auto val = queue.dequeue();
    ASSERT_TRUE(val.has_value());
    ASSERT_EQ(*val, i);
  }
  ASSERT_FALSE(queue.dequeue().has_value());
}

TEST(LockFreeBoundedMSQueue, QueueFullBehavior) {
  LockFreeBoundedMSQueue<int, strobe::Mallocator, false> queue(2);
  ASSERT_TRUE(queue.enqueue(1));
  ASSERT_TRUE(queue.enqueue(2));
  ASSERT_FALSE(queue.enqueue(3)); // Should fail when at capacity
}

// TODO multiple producers and in parallel single consumer tests.

TEST(LockFreeBoundedMSQueue, MultipleProducersSingleConsumer) {

  constexpr int NumProducers = 8;
  constexpr int ItemsPerProducer = 100000;

  using base_t = std::size_t;
  LockFreeBoundedMSQueue<base_t, strobe::Mallocator, false> queue(
      NumProducers);

  std::vector<std::thread> producers;
  for (std::size_t i = 0; i < NumProducers; ++i) {
    producers.emplace_back([&]() {
      for (int j = 0; j < ItemsPerProducer; ++j) {
        while (!queue.enqueue(static_cast<base_t>(j + 1))) {
        }
        std::this_thread::yield();
      }
    });
  }

  std::atomic<bool> succ = false;
  std::atomic<bool> abort = false;
  std::thread consumer([&]() {
    for (std::size_t i = 0; i < ItemsPerProducer * NumProducers; ++i) {
      std::optional<int> v;
      while (!(v = queue.dequeue()).has_value() && !abort) {
      }

      if (abort) {
        return;
      }
    }
    succ = true;
  });

  for (auto &t : producers) {
    t.join();
  }
  consumer.join();
  ASSERT_TRUE(succ);
}

TEST(LockFreeBoundedMSQueue, MultipleProducersSingleConsumer_ComplexType) {

  constexpr int NumProducers = 64;
  constexpr int ItemsPerProducer = 10000;

  LockFreeBoundedMSQueue<std::shared_ptr<int>, strobe::Mallocator, false> queue(
      NumProducers * ItemsPerProducer);

  std::vector<std::thread> producers;
  for (std::size_t i = 0; i < NumProducers; ++i) {
    producers.emplace_back([&]() {
      for (int j = 0; j < ItemsPerProducer; ++j) {
        while (!queue.enqueue(std::make_shared<int>(j))) {
        }
      }
    });
  }

  std::atomic<bool> succ = false;
  std::atomic<bool> abort = false;
  std::thread consumer([&]() {
    for (std::size_t i = 0; i < ItemsPerProducer * NumProducers; ++i) {
      std::optional<std::shared_ptr<int>> v;
      while (!(v = queue.dequeue()).has_value() && !abort) {
      }

      if (abort) {
        return;
      }
    }
    succ = true;
  });

  for (auto &t : producers) {
    t.join();
  }
  consumer.join();
  ASSERT_TRUE(succ);
}


TEST(LockFreeBoundedMSQueueTest, DestructorCorrectness) {
  // Tracking variables scoped to this test:
  static std::atomic<int> nextId{0};
  static std::mutex mutex;
  static std::set<int> constructedIds;
  static std::set<int> destructedIds;

  struct TestType {
    int id;

    TestType() : id(nextId++) {
      std::lock_guard<std::mutex> lock(mutex);
      constructedIds.insert(id);
    }

    ~TestType() {
      std::lock_guard<std::mutex> lock(mutex);
      destructedIds.insert(id);
    }

    TestType(const TestType&) = delete;
    TestType& operator=(const TestType&) = delete;
    TestType(TestType&&) = default;
    TestType& operator=(TestType&&) = default;
  };

  // Reset tracking
  {
    std::lock_guard<std::mutex> lock(mutex);
    constructedIds.clear();
    destructedIds.clear();
    nextId.store(0);
  }

  constexpr int capacity = 16;
  strobe::sync::LockFreeBoundedMSQueue<TestType, strobe::Mallocator> queue(capacity);

  for (int i = 0; i < capacity; ++i) {
    EXPECT_TRUE(queue.enqueue());
  }

  for (int i = 0; i < capacity; ++i) {
    auto item = queue.dequeue();
    EXPECT_TRUE(item.has_value());
  }

  {
    std::optional<TestType> item;
    while ((item = queue.dequeue()).has_value()) {}
  }

  {
    std::lock_guard<std::mutex> lock(mutex);
    EXPECT_EQ(constructedIds.size(), destructedIds.size())
        << "Every constructed object should have exactly one destructor call.";
    EXPECT_EQ(constructedIds, destructedIds)
        << "Constructed and destructed IDs should match.";
  }
}
