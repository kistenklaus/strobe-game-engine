#pragma once

#include "strobe/core/memory/AllocatorTraits.hpp"
#include "strobe/core/memory/sync_monotonic_pool_resource.hpp"

#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <ratio>
#include <tuple>
#include <utility>

namespace strobe {

template <Allocator Upstream, uint32_t MinRank = 4, uint32_t MaxRank = 10,
          typename GrowthFactor = std::ratio<2, 1>>
class RankedSyncResource {
public:
  using upstream_allocator = Upstream;
  using upstream_traits = AllocatorTraits<upstream_allocator>;

  static_assert(MinRank <= MaxRank);
  static_assert(GrowthFactor::num > GrowthFactor::den);

  static constexpr uint32_t min_rank = MinRank;
  static constexpr uint32_t max_rank = MaxRank;

  static constexpr std::size_t min_size = std::size_t{1} << MinRank;

  static constexpr std::size_t max_size = std::size_t{1} << MaxRank;

  static constexpr std::size_t pool_count = MaxRank - MinRank + 1;

  explicit RankedSyncResource(const Upstream &upstream)
      : m_upstream(upstream),
        m_pools(make_pools(upstream, std::make_index_sequence<pool_count>{})) {}

  RankedSyncResource(const RankedSyncResource &) = delete;
  RankedSyncResource &operator=(const RankedSyncResource &) = delete;

  RankedSyncResource(RankedSyncResource &&) = delete;
  RankedSyncResource &operator=(RankedSyncResource &&) = delete;

  void *allocate(std::size_t size, std::size_t align) {
    return allocate_at_least(size, align).first;
  }

  std::pair<void *, std::size_t> allocate_at_least(std::size_t size,
                                                   std::size_t align) {
    assert(size != 0);
    assert(align != 0);
    assert(std::has_single_bit(align));

    const std::size_t rank = rank_for(size, align);

    if (rank > MaxRank) {
      return upstream_traits::allocate_at_least(m_upstream, size, align);
    }

    const std::size_t index = rank - MinRank;

    assert(index < pool_count);

    return {
        s_allocate[index](m_pools),
        std::size_t{1} << rank,
    };
  }

  void deallocate(void *ptr, std::size_t size, std::size_t align) noexcept {
    assert(ptr != nullptr);
    assert(size != 0);
    assert(align != 0);
    assert(std::has_single_bit(align));

    const std::size_t rank = rank_for(size, align);

    if (rank > MaxRank) {
      upstream_traits::deallocate(m_upstream, ptr, size, align);
      return;
    }

    const std::size_t index = rank - MinRank;

    assert(index < pool_count);

    s_deallocate[index](m_pools, ptr);
  }

private:
  template <std::size_t I>
  using Pool = SyncMonotonicPoolResource<std::size_t{1} << (MinRank + I),
                                         std::size_t{1} << (MinRank + I),
                                         Upstream, GrowthFactor>;

  template <std::size_t... I> using PoolsImpl = std::tuple<Pool<I>...>;

  template <std::size_t... I>
  static PoolsImpl<I...> make_pools(const Upstream &upstream,
                                    std::index_sequence<I...>) {
    return PoolsImpl<I...>{(static_cast<void>(I), upstream)...};
  }

  using Pools = decltype(make_pools(std::declval<const Upstream &>(),
                                    std::make_index_sequence<pool_count>{}));

private:
  static constexpr std::size_t rank_for(std::size_t size,
                                        std::size_t align) noexcept {
    const std::size_t required = size > align ? size : align;

    if (required <= min_size) {
      return MinRank;
    }

    // ceil(log2(required))
    //
    // 32 -> 5
    // 33 -> 6
    // 63 -> 6
    // 64 -> 6
    return std::bit_width(required - 1);
  }

  template <std::size_t I> static void *allocate_from(Pools &pools) {
    return std::get<I>(pools).allocate();
  }

  template <std::size_t I>
  static void deallocate_to(Pools &pools, void *ptr) noexcept {
    std::get<I>(pools).deallocate(ptr);
  }

  using AllocateFn = void *(*)(Pools &);

  using DeallocateFn = void (*)(Pools &, void *) noexcept;

  template <std::size_t... I>
  static consteval auto make_allocate_table(std::index_sequence<I...>) {
    return std::array<AllocateFn, pool_count>{&allocate_from<I>...};
  }

  template <std::size_t... I>
  static consteval auto make_deallocate_table(std::index_sequence<I...>) {
    return std::array<DeallocateFn, pool_count>{&deallocate_to<I>...};
  }

  inline static constexpr auto s_allocate =
      make_allocate_table(std::make_index_sequence<pool_count>{});

  inline static constexpr auto s_deallocate =
      make_deallocate_table(std::make_index_sequence<pool_count>{});

private:
  [[no_unique_address]] Upstream m_upstream;
  Pools m_pools;
};

} // namespace strobe
