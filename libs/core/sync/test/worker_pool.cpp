#include <strobe/core/sync/job_scheduler.hpp>
#include <strobe/core/sync/sequencer.hpp>
#include <strobe/core/sync/worker_pool.hpp>

#include <gtest/gtest.h>

#include <atomic>
#include <cstdint>
#include <functional>

namespace {

struct TestJob {
  std::function<void(void *)> execute;

  void operator()(auto *context) noexcept {
    execute(static_cast<void *>(context));
  }
};

TEST(WorkerPool, ExecutesSubmittedJobs) {
  strobe::sync::allocator allocator;
  constexpr uint32_t workerCount = 4;
  strobe::sync::WorkerPool<TestJob> pool{&allocator, workerCount};

  std::atomic<uint32_t> executions = 0;
  TestJob job{[&](void *) { executions.fetch_add(1); }};

  pool.submit(&allocator, &job);
  pool.request_stop();
  pool.join();

  EXPECT_EQ(executions.load(), 1);
}

TEST(JobScheduler, ExecutesJobsAfterTheirDependencies) {
  strobe::sync::allocator allocator;
  strobe::sync::JobScheduler scheduler{&allocator, 4};

  std::atomic<uint32_t> phase = 0;
  const auto first = scheduler.acquire();
  const auto second = scheduler.acquire();

  scheduler.addDependency(second, first);
  scheduler.submit(second, [&]() noexcept {
    EXPECT_EQ(phase.load(std::memory_order_acquire), 1);
    phase.store(2, std::memory_order_release);
  });
  scheduler.submit(
      first, [&]() noexcept { phase.store(1, std::memory_order_release); });

  scheduler.stop();
  EXPECT_EQ(phase.load(std::memory_order_acquire), 2);
}

TEST(JobScheduler, WaitsWithoutStoppingPersistentWorkers) {
  strobe::sync::allocator allocator;
  strobe::sync::JobScheduler scheduler{&allocator, 4};

  std::atomic<uint32_t> executions = 0;
  const auto first = scheduler.submit(
      [&]() noexcept { executions.fetch_add(1, std::memory_order_release); });
  scheduler.wait(first);
  EXPECT_EQ(executions.load(std::memory_order_acquire), 1);

  const auto second = scheduler.submit(
      [&]() noexcept { executions.fetch_add(1, std::memory_order_release); });
  scheduler.wait(second);
  scheduler.wait(first);
  EXPECT_EQ(executions.load(std::memory_order_acquire), 2);

  scheduler.stop();
}

TEST(Sequencer, OrdersConflictingLocationAccesses) {
  strobe::sync::allocator allocator;
  strobe::sync::Sequencer sequencer{&allocator, 4};
  const auto location = sequencer.alloc();

  int value = 0;
  int observed = 0;
  sequencer.submit(
      [&](strobe::sync::Sequencer::MemoryScope &scope) noexcept {
        scope.release(location);
      },
      [&]() noexcept { value = 42; });
  sequencer.submit(
      [&](strobe::sync::Sequencer::MemoryScope &scope) noexcept {
        scope.acq_rel(location);
      },
      [&]() noexcept { observed = value; });
  sequencer.fence([&](strobe::sync::Sequencer::FenceScope &scope) noexcept {
    scope.acquire(location);
  });

  EXPECT_EQ(observed, 42);
  sequencer.free(location);
}

} // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
