#pragma once
#include <benchmark/benchmark.h>
#include "strobe/ecs/schedule/job_scheduler.hpp"
#include "small_matmul_kernel.hpp"

#include "strobe/ecs/schedule/allocator.hpp"
#include "strobe/ecs/schedule/job_scheduler.hpp"

#include <algorithm>
#include <atomic>
#include <benchmark/benchmark.h>
#include <cstdint>
#include <thread>
#include <vector>

template <uint32_t N, typename T = float>
struct SchedulerMatMulDesc {
  T const *a;
  T const *b;
  T *c;

  uint32_t matrixIndex;

  std::atomic<uint32_t> *remainingJobs;

  void run() noexcept {
    constexpr uint32_t matrixSize = N * N;
    const uint64_t offset = uint64_t{matrixIndex} * matrixSize;

    SmallMatMulKernel<N, T>::run(a + offset, b + offset, c + offset);

    remainingJobs->fetch_sub(1, std::memory_order_release);
  }
};

static void wait_until_done2(std::atomic<uint32_t> &remainingJobs) noexcept {
  while (remainingJobs.load(std::memory_order_acquire) != 0) {
    std::this_thread::yield();
  }
}

template <uint32_t N, typename T = float>
static void BM_JobSchedulerSmallMatMul(benchmark::State &state) {
  using Kernel = SmallMatMulKernel<N, T>;
  using Desc = SchedulerMatMulDesc<N, T>;

  const auto matrixCount = static_cast<uint32_t>(state.range(0));
  const auto workerCount = static_cast<uint32_t>(state.range(1));
  const auto slotCount = static_cast<uint32_t>(state.range(2));

  constexpr uint32_t matrixSize = N * N;

  assert(workerCount > 1);
  assert(slotCount >= workerCount);

  std::vector<T> a(uint64_t{matrixCount} * matrixSize);
  std::vector<T> b(uint64_t{matrixCount} * matrixSize);
  std::vector<T> c(uint64_t{matrixCount} * matrixSize);

  for (uint64_t i = 0; i < a.size(); ++i) {
    a[i] = T(0.001) * T((i * 17 + 3) & 1023);
    b[i] = T(0.001) * T((i * 31 + 7) & 1023);
  }

  std::atomic<uint32_t> remainingJobs{0};

  std::vector<Desc> descs(matrixCount);
  for (uint32_t m = 0; m < matrixCount; ++m) {
    descs[m] = Desc{
        .a = a.data(),
        .b = b.data(),
        .c = c.data(),
        .matrixIndex = m,
        .remainingJobs = &remainingJobs,
    };
  }

  strobe::ecs::allocator alloc;
  strobe::ecs::JobScheduler scheduler{&alloc, workerCount, slotCount};

  auto submit_all = [&]() noexcept {
    remainingJobs.store(matrixCount, std::memory_order_release);

    for (uint32_t m = 0; m < matrixCount; ++m) {
      Desc *desc = &descs[m];

      scheduler.submit([desc]() noexcept {
        desc->run();
      });
    }
  };

  // Untimed warm-up. This primes worker queues, slot reuse, and allocator paths.
  submit_all();
  wait_until_done2(remainingJobs);

  for (auto _ : state) {
    benchmark::DoNotOptimize(a.data());
    benchmark::DoNotOptimize(b.data());
    benchmark::DoNotOptimize(c.data());

    submit_all();
    wait_until_done2(remainingJobs);

    benchmark::ClobberMemory();
  }

  scheduler.stop();

  const uint64_t totalMatrices =
      uint64_t{matrixCount} * static_cast<uint64_t>(state.iterations());

  const uint64_t totalFlops = totalMatrices * Kernel::flops_per_matrix();

  const uint64_t totalBytes =
      totalMatrices * Kernel::bytes_per_matrix_nominal();

  state.counters["FLOP/s"] = benchmark::Counter(static_cast<double>(totalFlops),
                                                benchmark::Counter::kIsRate);

  state.counters["B/s"] = benchmark::Counter(static_cast<double>(totalBytes),
                                             benchmark::Counter::kIsRate);

  state.counters["s/matrix"] = benchmark::Counter(
      static_cast<double>(totalMatrices),
      benchmark::Counter::kIsRate | benchmark::Counter::kInvert);
}



BENCHMARK_TEMPLATE(BM_JobSchedulerSmallMatMul, 2, float)
    ->Args({1 << 20, 8, 1 << 14})
    ->UseRealTime();

BENCHMARK_TEMPLATE(BM_JobSchedulerSmallMatMul, 3, float)
    ->Args({1 << 20, 8, 1 << 14})
    ->UseRealTime();

BENCHMARK_TEMPLATE(BM_JobSchedulerSmallMatMul, 4, float)
    ->Args({1 << 20, 8, 1 << 14})
    ->UseRealTime();

BENCHMARK_TEMPLATE(BM_JobSchedulerSmallMatMul, 8, float)
    ->Args({1 << 18, 8, 1 << 14})
    ->UseRealTime();

BENCHMARK_TEMPLATE(BM_JobSchedulerSmallMatMul, 16, float)
    ->Args({1 << 16, 8, 1 << 14})
    ->UseRealTime();

BENCHMARK_TEMPLATE(BM_JobSchedulerSmallMatMul, 32, float)
    ->Args({1 << 14, 8, 1 << 14})
    ->UseRealTime();

BENCHMARK_TEMPLATE(BM_JobSchedulerSmallMatMul, 64, float)
    ->Args({1 << 12, 2, 1 << 12})
    ->Args({1 << 12, 4, 1 << 12})
    ->Args({1 << 12, 8, 1 << 12})
    ->Args({1 << 12, 16, 1 << 12})
    ->Args({1 << 12, 32, 1 << 12})
    ->Args({1 << 12, 64, 1 << 12})
    ->Args({1 << 12, 128, 1 << 12})
    ->UseRealTime();
