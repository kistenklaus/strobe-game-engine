#pragma once

#include <benchmark/benchmark.h>

#include <array>
#include <atomic>
#include <barrier>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <thread>
#include <vector>

#include "strobe/core/memory/mallocator.hpp"
#include "strobe/core/memory/monotonic_pool_allocator.hpp"
#include "strobe/core/memory/mpmc_monotonic_pool_resource.hpp"
#include "strobe/core/memory/mpsc_monotonic_pool_resource.hpp"
#include "strobe/core/memory/sync_monotonic_pool_resource.hpp"
#include "strobe/core/memory/sync_resource.hpp"

namespace strobe {
namespace {

constexpr std::size_t pool_block_size = 64;
constexpr std::size_t pool_block_align = 16;
constexpr std::size_t prewarm_block_count = 64;
constexpr std::size_t maximum_consumer_count = 7;
constexpr std::size_t random_batch_size = 4096;
constexpr std::size_t random_consumer_count = 3;

template <std::size_t BlockSize>
using MonotonicPool =
    MonotonicPoolResource<BlockSize, pool_block_align, Mallocator>;

template <std::size_t BlockSize>
using LockedMonotonicPool = SyncResource<MonotonicPool<BlockSize>>;

template <std::size_t BlockSize>
using SynchronizedMonotonicPool =
    SyncMonotonicPoolResource<BlockSize, pool_block_align, Mallocator>;

template <std::size_t BlockSize>
using MpscMonotonicPool =
    MPSCMonotonicPoolResource<BlockSize, pool_block_align, Mallocator>;

template <std::size_t BlockSize> struct alignas(pool_block_align) MpmcValue {
  std::byte bytes[BlockSize];
};

template <std::size_t BlockSize>
using MpmcMonotonicPool =
    MPMCMonotonicPoolResource<MpmcValue<BlockSize>, Mallocator>;

template <std::size_t BlockSize> struct MallocatorAdapter {
  Mallocator resource;

  void *allocate() {
    return resource.allocate(BlockSize, pool_block_align);
  }

  void deallocate(void *pointer) {
    resource.deallocate(pointer, BlockSize, pool_block_align);
  }
};

template <std::size_t BlockSize> struct LockedMonotonicPoolAdapter {
  LockedMonotonicPool<BlockSize> resource;

  void *allocate() {
    return resource.locked(
        [](MonotonicPool<BlockSize> &pool) { return pool.allocate(); });
  }

  void deallocate(void *pointer) {
    resource.locked([pointer](MonotonicPool<BlockSize> &pool) {
      pool.deallocate(pointer);
    });
  }
};

template <std::size_t BlockSize> struct SynchronizedMonotonicPoolAdapter {
  SynchronizedMonotonicPool<BlockSize> resource;

  void *allocate() { return resource.allocate(); }

  void deallocate(void *pointer) { resource.deallocate(pointer); }
};

template <std::size_t BlockSize> struct MpscMonotonicPoolAdapter {
  MpscMonotonicPool<BlockSize> resource;

  void *allocate() { return resource.allocate(); }

  void deallocate(void *pointer) { resource.deallocate(pointer); }
};

template <std::size_t BlockSize> struct MpmcResourceAdapter {
  MpmcMonotonicPool<BlockSize> resource;

  void *allocate() { return resource.allocate(); }

  void deallocate(void *pointer) {
    resource.deallocate(
        static_cast<typename MpmcMonotonicPool<BlockSize>::Node *>(pointer));
  }
};

template <std::size_t Count, typename Adapter> void prewarm(Adapter &adapter) {
  std::array<void *, Count> pointers;

  for (void *&pointer : pointers) {
    pointer = adapter.allocate();
  }
  for (void *pointer : pointers) {
    adapter.deallocate(pointer);
  }
}

template <typename Adapter>
void run_single_threaded_pool(benchmark::State &state) {
  Adapter adapter;
  prewarm<prewarm_block_count>(adapter);

  for (auto _ : state) {
    void *pointer = adapter.allocate();
    benchmark::DoNotOptimize(pointer);
    adapter.deallocate(pointer);
  }

  state.SetItemsProcessed(state.iterations());
  state.SetBytesProcessed(state.iterations() *
                          static_cast<std::int64_t>(pool_block_size));
}

void BM_Pool_Mallocator(benchmark::State &state) {
  run_single_threaded_pool<MallocatorAdapter<pool_block_size>>(state);
}

void BM_Pool_SyncResourceMonotonicPool(benchmark::State &state) {
  run_single_threaded_pool<LockedMonotonicPoolAdapter<pool_block_size>>(state);
}

void BM_Pool_SyncMonotonicPool(benchmark::State &state) {
  run_single_threaded_pool<
      SynchronizedMonotonicPoolAdapter<pool_block_size>>(state);
}

void BM_Pool_MPSCMonotonicPool(benchmark::State &state) {
  run_single_threaded_pool<MpscMonotonicPoolAdapter<pool_block_size>>(state);
}

struct alignas(64) HandoffSlot {
  std::atomic<void *> pointer{nullptr};
};

template <typename Adapter> struct MpscContext {
  MpscContext() { prewarm<prewarm_block_count>(adapter); }

  Adapter adapter;
  std::array<HandoffSlot, maximum_consumer_count> slots;
};

template <typename Adapter> void run_mpsc_pool(benchmark::State &state) {
  static MpscContext<Adapter> context;
  const auto consumer_count = static_cast<std::size_t>(state.threads() - 1);

  if (consumer_count == 0) {
    for (auto _ : state) {
      void *pointer = context.adapter.allocate();
      benchmark::DoNotOptimize(pointer);
      context.adapter.deallocate(pointer);
    }
  } else if (state.thread_index() == 0) {
    for (auto _ : state) {
      for (std::size_t index = 0; index < consumer_count; ++index) {
        HandoffSlot &slot = context.slots[index];
        while (slot.pointer.load(std::memory_order_acquire) != nullptr) {
        }

        void *pointer = context.adapter.allocate();
        benchmark::DoNotOptimize(pointer);
        slot.pointer.store(pointer, std::memory_order_release);
      }
    }
  } else {
    HandoffSlot &slot =
        context.slots[static_cast<std::size_t>(state.thread_index() - 1)];

    for (auto _ : state) {
      void *pointer = nullptr;
      while ((pointer = slot.pointer.exchange(nullptr,
                                              std::memory_order_acquire)) ==
             nullptr) {
      }
      context.adapter.deallocate(pointer);
    }
  }

  // Google Benchmark sums both counters and elapsed time across participating
  // threads. Reporting the pipeline total from every thread preserves the
  // aggregate completed-pairs rate after that reduction.
  const auto operations_per_iteration =
      static_cast<std::int64_t>(consumer_count == 0 ? 1 : consumer_count);
  const auto operations = state.iterations() * operations_per_iteration;
  state.SetItemsProcessed(operations);
  state.SetBytesProcessed(operations *
                          static_cast<std::int64_t>(pool_block_size));
}

void BM_MPSC_Mallocator(benchmark::State &state) {
  run_mpsc_pool<MallocatorAdapter<pool_block_size>>(state);
}

void BM_MPSC_MPSCMonotonicPool(benchmark::State &state) {
  run_mpsc_pool<MpscMonotonicPoolAdapter<pool_block_size>>(state);
}

void BM_MPSC_SyncResourceOfMonotonicPool(benchmark::State &state) {
  run_mpsc_pool<LockedMonotonicPoolAdapter<pool_block_size>>(state);
}

void BM_MPSC_SyncMonotonicPool(benchmark::State &state) {
  run_mpsc_pool<SynchronizedMonotonicPoolAdapter<pool_block_size>>(state);
}

void BM_MPSC_MPMCMonotonicPool(benchmark::State &state) {
  run_mpsc_pool<MpmcResourceAdapter<pool_block_size>>(state);
}

inline std::uint64_t next_random(std::uint64_t &state) {
  state ^= state << 13;
  state ^= state >> 7;
  state ^= state << 17;
  return state;
}

template <std::size_t BlockSize, typename Adapter>
void run_random_mpsc_pool(benchmark::State &state) {
  const auto thread_count = static_cast<std::size_t>(state.range(0));
  const std::size_t consumer_count = thread_count - 1;
  assert(thread_count >= 1 && thread_count <= random_consumer_count + 1);

  Adapter adapter;
  prewarm<random_batch_size>(adapter);

  std::array<void *, random_batch_size> pointers;
  std::array<std::vector<std::size_t>, random_consumer_count + 1>
      deallocation_indices;
  std::uint64_t random_state = 0x9e3779b97f4a7c15ULL;

  for (std::size_t index = 0; index < random_batch_size; ++index) {
    const std::uint64_t random = next_random(random_state);
    std::size_t owner = 0;
    if (consumer_count != 0 && (random & 3U) != 0) {
      owner = 1 + ((random >> 2U) % consumer_count);
    }
    deallocation_indices[owner].push_back(index);
  }

  std::barrier phase{static_cast<std::ptrdiff_t>(thread_count)};
  std::atomic<bool> stop{false};
  std::array<std::thread, random_consumer_count> consumers;

  for (std::size_t index = 0; index < consumer_count; ++index) {
    consumers[index] = std::thread([&, index] {
      while (true) {
        phase.arrive_and_wait();
        if (stop.load(std::memory_order_acquire)) {
          break;
        }

        for (std::size_t pointer_index : deallocation_indices[index + 1]) {
          adapter.deallocate(pointers[pointer_index]);
        }

        phase.arrive_and_wait();
      }
    });
  }

  for (auto _ : state) {
    for (std::size_t index = 0; index < random_batch_size; ++index) {
      pointers[index] = adapter.allocate();
      assert(pointers[index] != nullptr);
    }

    phase.arrive_and_wait();

    for (std::size_t pointer_index : deallocation_indices[0]) {
      assert(pointers[pointer_index] != nullptr);
      adapter.deallocate(pointers[pointer_index]);
    }

    phase.arrive_and_wait();
  }

  stop.store(true, std::memory_order_release);
  phase.arrive_and_wait();
  for (std::size_t index = 0; index < consumer_count; ++index) {
    consumers[index].join();
  }

  const auto operations =
      state.iterations() * static_cast<std::int64_t>(random_batch_size);
  state.SetItemsProcessed(operations);
  state.SetBytesProcessed(operations * static_cast<std::int64_t>(BlockSize));
}

template <std::size_t BlockSize>
void BM_MPSC_Random_Mallocator(benchmark::State &state) {
  run_random_mpsc_pool<BlockSize, MallocatorAdapter<BlockSize>>(state);
}

template <std::size_t BlockSize>
void BM_MPSC_Random_MPSCMonotonicPool(benchmark::State &state) {
  run_random_mpsc_pool<BlockSize, MpscMonotonicPoolAdapter<BlockSize>>(state);
}

template <std::size_t BlockSize>
void BM_MPSC_Random_SyncResourceOfMonotonicPool(benchmark::State &state) {
  run_random_mpsc_pool<BlockSize, LockedMonotonicPoolAdapter<BlockSize>>(state);
}

template <std::size_t BlockSize>
void BM_MPSC_Random_SyncMonotonicPool(benchmark::State &state) {
  run_random_mpsc_pool<BlockSize,
                       SynchronizedMonotonicPoolAdapter<BlockSize>>(state);
}

template <std::size_t BlockSize>
void BM_MPSC_Random_MPMCMonotonicPool(benchmark::State &state) {
  run_random_mpsc_pool<BlockSize, MpmcResourceAdapter<BlockSize>>(state);
}

template <typename Adapter> struct MpmcContext {
  MpmcContext() { prewarm<prewarm_block_count>(adapter); }

  Adapter adapter;
};

template <typename Adapter> void run_mpmc_pool(benchmark::State &state) {
  static MpmcContext<Adapter> context;

  for (auto _ : state) {
    void *pointer = context.adapter.allocate();
    benchmark::DoNotOptimize(pointer);
    context.adapter.deallocate(pointer);
  }

  const auto thread_count = static_cast<std::int64_t>(state.threads());
  const auto operations = state.iterations() * thread_count;
  state.SetItemsProcessed(operations);
  state.SetBytesProcessed(operations *
                          static_cast<std::int64_t>(pool_block_size));
}

void BM_MPMC_Mallocator(benchmark::State &state) {
  run_mpmc_pool<MallocatorAdapter<pool_block_size>>(state);
}

void BM_MPMC_SyncResourceOfMonotonicPool(benchmark::State &state) {
  run_mpmc_pool<LockedMonotonicPoolAdapter<pool_block_size>>(state);
}

void BM_MPMC_SyncMonotonicPool(benchmark::State &state) {
  run_mpmc_pool<SynchronizedMonotonicPoolAdapter<pool_block_size>>(state);
}

void BM_MPMC_MPMCMonotonicPool(benchmark::State &state) {
  run_mpmc_pool<MpmcResourceAdapter<pool_block_size>>(state);
}

constexpr std::size_t maximum_mpmc_thread_count = 8;

template <std::size_t BlockSize, typename Adapter>
void run_random_mpmc_pool(benchmark::State &state) {
  const auto thread_count = static_cast<std::size_t>(state.range(0));
  assert(thread_count >= 1 && thread_count <= maximum_mpmc_thread_count);

  Adapter adapter;
  prewarm<random_batch_size>(adapter);

  std::array<void *, random_batch_size> pointers;
  std::array<std::vector<std::size_t>, maximum_mpmc_thread_count>
      allocation_indices;
  std::array<std::vector<std::size_t>, maximum_mpmc_thread_count>
      deallocation_indices;
  std::uint64_t random_state = 0xd1b54a32d192ed03ULL;

  for (std::size_t index = 0; index < random_batch_size; ++index) {
    const std::uint64_t allocation_random = next_random(random_state);
    const std::size_t allocator = allocation_random % thread_count;
    allocation_indices[allocator].push_back(index);

    std::size_t deallocator = allocator;
    const std::uint64_t deallocation_random = next_random(random_state);
    if (thread_count > 1 && (deallocation_random & 3U) != 0) {
      const std::size_t offset =
          1 + ((deallocation_random >> 2U) % (thread_count - 1));
      deallocator = (allocator + offset) % thread_count;
    }
    deallocation_indices[deallocator].push_back(index);
  }

  std::barrier phase{static_cast<std::ptrdiff_t>(thread_count)};
  std::atomic<bool> stop{false};
  std::array<std::thread, maximum_mpmc_thread_count - 1> workers;

  for (std::size_t thread = 1; thread < thread_count; ++thread) {
    workers[thread - 1] = std::thread([&, thread] {
      while (true) {
        phase.arrive_and_wait();
        if (stop.load(std::memory_order_acquire)) {
          break;
        }

        for (std::size_t index : allocation_indices[thread]) {
          pointers[index] = adapter.allocate();
          assert(pointers[index] != nullptr);
        }

        phase.arrive_and_wait();

        for (std::size_t index : deallocation_indices[thread]) {
          assert(pointers[index] != nullptr);
          adapter.deallocate(pointers[index]);
        }

        phase.arrive_and_wait();
      }
    });
  }

  for (auto _ : state) {
    phase.arrive_and_wait();

    for (std::size_t index : allocation_indices[0]) {
      pointers[index] = adapter.allocate();
      assert(pointers[index] != nullptr);
    }

    phase.arrive_and_wait();

    for (std::size_t index : deallocation_indices[0]) {
      assert(pointers[index] != nullptr);
      adapter.deallocate(pointers[index]);
    }

    phase.arrive_and_wait();
  }

  stop.store(true, std::memory_order_release);
  phase.arrive_and_wait();
  for (std::size_t thread = 1; thread < thread_count; ++thread) {
    workers[thread - 1].join();
  }

  const auto operations =
      state.iterations() * static_cast<std::int64_t>(random_batch_size);
  state.SetItemsProcessed(operations);
  state.SetBytesProcessed(operations * static_cast<std::int64_t>(BlockSize));
}

template <std::size_t BlockSize>
void BM_MPMC_Random_Mallocator(benchmark::State &state) {
  run_random_mpmc_pool<BlockSize, MallocatorAdapter<BlockSize>>(state);
}

template <std::size_t BlockSize>
void BM_MPMC_Random_SyncResourceOfMonotonicPool(benchmark::State &state) {
  run_random_mpmc_pool<BlockSize, LockedMonotonicPoolAdapter<BlockSize>>(state);
}

template <std::size_t BlockSize>
void BM_MPMC_Random_SyncMonotonicPool(benchmark::State &state) {
  run_random_mpmc_pool<BlockSize,
                       SynchronizedMonotonicPoolAdapter<BlockSize>>(state);
}

template <std::size_t BlockSize>
void BM_MPMC_Random_MPMCMonotonicPool(benchmark::State &state) {
  run_random_mpmc_pool<BlockSize, MpmcResourceAdapter<BlockSize>>(state);
}

BENCHMARK(BM_Pool_Mallocator)->Unit(benchmark::kNanosecond);
BENCHMARK(BM_Pool_SyncResourceMonotonicPool)->Unit(benchmark::kNanosecond);
BENCHMARK(BM_Pool_SyncMonotonicPool)->Unit(benchmark::kNanosecond);
BENCHMARK(BM_Pool_MPSCMonotonicPool)->Unit(benchmark::kNanosecond);

#define STROBE_MPSC_BENCHMARK(function)                                      \
  BENCHMARK(function)                                                        \
      ->Threads(1)                                                           \
      ->Threads(2)                                                           \
      ->Threads(3)                                                           \
      ->Threads(4)                                                           \
      ->Threads(8)                                                           \
      ->UseRealTime()                                                        \
      ->Unit(benchmark::kNanosecond)

STROBE_MPSC_BENCHMARK(BM_MPSC_Mallocator);
STROBE_MPSC_BENCHMARK(BM_MPSC_MPSCMonotonicPool);
STROBE_MPSC_BENCHMARK(BM_MPSC_SyncResourceOfMonotonicPool);
STROBE_MPSC_BENCHMARK(BM_MPSC_SyncMonotonicPool);
STROBE_MPSC_BENCHMARK(BM_MPSC_MPMCMonotonicPool);

#undef STROBE_MPSC_BENCHMARK

#define STROBE_RANDOM_MPSC_BENCHMARK(function)                               \
  BENCHMARK_TEMPLATE(function, 16)                                           \
      ->DenseRange(1, 4)                                                     \
      ->ArgName("threads")                                                   \
      ->UseRealTime()                                                        \
      ->Unit(benchmark::kNanosecond);                                        \
  BENCHMARK_TEMPLATE(function, 64)                                           \
      ->DenseRange(1, 4)                                                     \
      ->ArgName("threads")                                                   \
      ->UseRealTime()                                                        \
      ->Unit(benchmark::kNanosecond);                                        \
  BENCHMARK_TEMPLATE(function, 256)                                          \
      ->DenseRange(1, 4)                                                     \
      ->ArgName("threads")                                                   \
      ->UseRealTime()                                                        \
      ->Unit(benchmark::kNanosecond)

STROBE_RANDOM_MPSC_BENCHMARK(BM_MPSC_Random_Mallocator);
STROBE_RANDOM_MPSC_BENCHMARK(BM_MPSC_Random_MPSCMonotonicPool);
STROBE_RANDOM_MPSC_BENCHMARK(BM_MPSC_Random_SyncResourceOfMonotonicPool);
STROBE_RANDOM_MPSC_BENCHMARK(BM_MPSC_Random_SyncMonotonicPool);
STROBE_RANDOM_MPSC_BENCHMARK(BM_MPSC_Random_MPMCMonotonicPool);

#undef STROBE_RANDOM_MPSC_BENCHMARK

#define STROBE_MPMC_BENCHMARK(function)                                      \
  BENCHMARK(function)                                                        \
      ->Threads(1)                                                           \
      ->Threads(2)                                                           \
      ->Threads(3)                                                           \
      ->Threads(4)                                                           \
      ->Threads(8)                                                           \
      ->UseRealTime()                                                        \
      ->Unit(benchmark::kNanosecond)

STROBE_MPMC_BENCHMARK(BM_MPMC_Mallocator);
STROBE_MPMC_BENCHMARK(BM_MPMC_SyncResourceOfMonotonicPool);
STROBE_MPMC_BENCHMARK(BM_MPMC_SyncMonotonicPool);
STROBE_MPMC_BENCHMARK(BM_MPMC_MPMCMonotonicPool);

#undef STROBE_MPMC_BENCHMARK

#define STROBE_RANDOM_MPMC_BENCHMARK(function)                               \
  BENCHMARK_TEMPLATE(function, 16)                                           \
      ->ArgsProduct({{1, 2, 3, 4, 8}})                                      \
      ->ArgName("threads")                                                   \
      ->UseRealTime()                                                        \
      ->Unit(benchmark::kNanosecond);                                        \
  BENCHMARK_TEMPLATE(function, 64)                                           \
      ->ArgsProduct({{1, 2, 3, 4, 8}})                                      \
      ->ArgName("threads")                                                   \
      ->UseRealTime()                                                        \
      ->Unit(benchmark::kNanosecond);                                        \
  BENCHMARK_TEMPLATE(function, 256)                                          \
      ->ArgsProduct({{1, 2, 3, 4, 8}})                                      \
      ->ArgName("threads")                                                   \
      ->UseRealTime()                                                        \
      ->Unit(benchmark::kNanosecond)

STROBE_RANDOM_MPMC_BENCHMARK(BM_MPMC_Random_Mallocator);
STROBE_RANDOM_MPMC_BENCHMARK(BM_MPMC_Random_SyncResourceOfMonotonicPool);
STROBE_RANDOM_MPMC_BENCHMARK(BM_MPMC_Random_SyncMonotonicPool);
STROBE_RANDOM_MPMC_BENCHMARK(BM_MPMC_Random_MPMCMonotonicPool);

#undef STROBE_RANDOM_MPMC_BENCHMARK

} // namespace
} // namespace strobe
