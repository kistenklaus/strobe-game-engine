#include <strobe/core/sync/job_scheduler.hpp>

#include <gtest/gtest.h>

#include <array>
#include <atomic>
#include <cstdint>

namespace {

TEST(JobScheduler, RunsFanInConsumerExactlyOnce) {
  strobe::sync::allocator allocator;
  strobe::sync::JobScheduler scheduler{&allocator, 8};

  constexpr std::uint32_t source_count = 3;
  std::array<std::atomic<std::uint32_t>, source_count> sources{};
  std::atomic<std::uint32_t> consumer_runs = 0;

  const auto consumer = scheduler.acquire();
  std::array<strobe::sync::job_id, source_count> source_jobs;
  for (std::uint32_t index = 0; index < source_count; ++index) {
    source_jobs[index] = scheduler.acquire();
    scheduler.addDependency(consumer, source_jobs[index]);
  }

  scheduler.submit(consumer, [&consumer_runs]() noexcept {
    consumer_runs.fetch_add(1, std::memory_order_relaxed);
  });
  for (std::uint32_t index = 0; index < source_count; ++index) {
    scheduler.submit(source_jobs[index], [&sources, index]() noexcept {
      sources[index].fetch_add(1, std::memory_order_relaxed);
    });
  }

  scheduler.wait(consumer);

  EXPECT_EQ(consumer_runs.load(std::memory_order_relaxed), 1);
  for (const auto &source : sources) {
    EXPECT_EQ(source.load(std::memory_order_relaxed), 1);
  }
}

TEST(JobScheduler, ReusesSlotsAfterConcurrentFanInCompletion) {
  strobe::sync::allocator allocator;
  strobe::sync::JobScheduler scheduler{&allocator, 8};

  constexpr std::uint32_t source_count = 7;
  constexpr std::uint32_t round_count = 10000;
  std::atomic<std::uint32_t> consumer_runs{0};

  for (std::uint32_t round = 0; round < round_count; ++round) {
    const auto consumer = scheduler.acquire();
    std::array<strobe::sync::job_id, source_count> sources;
    for (auto &source : sources) {
      source = scheduler.acquire();
      scheduler.addDependency(consumer, source);
    }

    scheduler.submit(consumer, [&consumer_runs]() noexcept {
      consumer_runs.fetch_add(1, std::memory_order_relaxed);
    });
    for (const auto source : sources) {
      scheduler.submit(source, []() noexcept {});
    }
    scheduler.wait(consumer);
  }

  EXPECT_EQ(consumer_runs.load(std::memory_order_relaxed), round_count);
}

TEST(JobScheduler, RegistersDependenciesWhileSourcesComplete) {
  strobe::sync::allocator allocator;
  strobe::sync::JobScheduler scheduler{&allocator, 8};

  constexpr std::uint32_t round_count = 10000;
  std::atomic<std::uint32_t> consumer_runs{0};

  for (std::uint32_t round = 0; round < round_count; ++round) {
    const auto source = scheduler.submit([]() noexcept {});
    const auto consumer = scheduler.acquire();
    scheduler.addDependency(consumer, source);
    scheduler.submit(consumer, [&consumer_runs]() noexcept {
      consumer_runs.fetch_add(1, std::memory_order_relaxed);
    });
    scheduler.wait(consumer);
  }

  EXPECT_EQ(consumer_runs.load(std::memory_order_relaxed), round_count);
}

TEST(JobScheduler, ExecutesDependencyGraphWithOneWorker) {
  strobe::sync::allocator allocator;
  strobe::sync::JobScheduler scheduler{&allocator, 1};

  std::atomic<std::uint32_t> phase = 0;
  const auto first = scheduler.submit([&]() noexcept {
    phase.fetch_add(1, std::memory_order_release);
  });
  const auto second = scheduler.acquire();
  scheduler.addDependency(second, first);
  scheduler.submit(second, [&]() noexcept {
    EXPECT_EQ(phase.load(std::memory_order_acquire), 1);
    phase.fetch_add(1, std::memory_order_release);
  });

  scheduler.wait(second);
  scheduler.stop();
  EXPECT_EQ(phase.load(std::memory_order_acquire), 2);
}

TEST(JobScheduler, ReportsSubmittedJobCompletion) {
  strobe::sync::allocator allocator;
  strobe::sync::JobScheduler scheduler{&allocator, 1};

  std::atomic<bool> entered = false;
  std::atomic<bool> proceed = false;
  const auto job = scheduler.submit([&]() noexcept {
    entered.store(true, std::memory_order_release);
    while (!proceed.load(std::memory_order_acquire)) {
    }
  });
  while (!entered.load(std::memory_order_acquire)) {
  }

  EXPECT_FALSE(scheduler.completed(job));
  proceed.store(true, std::memory_order_release);
  scheduler.wait(job);
  EXPECT_TRUE(scheduler.completed(job));
  scheduler.stop();
}

} // namespace
