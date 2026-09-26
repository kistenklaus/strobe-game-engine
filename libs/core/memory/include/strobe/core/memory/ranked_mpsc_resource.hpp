#pragma once

#include "strobe/core/memory/allocator_traits.hpp"
#include "strobe/core/memory/mpsc_monotonic_pool_resource.hpp"

#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <ratio>
#include <tuple>
#include <utility>

namespace strobe {

/**
 * \ingroup core
 * \brief Routes variable-size allocations to fixed-size MPSC pools.
 * \code{.cpp}
 * template<Allocator Upstream, uint32_t MinRank = 4,
 *          uint32_t MaxRank = 10,
 *          typename GrowthFactor = std::ratio<2, 1>>
 * class RankedMPSCResource;
 * \endcode
 *
 * Requests are rounded up to the smallest power-of-two rank satisfying both
 * their size and alignment. Requests above the largest configured rank are
 * forwarded to the upstream allocator.
 *
 * \tparam Upstream Allocator used by the rank pools and oversized requests.
 * \tparam MinRank Smallest managed power-of-two exponent.
 * \tparam MaxRank Largest managed power-of-two exponent.
 * \tparam GrowthFactor Growth ratio used by each rank pool.
 *
 * \attention 1. Allocation calls must be externally serialized.
 * \attention 2. Deallocation may be called concurrently by multiple threads.
 */
template <Allocator Upstream, uint32_t MinRank = 4, uint32_t MaxRank = 10,
          typename GrowthFactor = std::ratio<2, 1>>
class RankedMPSCResource {
public:
  static_assert(MaxRank < std::numeric_limits<std::size_t>::digits);

  /**
   * \brief Type of the upstream allocator.
   * \code{.cpp}
   * using upstream_allocator = Upstream;
   * \endcode
   */
  using upstream_allocator = Upstream;

  /**
   * \brief Traits used for oversized allocations.
   * \code{.cpp}
   * using upstream_traits = AllocatorTraits<upstream_allocator>;
   * \endcode
   */
  using upstream_traits = AllocatorTraits<upstream_allocator>;

  static_assert(MinRank <= MaxRank);
  static_assert(GrowthFactor::num > GrowthFactor::den);

  /**
   * \brief Smallest managed rank.
   * \code{.cpp}
   * static constexpr uint32_t min_rank;
   * \endcode
   */
  static constexpr uint32_t min_rank = MinRank;

  /**
   * \brief Largest managed rank.
   * \code{.cpp}
   * static constexpr uint32_t max_rank;
   * \endcode
   */
  static constexpr uint32_t max_rank = MaxRank;

  /**
   * \brief Smallest managed allocation size.
   * \code{.cpp}
   * static constexpr std::size_t min_size;
   * \endcode
   */
  static constexpr std::size_t min_size = std::size_t{1} << MinRank;

  /**
   * \brief Largest managed allocation size.
   * \code{.cpp}
   * static constexpr std::size_t max_size;
   * \endcode
   */
  static constexpr std::size_t max_size = std::size_t{1} << MaxRank;

  /**
   * \brief Number of managed rank pools.
   * \code{.cpp}
   * static constexpr std::size_t pool_count;
   * \endcode
   */
  static constexpr std::size_t pool_count = MaxRank - MinRank + 1;

  /**
   * \brief Constructs all rank pools.
   * \code{.cpp}
   * explicit RankedMPSCResource(const Upstream& upstream);
   * RankedMPSCResource(const RankedMPSCResource&) = delete;
   * RankedMPSCResource& operator=(const RankedMPSCResource&) = delete;
   * RankedMPSCResource(RankedMPSCResource&& other) = default;
   * RankedMPSCResource& operator=(RankedMPSCResource&& other) = default;
   * \endcode
   *
   * \param upstream Allocator used by all rank pools.
   */
  explicit RankedMPSCResource(const Upstream &upstream)
      : m_upstream(upstream),
        m_pools(make_pools(upstream, std::make_index_sequence<pool_count>{})) {}

  RankedMPSCResource(const RankedMPSCResource &) = delete;
  RankedMPSCResource &operator=(const RankedMPSCResource &) = delete;

  RankedMPSCResource(RankedMPSCResource &&) = default;
  RankedMPSCResource &operator=(RankedMPSCResource &&) = default;

  /**
   * \brief Allocates storage routed to the appropriate rank pool.
   * \code{.cpp}
   * [[nodiscard]] void* allocate(std::size_t size, std::size_t align);
   * \endcode
   *
   * \param size Number of bytes requested.
   * \param align Required power-of-two alignment.
   * \return Storage satisfying both \p size and \p align.
   *
   * \attention 1. \p size and \p align must be nonzero.
   * \attention 2. \p align must be a power of two.
   * \attention 3. Allocation calls must be externally serialized.
   */
  [[nodiscard]] void *allocate(std::size_t size, std::size_t align) {
    return allocate_at_least(size, align).first;
  }

  /**
   * \brief Allocates storage and reports the size provided.
   * \code{.cpp}
   * [[nodiscard]] std::pair<void*, std::size_t>
   * allocate_at_least(std::size_t size, std::size_t align);
   * \endcode
   *
   * \param size Minimum number of bytes requested.
   * \param align Required power-of-two alignment.
   * \return A pointer and the number of bytes available at that pointer.
   *
   * \attention 1. \p size and \p align must be nonzero.
   * \attention 2. \p align must be a power of two.
   * \attention 3. Allocation calls must be externally serialized.
   */
  [[nodiscard]] std::pair<void *, std::size_t>
  allocate_at_least(std::size_t size, std::size_t align) {
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

  /**
   * \brief Releases storage previously allocated by this resource.
   * \code{.cpp}
   * void deallocate(void* ptr, std::size_t size, std::size_t align) noexcept;
   * \endcode
   *
   * \param ptr Pointer returned by allocate() or allocate_at_least().
   * \param size Size originally requested for the allocation.
   * \param align Alignment originally requested for the allocation.
   *
   * \attention 1. The arguments must describe an allocation returned by this
   * resource and must be passed unchanged.
   * \attention 2. Deallocation may be called concurrently by multiple threads.
   */
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
  using Pool = MPSCMonotonicPoolResource<std::size_t{1} << (MinRank + I),
                                         std::size_t{1} << (MinRank + I),
                                         Upstream, GrowthFactor>;

  template <std::size_t... I> using PoolsImpl = std::tuple<Pool<I>...>;

  template <std::size_t... I>
  static PoolsImpl<I...> make_pools(const Upstream &upstream,
                                    std::index_sequence<I...>) {
    return PoolsImpl<I...>{Pool<I>{upstream}...};
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
