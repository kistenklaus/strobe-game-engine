#pragma once
#include "small_matmul_kernel.hpp"
#include "strobe/ecs/schedule/allocator.hpp"
#include "strobe/ecs/schedule/worker_pool.hpp"
#include <algorithm>
#include <atomic>
#include <barrier>
#include <benchmark/benchmark.h>
#include <cstdint>
#include <thread>
#include <vector>

template <uint32_t N, typename T = float>
static void sequential_matmul_batch(T const *a, T const *b, T *c,
                                    uint32_t matrixCount) noexcept {
  using Kernel = SmallMatMulKernel<N, T>;
  constexpr uint32_t matrixSize = N * N;

  for (uint32_t m = 0; m < matrixCount; ++m) {
    Kernel::run(a + uint64_t{m} * matrixSize, b + uint64_t{m} * matrixSize,
                c + uint64_t{m} * matrixSize);
  }
}

template <uint32_t N, typename T = float>
static void BM_SequentialSmallMatMul(benchmark::State &state) {
  using Kernel = SmallMatMulKernel<N, T>;

  const auto matrixCount = static_cast<uint32_t>(state.range(0));
  constexpr uint32_t matrixSize = N * N;

  std::vector<T> a(uint64_t{matrixCount} * matrixSize);
  std::vector<T> b(uint64_t{matrixCount} * matrixSize);
  std::vector<T> c(uint64_t{matrixCount} * matrixSize);

  for (uint64_t i = 0; i < a.size(); ++i) {
    a[i] = T(0.001) * T((i * 17 + 3) & 1023);
    b[i] = T(0.001) * T((i * 31 + 7) & 1023);
  }

  for (auto _ : state) {
    benchmark::DoNotOptimize(a.data());
    benchmark::DoNotOptimize(b.data());
    benchmark::DoNotOptimize(c.data());

    sequential_matmul_batch<N, T>(a.data(), b.data(), c.data(), matrixCount);

    benchmark::ClobberMemory();
  }

  const uint64_t totalMatrices =
      uint64_t{matrixCount} * static_cast<uint64_t>(state.iterations());

  const uint64_t totalFlops = totalMatrices * Kernel::flops_per_matrix();

  const uint64_t totalBytes =
      totalMatrices * Kernel::bytes_per_matrix_nominal();

  // state.SetItemsProcessed(static_cast<int64_t>(totalMatrices));
  // state.SetBytesProcessed(static_cast<int64_t>(totalBytes));

  state.counters["FLOP/s"] = benchmark::Counter(static_cast<double>(totalFlops),
                                                benchmark::Counter::kIsRate);

  state.counters["B/s"] = benchmark::Counter(static_cast<double>(totalBytes),
                                             benchmark::Counter::kIsRate);
  state.counters["s/matrix"] = benchmark::Counter(
      static_cast<double>(totalMatrices),
      benchmark::Counter::kIsRate | benchmark::Counter::kInvert);
}

template <uint32_t N, typename T = float> struct WorkerPoolMatMulJob {
  using Kernel = SmallMatMulKernel<N, T>;

  T const *a;
  T const *b;
  T *c;

  uint32_t matrixIndex;

  std::atomic<uint32_t> *remainingJobs;

  void operator()(auto *) noexcept {
    constexpr uint32_t matrixSize = N * N;

    const uint64_t offset = uint64_t{matrixIndex} * matrixSize;

    Kernel::run(a + offset, b + offset, c + offset);

    remainingJobs->fetch_sub(1, std::memory_order_acq_rel);
  }
};

template <uint32_t N, typename T>
static void
submit_matmul_jobs(strobe::ecs::WorkerPool<WorkerPoolMatMulJob<N, T>> &pool,
                   strobe::ecs::job_allocator *jobAlloc,
                   std::vector<WorkerPoolMatMulJob<N, T>> &jobs,
                   std::atomic<uint32_t> &remainingJobs) noexcept {
  remainingJobs.store(static_cast<uint32_t>(jobs.size()),
                      std::memory_order_release);

  for (WorkerPoolMatMulJob<N, T> &job : jobs) {
    pool.submit(jobAlloc, &job);
  }
}

static void wait_until_done(std::atomic<uint32_t> &remainingJobs) noexcept {
  while (remainingJobs.load(std::memory_order_acquire) != 0) {
    std::this_thread::yield();
  }
}

