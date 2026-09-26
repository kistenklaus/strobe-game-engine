#pragma once

#include "strobe/core/memory/allocator_traits.hpp"
#include "strobe/core/memory/mpsc_monotonic_pool_resource.hpp"

#include <cstddef>
#include <mutex>
#include <ratio>
#include <type_traits>

namespace strobe {

/**
 * \ingroup core
 * \brief Mutex-serialized allocation wrapper around an MPSC pool.
 * \code{.cpp}
 * template<std::size_t BlockSize, std::size_t BlockAlign, Allocator A,
 *          typename GrowthFactor = std::ratio<2, 1>>
 * class SyncMonotonicPoolResource;
 * \endcode
 *
 * Allocation calls are serialized by an internal mutex. Deallocation uses the
 * underlying MPSC pool's concurrent return path and may run concurrently with
 * allocation or with other deallocations.
 *
 * \tparam BlockSize Maximum allocation size supported by the pool.
 * \tparam BlockAlign Alignment supported by the pool.
 * \tparam A Upstream allocator used for pool chunks.
 * \tparam GrowthFactor Ratio used to grow chunks.
 *
 * \attention 1. Allocation, deallocation, and destruction must not race with
 * destruction or any operation that invalidates the resource.
 */
template <std::size_t BlockSize, std::size_t BlockAlign, Allocator A,
          typename GrowthFactor = std::ratio<2, 1>>
class SyncMonotonicPoolResource {
public:
  /**
   * \brief Type of the upstream allocator.
   * \code{.cpp}
   * using upstream_allocator = A;
   * \endcode
   */
  using upstream_allocator = A;

  /**
   * \brief Traits used to manage upstream chunks.
   * \code{.cpp}
   * using upstream_traits = AllocatorTraits<upstream_allocator>;
   * \endcode
   */
  using upstream_traits = AllocatorTraits<upstream_allocator>;

  /**
   * \brief Type of the underlying MPSC pool.
   * \code{.cpp}
   * using mpsc_pool = MPSCMonotonicPoolResource<BlockSize, BlockAlign, A,
   *                                             GrowthFactor>;
   * \endcode
   */
  using mpsc_pool =
      MPSCMonotonicPoolResource<BlockSize, BlockAlign, A, GrowthFactor>;

  /**
   * \brief Constructs an empty synchronized pool.
   * \code{.cpp}
   * explicit SyncMonotonicPoolResource(const A& upstream);
   * SyncMonotonicPoolResource()
   *   requires std::default_initializable<A>;
   * ~SyncMonotonicPoolResource();
   * SyncMonotonicPoolResource(const SyncMonotonicPoolResource&) = delete;
   * SyncMonotonicPoolResource& operator=(const SyncMonotonicPoolResource&) = delete;
   * SyncMonotonicPoolResource(SyncMonotonicPoolResource&&) = delete;
   * SyncMonotonicPoolResource& operator=(SyncMonotonicPoolResource&&) = delete;
   * \endcode
   *
   * \param upstream Allocator used to obtain and release pool chunks.
   */
  explicit SyncMonotonicPoolResource(const A &upstream)
      : m_upstream(upstream), m_mpsc(m_upstream) {}

  SyncMonotonicPoolResource()
    requires std::default_initializable<A>
      : SyncMonotonicPoolResource(A{}) {}

  ~SyncMonotonicPoolResource() = default;

  SyncMonotonicPoolResource(const SyncMonotonicPoolResource &) = delete;
  SyncMonotonicPoolResource &operator=(const SyncMonotonicPoolResource &) =
      delete;
  SyncMonotonicPoolResource(SyncMonotonicPoolResource &&) = delete;
  SyncMonotonicPoolResource &operator=(SyncMonotonicPoolResource &&) = delete;

  /**
   * \brief Allocates storage from the synchronized pool.
   * \code{.cpp}
   * [[nodiscard]] void* allocate(std::size_t size, std::size_t align);
   * \endcode
   *
   * \param size Number of bytes requested.
   * \param align Required alignment.
   * \return Pointer to storage satisfying the request.
   *
   * \attention 1. Allocation calls are serialized internally.
   */
  [[nodiscard]] void *allocate(std::size_t size, std::size_t align) {
    std::lock_guard lock{m_mutex};
    return m_mpsc.allocate(size, align);
  }

  /**
   * \brief Allocates one pool block.
   * \code{.cpp}
   * [[nodiscard]] void* allocate();
   * \endcode
   *
   * \return Pointer to one block with the configured size and alignment.
   */
  [[nodiscard]] void *allocate() {
    std::lock_guard lock{m_mutex};
    return m_mpsc.allocate();
  }

  /**
   * \brief Returns storage to the pool.
   * \code{.cpp}
   * void deallocate(void* pointer, std::size_t size,
   *                 std::size_t align) noexcept;
   * void deallocate(void* pointer) noexcept;
   * \endcode
   *
   * \param pointer Storage previously returned by allocate().
   * \param size Size originally requested, when supplied.
   * \param align Alignment originally requested, when supplied.
   *
   * Deallocation is safe concurrently with allocation and other
   * deallocations, and does not acquire the allocation mutex.
   */
  void deallocate(void *pointer, std::size_t size,
                  std::size_t align) noexcept {
    m_mpsc.deallocate(pointer, size, align);
  }

  void deallocate(void *pointer) noexcept { m_mpsc.deallocate(pointer); }

  /**
   * \brief Creates an empty resource using the same upstream allocator.
   * \code{.cpp}
   * SyncMonotonicPoolResource select_on_container_copy_construction() const;
   * \endcode
   *
   * \return A distinct empty resource with a copy of the upstream allocator.
   */
  SyncMonotonicPoolResource select_on_container_copy_construction() const {
    return SyncMonotonicPoolResource(m_upstream);
  }

  /**
   * \brief Compares resource identity.
   * \code{.cpp}
   * bool operator==(const SyncMonotonicPoolResource& other) const noexcept;
   * \endcode
   *
   * \param other Resource to compare with.
   * \return Whether both expressions refer to the same resource.
   */
  bool operator==(const SyncMonotonicPoolResource &other) const noexcept {
    return this == &other;
  }

  /**
   * \brief Compares resource identity for inequality.
   * \code{.cpp}
   * bool operator!=(const SyncMonotonicPoolResource& other) const noexcept;
   * \endcode
   *
   * \param other Resource to compare with.
   * \return Whether the resources are distinct.
   */
  bool operator!=(const SyncMonotonicPoolResource &other) const noexcept =
      default;

private:
  [[no_unique_address]] A m_upstream;
  mutable std::mutex m_mutex;
  mpsc_pool m_mpsc;
};

} // namespace strobe
