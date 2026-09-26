#pragma once

#include "strobe/core/memory/allocator_traits.hpp"
#include "strobe/core/memory/sync_monotonic_pool_resource.hpp"

#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <mutex>
#include <ratio>
#include <tuple>
#include <utility>

namespace strobe {

/**
 * \ingroup core
 * \brief Routes allocations to independently synchronized rank pools.
 * \code{.cpp}
 * template<Allocator Upstream, uint32_t MinRank = 4,
 *          uint32_t MaxRank = 10,
 *          typename GrowthFactor = std::ratio<2, 1>>
 * class RankedSyncResource;
 * \endcode
 *
 * Requests are rounded up to the smallest power-of-two rank satisfying both
 * their size and alignment. Each rank has its own synchronized pool. Requests
 * above the largest configured rank are forwarded through the upstream
 * allocator under a separate mutex.
 *
 * \tparam Upstream Allocator used by the rank pools and oversized requests.
 * \tparam MinRank Smallest managed power-of-two exponent.
 * \tparam MaxRank Largest managed power-of-two exponent.
 * \tparam GrowthFactor Ratio used to grow each rank pool.
 *
 * \attention 1. Allocation and deallocation may run concurrently.
 * \attention 2. Destruction must not race with any operation.
 */
template <Allocator Upstream, uint32_t MinRank = 4, uint32_t MaxRank = 10,
          typename GrowthFactor = std::ratio<2, 1>>
class RankedSyncResource {
public:
  static_assert(MaxRank < std::numeric_limits<std::size_t>::digits);
  static_assert(MinRank <= MaxRank);
  static_assert(GrowthFactor::num > GrowthFactor::den);

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
   * explicit RankedSyncResource(const Upstream& upstream);
   * RankedSyncResource(const RankedSyncResource&) = delete;
   * RankedSyncResource& operator=(const RankedSyncResource&) = delete;
   * RankedSyncResource(RankedSyncResource&&) = delete;
   * RankedSyncResource& operator=(RankedSyncResource&&) = delete;
   * \endcode
   *
   * \param upstream Allocator used by all rank pools and oversized requests.
   */
  explicit RankedSyncResource(const Upstream &upstream)
      : m_upstream(upstream),
        m_pools(make_pools(upstream, std::make_index_sequence<pool_count>{})) {}

  RankedSyncResource(const RankedSyncResource &) = delete;
  RankedSyncResource &operator=(const RankedSyncResource &) = delete;
  RankedSyncResource(RankedSyncResource &&) = delete;
  RankedSyncResource &operator=(RankedSyncResource &&) = delete;

  /**
   * \brief Allocates storage from the appropriate rank pool.
   * \code{.cpp}
   * [[nodiscard]] void* allocate(std::size_t size, std::size_t align);
   * \endcode
   *
   * \param size Number of bytes requested.
   * \param align Required power-of-two alignment.
   * \return Storage satisfying both request parameters.
   * \attention 1. \p size and \p align must be nonzero.
   * \attention 2. \p align must be a power of two.
   */
  [[nodiscard]] void *allocate(std::size_t size, std::size_t align) {
    return allocate_at_least(size, align).first;
  }

  /**
   * \brief Allocates storage and reports the available size.
   * \code{.cpp}
   * [[nodiscard]] std::pair<void*, std::size_t>
   * allocate_at_least(std::size_t size, std::size_t align);
   * \endcode
   *
   * \param size Minimum number of bytes requested.
   * \param align Required power-of-two alignment.
   * \return Pointer and number of bytes available.
   * \attention 1. \p size and \p align must be nonzero.
   * \attention 2. \p align must be a power of two.
   */
  [[nodiscard]] std::pair<void *, std::size_t>
  allocate_at_least(std::size_t size, std::size_t align) {
    assert(size != 0);
    assert(align != 0);
    assert(std::has_single_bit(align));

    const std::size_t rank = rank_for(size, align);
    if (rank > MaxRank) {
      std::lock_guard lock{m_upstream_mutex};
      return upstream_traits::allocate_at_least(m_upstream, size, align);
    }

    const std::size_t index = rank - MinRank;
    assert(index < pool_count);
    return {s_allocate[index](m_pools), std::size_t{1} << rank};
  }

  /**
   * \brief Releases storage previously allocated by this resource.
   * \code{.cpp}
   * void deallocate(void* pointer, std::size_t size,
   *                 std::size_t align) noexcept;
   * \endcode
   *
   * \param pointer Storage returned by allocate() or allocate_at_least().
   * \param size Size originally requested.
   * \param align Alignment originally requested.
   * \attention 1. The arguments must describe the original allocation.
   */
  void deallocate(void *pointer, std::size_t size,
                  std::size_t align) noexcept {
    assert(pointer != nullptr);
    assert(size != 0);
    assert(align != 0);
    assert(std::has_single_bit(align));

    const std::size_t rank = rank_for(size, align);
    if (rank > MaxRank) {
      std::lock_guard lock{m_upstream_mutex};
      upstream_traits::deallocate(m_upstream, pointer, size, align);
      return;
    }

    const std::size_t index = rank - MinRank;
    assert(index < pool_count);
    s_deallocate[index](m_pools, pointer);
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

  static constexpr std::size_t rank_for(std::size_t size,
                                        std::size_t align) noexcept {
    const std::size_t required = size > align ? size : align;
    if (required <= min_size) {
      return MinRank;
    }
    return std::bit_width(required - 1);
  }

  template <std::size_t I> static void *allocate_from(Pools &pools) {
    return std::get<I>(pools).allocate();
  }

  template <std::size_t I>
  static void deallocate_to(Pools &pools, void *pointer) noexcept {
    std::get<I>(pools).deallocate(pointer);
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

  [[no_unique_address]] Upstream m_upstream;
  mutable std::mutex m_upstream_mutex;
  Pools m_pools;
};

} // namespace strobe