template <uint32_t N, typename T = float>
static void BM_WorkerPoolSmallMatMul(benchmark::State &state) {
  using Kernel = SmallMatMulKernel<N, T>;
  using Job = WorkerPoolMatMulJob<N, T>;

  const auto matrixCount = static_cast<uint32_t>(state.range(0));
  const auto threadCount = static_cast<uint32_t>(state.range(1));

  constexpr uint32_t matrixSize = N * N;

  std::vector<T> a(uint64_t{matrixCount} * matrixSize);
  std::vector<T> b(uint64_t{matrixCount} * matrixSize);
  std::vector<T> c(uint64_t{matrixCount} * matrixSize);

  for (uint64_t i = 0; i < a.size(); ++i) {
    a[i] = T(0.001) * T((i * 17 + 3) & 1023);
    b[i] = T(0.001) * T((i * 31 + 7) & 1023);
  }

  std::atomic<uint32_t> remainingJobs{0};

  std::vector<Job> jobs(matrixCount);
  for (uint32_t m = 0; m < matrixCount; ++m) {
    jobs[m] = Job{
        .a = a.data(),
        .b = b.data(),
        .c = c.data(),
        .matrixIndex = m,
        .remainingJobs = &remainingJobs,
    };
  }

  strobe::ecs::allocator alloc;
  strobe::ecs::job_allocator jobAlloc{std::in_place, &alloc};

  strobe::ecs::WorkerPool<Job> pool{&jobAlloc, threadCount};

  // Untimed warm-up. This primes WorkerQueue::Pool chunks and wakes workers.
  submit_matmul_jobs(pool, &jobAlloc, jobs, remainingJobs);
  wait_until_done(remainingJobs);

  for (auto _ : state) {
    benchmark::DoNotOptimize(a.data());
    benchmark::DoNotOptimize(b.data());
    benchmark::DoNotOptimize(c.data());

    submit_matmul_jobs(pool, &jobAlloc, jobs, remainingJobs);
    wait_until_done(remainingJobs);

    benchmark::ClobberMemory();
  }

  pool.request_stop();
  pool.join();

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

template <uint32_t N, typename T = float>
static void matmul_batch_range(T const *a, T const *b, T *c,
                               uint32_t beginMatrix,
                               uint32_t endMatrix) noexcept {
  using Kernel = SmallMatMulKernel<N, T>;
  constexpr uint32_t matrixSize = N * N;

  for (uint32_t m = beginMatrix; m < endMatrix; ++m) {
    Kernel::run(a + uint64_t{m} * matrixSize, b + uint64_t{m} * matrixSize,
                c + uint64_t{m} * matrixSize);
  }
}

template <uint32_t N, typename T = float>
static void BM_StdThreadPerJobSmallMatMul(benchmark::State &state) {
  using Kernel = SmallMatMulKernel<N, T>;

  const auto matrixCount = static_cast<uint32_t>(state.range(0));
  constexpr uint32_t matrixSize = N * N;

  std::vector<T> a(uint64_t{matrixCount} * matrixSize);
  std::vector<T> b(uint64_t{matrixCount} * matrixSize);
  std::vector<T> c(uint64_t{matrixCount} * matrixSize);

  for (uint64_t i = 0; i < a.size(); ++i) {
    a[i] = T(0.001) * T((i * 17 + 3) & 1023);
    b[i] = T(0.001) * T((i * 31 + 7) & 1023);
  }

  std::vector<std::thread> threads;
  threads.reserve(matrixCount);

  for (auto _ : state) {
    benchmark::DoNotOptimize(a.data());
    benchmark::DoNotOptimize(b.data());
    benchmark::DoNotOptimize(c.data());

    threads.clear();

    for (uint32_t m = 0; m < matrixCount; ++m) {
      threads.emplace_back([&, m]() noexcept {
        matmul_batch_range<N, T>(a.data(), b.data(), c.data(), m, m + 1);
      });
    }

    for (std::thread &thread : threads) {
      thread.join();
    }

    benchmark::ClobberMemory();
  }

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

template <uint32_t N, typename T = float>
static void BM_StdThreadStaticPartitionSmallMatMul(benchmark::State &state) {
  using Kernel = SmallMatMulKernel<N, T>;

  const auto matrixCount = static_cast<uint32_t>(state.range(0));
  const auto threadCount = static_cast<uint32_t>(state.range(1));

  constexpr uint32_t matrixSize = N * N;

  std::vector<T> a(uint64_t{matrixCount} * matrixSize);
  std::vector<T> b(uint64_t{matrixCount} * matrixSize);
  std::vector<T> c(uint64_t{matrixCount} * matrixSize);

  for (uint64_t i = 0; i < a.size(); ++i) {
    a[i] = T(0.001) * T((i * 17 + 3) & 1023);
    b[i] = T(0.001) * T((i * 31 + 7) & 1023);
  }

  std::barrier startBarrier(static_cast<std::ptrdiff_t>(threadCount + 1));
  std::barrier doneBarrier(static_cast<std::ptrdiff_t>(threadCount + 1));

  std::atomic<bool> stopRequested{false};

  std::vector<std::thread> threads;
  threads.reserve(threadCount);

  for (uint32_t threadId = 0; threadId < threadCount; ++threadId) {
    threads.emplace_back([&, threadId]() noexcept {
      const uint32_t begin = uint64_t{matrixCount} * threadId / threadCount;

      const uint32_t end = uint64_t{matrixCount} * (threadId + 1) / threadCount;

      while (true) {
        startBarrier.arrive_and_wait();

        if (stopRequested.load(std::memory_order_acquire)) {
          break;
        }

        matmul_batch_range<N, T>(a.data(), b.data(), c.data(), begin, end);

        doneBarrier.arrive_and_wait();
      }
    });
  }

  for (auto _ : state) {
    benchmark::DoNotOptimize(a.data());
    benchmark::DoNotOptimize(b.data());
    benchmark::DoNotOptimize(c.data());

    startBarrier.arrive_and_wait();
    doneBarrier.arrive_and_wait();

    benchmark::ClobberMemory();
  }

  stopRequested.store(true, std::memory_order_release);
  startBarrier.arrive_and_wait();

  for (std::thread &thread : threads) {
    thread.join();
  }

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

BENCHMARK_TEMPLATE(BM_SequentialSmallMatMul, 2, float)
    ->Arg(1 << 20)
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_WorkerPoolSmallMatMul, 2, float)
    ->Args({1 << 20, 8})
    ->UseRealTime();

BENCHMARK_TEMPLATE(BM_SequentialSmallMatMul, 3, float)
    ->Arg(1 << 20)
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_WorkerPoolSmallMatMul, 3, float)
    ->Args({1 << 20, 8})
    ->UseRealTime();

BENCHMARK_TEMPLATE(BM_SequentialSmallMatMul, 4, float)
    ->Arg(1 << 20)
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_WorkerPoolSmallMatMul, 4, float)
    ->Args({1 << 20, 8})
    ->UseRealTime();

