#include <gtest/gtest.h>

#include "strobe/ecs/scheduler/allocator.hpp"
#include "strobe/ecs/scheduler/worker_pool.hpp"

struct TestFn {
  std::function<void(void *)> execute;
  void operator()(auto *ctx) noexcept { execute(static_cast<void *>(ctx)); }
};

// Basic Initialization Test
TEST(WorkerPool, Basic) {
  strobe::ecs::allocator alloc;
  strobe::ecs::scheduler::allocator job_alloc{std::in_place, &alloc};
  const uint32_t threadCount = 8;

  strobe::ecs::WorkerPool<TestFn> pool{&job_alloc, threadCount};

  int x = 1;
  TestFn fn{[&](void *) { x = 2; }};

  pool.submit(&job_alloc, &fn);

  pool.request_stop();
  pool.join();

  EXPECT_EQ(x, 2);
}
