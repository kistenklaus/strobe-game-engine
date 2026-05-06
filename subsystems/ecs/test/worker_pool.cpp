#include <chrono>
#include <gtest/gtest.h>

#include "strobe/ecs/schedule/allocator.hpp"
#include "strobe/ecs/schedule/job_scheduler.hpp"
#include "strobe/ecs/schedule/worker_pool.hpp"

struct TestFn {
  std::function<void(void *)> execute;
  void operator()(auto *ctx) noexcept { execute(static_cast<void *>(ctx)); }
};

// Basic Initialization Test
TEST(WorkerPool, Basic) {
  strobe::ecs::allocator alloc;
  strobe::ecs::job_allocator job_alloc{std::in_place, &alloc};
  const uint32_t threadCount = 8;

  strobe::ecs::WorkerPool<TestFn> pool{&job_alloc, threadCount};

  int x = 1;
  TestFn fn{[&](void *) { x = 2; }};

  pool.submit(&job_alloc, &fn);

  pool.request_stop();
  pool.join();

  EXPECT_EQ(x, 2);
}
