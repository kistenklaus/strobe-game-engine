#pragma once

#include <benchmark/benchmark.h>

#include <cstddef>
#include <cstdint>
#include <cstdlib>

#include "strobe/core/memory/mallocator.hpp"

namespace strobe {
namespace {

void BM_MallocatorAllocateDeallocate(benchmark::State &state) {
  Mallocator allocator;
  const auto size = static_cast<std::size_t>(state.range(0));

  for (auto _ : state) {
    void *allocation = allocator.allocate(size, alignof(std::max_align_t));
    benchmark::DoNotOptimize(allocation);
    allocator.deallocate(allocation, size, alignof(std::max_align_t));
  }

  state.SetItemsProcessed(state.iterations());
  state.SetBytesProcessed(state.iterations() *
                          static_cast<std::int64_t>(size));
}

void BM_MallocFree(benchmark::State &state) {
  const auto size = static_cast<std::size_t>(state.range(0));

  for (auto _ : state) {
    void *allocation = std::malloc(size);
    benchmark::DoNotOptimize(allocation);
    std::free(allocation);
  }

  state.SetItemsProcessed(state.iterations());
  state.SetBytesProcessed(state.iterations() *
                          static_cast<std::int64_t>(size));
}

void allocation_sizes(benchmark::internal::Benchmark *benchmark) {
  for (int size : {16, 64, 256, 1024, 4096, 16384}) {
    benchmark->Arg(size);
  }
}

BENCHMARK(BM_MallocatorAllocateDeallocate)
    ->Apply(allocation_sizes)
    ->ArgName("bytes")
    ->Unit(benchmark::kNanosecond);

BENCHMARK(BM_MallocFree)
    ->Apply(allocation_sizes)
    ->ArgName("bytes")
    ->Unit(benchmark::kNanosecond);

} // namespace
} // namespace strobe
