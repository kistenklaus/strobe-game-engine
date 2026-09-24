#include <benchmark/benchmark.h>

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

#include "strobe/core/containers/hash_map.hpp"
#include "strobe/core/containers/linear_map.hpp"
#include "strobe/core/containers/swizz_hash_map.hpp"
#include "strobe/core/memory/Mallocator.hpp"

namespace strobe {
namespace {

using Key = std::string;
using Value = std::uint32_t;

using Linear = LinearMap<Key, Value>;
using Hashed = HashMap<Key, Value>;
using Swizz = SwizzHashMap<Key, Value>;
using Standard = std::unordered_map<Key, Value>;

template <typename Map> bool contains(const Map &map, const Key &key) {
  if constexpr (requires { map.containsKey(key); })
    return map.containsKey(key);
  else
    return map.contains(key);
}

template <typename Map> void insert(Map &map, const Key &key, Value value) {
  if constexpr (std::same_as<Map, Standard>)
    map.emplace(key, value);
  else
    map.insert(key, value);
}

// Produces distinct, deterministic strings of exactly `length` characters.
Key make_key(std::uint32_t id, std::size_t length) {
  Key key(length, 'a');
  std::uint32_t state = id ^ 0x9e3779b9u;

  for (std::size_t i = 0; i < length; ++i) {
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    key[i] = static_cast<char>('a' + (state % 26));
  }

  // Guarantee uniqueness for the tested ID range.
  for (std::size_t i = 0; i < 4; ++i) {
    key[length - 1 - i] = static_cast<char>('a' + (id % 26));
    id /= 26;
  }

  return key;
}

std::vector<Key> make_keys(std::size_t count, std::size_t length,
                           std::uint32_t firstId = 0) {
  std::vector<Key> keys;
  keys.reserve(count);

  for (std::size_t i = 0; i < count; ++i)
    keys.push_back(make_key(firstId + static_cast<std::uint32_t>(i), length));

  return keys;
}

std::vector<Key> make_queries(const std::vector<Key> &source) {
  std::vector<Key> queries;
  queries.reserve(4096);

  for (std::size_t i = 0; i < 4096; ++i)
    queries.push_back(source[i % source.size()]);

  std::mt19937 rng(0x5A17u);
  std::shuffle(queries.begin(), queries.end(), rng);
  return queries;
}

template <typename Map, bool Hit>
void BM_StringLookup(benchmark::State &state) {
  const auto count = static_cast<std::size_t>(state.range(0));
  const auto length = static_cast<std::size_t>(state.range(1));

  const auto keys = make_keys(count, length);
  const auto queryKeys =
      Hit ? keys : make_keys(count, length, static_cast<std::uint32_t>(count));
  const auto queries = make_queries(queryKeys);

  Map map;
  map.reserve(count);

  for (std::size_t i = 0; i < count; ++i)
    insert(map, keys[i], static_cast<Value>(i));

  std::size_t query = 0;

  for (auto _ : state) {
    const Key &key = queries[query++ & (queries.size() - 1)];
    benchmark::DoNotOptimize(contains(map, key));
  }

  state.SetItemsProcessed(state.iterations());
}

template <typename Map> void BM_StringReservedInsert(benchmark::State &state) {
  const auto count = static_cast<std::size_t>(state.range(0));
  const auto length = static_cast<std::size_t>(state.range(1));
  const auto keys = make_keys(count, length);

  Map map;
  map.reserve(count);

  for (auto _ : state) {
    state.PauseTiming();
    map.clear();
    state.ResumeTiming();

    for (std::size_t i = 0; i < count; ++i)
      insert(map, keys[i], static_cast<Value>(i));

    benchmark::DoNotOptimize(map.size());
  }

  state.SetItemsProcessed(state.iterations() *
                          static_cast<std::int64_t>(count));
}

void string_key_sizes(benchmark::internal::Benchmark *bench) {
  for (int count : {4, 16, 64, 256, 1024})
    for (int length : {8, 24, 64, 256})
      bench->Args({count, length});
}

#define STROBE_STRING_BENCHMARK_ARGS                                           \
  Apply(string_key_sizes)                                                      \
      ->ArgNames({"count", "key_length"})                                      \
      ->Unit(benchmark::kNanosecond)

// BENCHMARK_TEMPLATE(BM_StringLookup, Linear, true)
//     ->STROBE_STRING_BENCHMARK_ARGS;
// BENCHMARK_TEMPLATE(BM_StringLookup, Hashed, true)
//     ->STROBE_STRING_BENCHMARK_ARGS;
BENCHMARK_TEMPLATE(BM_StringLookup, Swizz, true)->STROBE_STRING_BENCHMARK_ARGS;
// BENCHMARK_TEMPLATE(BM_StringLookup, Standard, true)
//     ->STROBE_STRING_BENCHMARK_ARGS;

// BENCHMARK_TEMPLATE(BM_StringLookup, Linear, false)
//     ->STROBE_STRING_BENCHMARK_ARGS;
// BENCHMARK_TEMPLATE(BM_StringLookup, Hashed, false)
//     ->STROBE_STRING_BENCHMARK_ARGS;
BENCHMARK_TEMPLATE(BM_StringLookup, Swizz, false)->STROBE_STRING_BENCHMARK_ARGS;
// BENCHMARK_TEMPLATE(BM_StringLookup, Standard, false)
//     ->STROBE_STRING_BENCHMARK_ARGS;

// BENCHMARK_TEMPLATE(BM_StringReservedInsert, Linear)
//     ->STROBE_STRING_BENCHMARK_ARGS;
// BENCHMARK_TEMPLATE(BM_StringReservedInsert, Hashed)
//     ->STROBE_STRING_BENCHMARK_ARGS;
BENCHMARK_TEMPLATE(BM_StringReservedInsert, Swizz)
    ->STROBE_STRING_BENCHMARK_ARGS;
// BENCHMARK_TEMPLATE(BM_StringReservedInsert, Standard)
//     ->STROBE_STRING_BENCHMARK_ARGS;

#undef STROBE_STRING_BENCHMARK_ARGS

} // namespace
} // namespace strobe