BENCHMARK_TEMPLATE(BM_SequentialSmallMatMul, 8, float)
    ->Arg(1 << 18)
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_WorkerPoolSmallMatMul, 8, float)
    ->Args({1 << 18, 8})
    ->UseRealTime();

BENCHMARK_TEMPLATE(BM_SequentialSmallMatMul, 16, float)
    ->Arg(1 << 16)
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_WorkerPoolSmallMatMul, 16, float)
    ->Args({1 << 16, 8})
    ->UseRealTime();

BENCHMARK_TEMPLATE(BM_SequentialSmallMatMul, 32, float)
    ->Arg(1 << 14)
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_WorkerPoolSmallMatMul, 32, float)
    ->Args({1 << 14, 8})
    ->UseRealTime();

BENCHMARK_TEMPLATE(BM_SequentialSmallMatMul, 64, float)
    ->Arg(1 << 12)
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_WorkerPoolSmallMatMul, 64, float)
    ->Args({1 << 12, 2})
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_WorkerPoolSmallMatMul, 64, float)
    ->Args({1 << 12, 4})
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_WorkerPoolSmallMatMul, 64, float)
    ->Args({1 << 12, 8})
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_WorkerPoolSmallMatMul, 64, float)
    ->Args({1 << 12, 16})
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_WorkerPoolSmallMatMul, 64, float)
    ->Args({1 << 12, 32})
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_WorkerPoolSmallMatMul, 64, float)
    ->Args({1 << 12, 64})
    ->UseRealTime();
BENCHMARK_TEMPLATE(BM_WorkerPoolSmallMatMul, 64, float)
    ->Args({1 << 12, 128})
    ->UseRealTime();

BENCHMARK_TEMPLATE(BM_StdThreadPerJobSmallMatMul, 16, float)
    ->Arg(256)
    ->UseRealTime();

BENCHMARK_TEMPLATE(BM_StdThreadPerJobSmallMatMul, 32, float)
    ->Arg(256)
    ->UseRealTime();

BENCHMARK_TEMPLATE(BM_StdThreadPerJobSmallMatMul, 64, float)
    ->Arg(256)
    ->UseRealTime();

BENCHMARK_TEMPLATE(BM_StdThreadStaticPartitionSmallMatMul, 16, float)
    ->Args({1 << 16, 2})
    ->Args({1 << 16, 4})
    ->Args({1 << 16, 8})
    ->Args({1 << 16, 16})
    ->UseRealTime();

BENCHMARK_TEMPLATE(BM_StdThreadStaticPartitionSmallMatMul, 32, float)
    ->Args({1 << 14, 2})
    ->Args({1 << 14, 4})
    ->Args({1 << 14, 8})
    ->Args({1 << 14, 16})
    ->UseRealTime();

BENCHMARK_TEMPLATE(BM_StdThreadStaticPartitionSmallMatMul, 64, float)
    ->Args({1 << 12, 2})
    ->Args({1 << 12, 4})
    ->Args({1 << 12, 8})
    ->Args({1 << 12, 16})
    ->Args({1 << 12, 32})
    ->UseRealTime();
