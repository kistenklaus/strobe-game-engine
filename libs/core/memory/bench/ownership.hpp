#pragma once

#include <benchmark/benchmark.h>

#include <cstdint>
#include <memory>

#include "strobe/core/memory/arc.hpp"
#include "strobe/core/memory/box.hpp"
#include "strobe/core/memory/mallocator.hpp"
#include "strobe/core/memory/rc.hpp"

namespace strobe {
namespace {

void BM_MakeBox(benchmark::State &state) {
  for (auto _ : state) {
    auto value = make_box<std::uint64_t>(Mallocator{}, 42);
    benchmark::DoNotOptimize(value.get());
  }

  state.SetItemsProcessed(state.iterations());
}

void BM_MakeUnique(benchmark::State &state) {
  for (auto _ : state) {
    auto value = std::make_unique<std::uint64_t>(42);
    benchmark::DoNotOptimize(value.get());
  }

  state.SetItemsProcessed(state.iterations());
}

void BM_MakeRc(benchmark::State &state) {
  for (auto _ : state) {
    auto value = make_rc<std::uint64_t>(42);
    benchmark::DoNotOptimize(value.get());
  }

  state.SetItemsProcessed(state.iterations());
}

void BM_MakeArc(benchmark::State &state) {
  for (auto _ : state) {
    auto value = make_arc<std::uint64_t>(42);
    benchmark::DoNotOptimize(value.get());
  }

  state.SetItemsProcessed(state.iterations());
}

void BM_MakeShared(benchmark::State &state) {
  for (auto _ : state) {
    auto value = std::make_shared<std::uint64_t>(42);
    benchmark::DoNotOptimize(value.get());
  }

  state.SetItemsProcessed(state.iterations());
}

void BM_RcCopy(benchmark::State &state) {
  const auto source = make_rc<std::uint64_t>(42);

  for (auto _ : state) {
    auto copy = source;
    benchmark::DoNotOptimize(copy.get());
    benchmark::ClobberMemory();
  }

  state.SetItemsProcessed(state.iterations());
}

void BM_ArcCopy(benchmark::State &state) {
  const auto source = make_arc<std::uint64_t>(42);

  for (auto _ : state) {
    auto copy = source;
    benchmark::DoNotOptimize(copy.get());
    benchmark::ClobberMemory();
  }

  state.SetItemsProcessed(state.iterations());
}

void BM_SharedPtrCopy(benchmark::State &state) {
  const auto source = std::make_shared<std::uint64_t>(42);

  for (auto _ : state) {
    auto copy = source;
    benchmark::DoNotOptimize(copy.get());
    benchmark::ClobberMemory();
  }

  state.SetItemsProcessed(state.iterations());
}

BENCHMARK(BM_MakeBox)->Unit(benchmark::kNanosecond);
BENCHMARK(BM_MakeUnique)->Unit(benchmark::kNanosecond);
BENCHMARK(BM_MakeRc)->Unit(benchmark::kNanosecond);
BENCHMARK(BM_MakeArc)->Unit(benchmark::kNanosecond);
BENCHMARK(BM_MakeShared)->Unit(benchmark::kNanosecond);
BENCHMARK(BM_RcCopy)->Unit(benchmark::kNanosecond);
BENCHMARK(BM_ArcCopy)->Unit(benchmark::kNanosecond);
BENCHMARK(BM_SharedPtrCopy)->Unit(benchmark::kNanosecond);

} // namespace
} // namespace strobe
