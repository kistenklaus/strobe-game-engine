#pragma once

#include "strobe/core/memory/allocator_traits.hpp"

#include <algorithm>
#include <atomic>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <ratio>
#include <tracy/Tracy.hpp>
#include <utility>

namespace strobe {

/**
 * \ingroup core
 * \brief Growing fixed-size pool with single-producer allocation and
 * multi-producer deallocation.
 * \code{.cpp}
 * template<std::size_t BlockSize, std::size_t BlockAlign, Allocator A,
 *          typename GrowthFactor = std::ratio<2, 1>>
 * class MPSCMonotonicPoolResource;
 * \endcode
 *
 * Allocation uses allocator-local freelist state and must be externally
 * synchronized. Deallocation publishes returned blocks atomically and may be
 * called concurrently by multiple threads, including while the allocation
 * thread is active.
 *
 * \tparam BlockSize Maximum requested allocation size in bytes.
 * \tparam BlockAlign Alignment supported by the pool.
 * \tparam A Upstream allocator used for chunks.
 * \tparam GrowthFactor Ratio used to grow chunks.
 *
 * \attention 1. \p BlockSize and \p BlockAlign must be nonzero, and
 * \p BlockAlign must be a power of two.
 * \attention 2. \p GrowthFactor must be positive and no less than one.
 * \attention 3. Allocation calls must be externally serialized.
 * \attention 4. Deallocation, release(), and destruction must not race with
 * destruction or release().
 */
template <std::size_t BlockSize, std::size_t BlockAlign, Allocator A,
          typename GrowthFactor = std::ratio<2, 1>>
class MPSCMonotonicPoolResource {
public:
  static_assert(BlockSize > 0);
  static_assert(BlockAlign > 0);
  static_assert(std::has_single_bit(BlockAlign));
  static_assert(GrowthFactor::num > 0);
  static_assert(GrowthFactor::den > 0);
  static_assert(GrowthFactor::num >= GrowthFactor::den);

  /**
   * \brief This resource's complete type.
   * \code{.cpp}
   * using Self = MPSCMonotonicPoolResource<BlockSize, BlockAlign, A, GrowthFactor>;
   * \endcode
   */
  using Self =
      MPSCMonotonicPoolResource<BlockSize, BlockAlign, A, GrowthFactor>;

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
   * \brief Maximum allocation size supported by the pool.
   * \code{.cpp}
   * static constexpr std::size_t block_size;
   * \endcode
   */
  static constexpr std::size_t block_size = std::max(
      std::max(BlockSize, BlockAlign), sizeof(void *) + sizeof(std::size_t));

  /**
   * \brief Alignment provided by the pool.
   * \code{.cpp}
   * static constexpr std::size_t block_align;
   * \endcode
   */
  static constexpr std::size_t block_align = BlockAlign;

  /**
   * \brief Constructs an empty pool.
   * \code{.cpp}
   * explicit MPSCMonotonicPoolResource(const A& upstream);
   * MPSCMonotonicPoolResource();
   * ~MPSCMonotonicPoolResource();
   * MPSCMonotonicPoolResource(const MPSCMonotonicPoolResource&) = delete;
   * MPSCMonotonicPoolResource& operator=(const MPSCMonotonicPoolResource&) = delete;
   * MPSCMonotonicPoolResource(MPSCMonotonicPoolResource&& other) noexcept;
   * MPSCMonotonicPoolResource& operator=(MPSCMonotonicPoolResource&& other) noexcept;
   * \endcode
   *
   * \param upstream Allocator used to obtain and release chunks.
   */
  explicit MPSCMonotonicPoolResource(const A &upstream)
      : m_upstream(upstream), m_buffer(nullptr), m_freelist(nullptr) {}

  MPSCMonotonicPoolResource()
    requires std::default_initializable<A>
      : MPSCMonotonicPoolResource(A{}) {}

  ~MPSCMonotonicPoolResource() { release(); }

  MPSCMonotonicPoolResource(const MPSCMonotonicPoolResource &) = delete;

  MPSCMonotonicPoolResource &
  operator=(const MPSCMonotonicPoolResource &) = delete;

  MPSCMonotonicPoolResource(MPSCMonotonicPoolResource &&o) noexcept
      : m_upstream(std::move(o.m_upstream)),
        m_buffer(std::exchange(o.m_buffer, nullptr)),
        m_freelist(std::exchange(o.m_freelist, nullptr)),
        m_returned(o.m_returned.exchange(nullptr, std::memory_order_relaxed)) {}

  MPSCMonotonicPoolResource &operator=(MPSCMonotonicPoolResource &&o) noexcept {
    if (this == &o) {
      return *this;
    }

    static_assert(
        upstream_traits::propagate_on_container_move_assignment,
        "Required for upstream allocators, because otherwise moving the "
        "resource could invalidate allocated pointers.");

    release();

    m_upstream = std::move(o.m_upstream);
    m_buffer = std::exchange(o.m_buffer, nullptr);
    m_freelist = std::exchange(o.m_freelist, nullptr);
    m_returned.store(o.m_returned.exchange(nullptr, std::memory_order_relaxed),
                     std::memory_order_relaxed);

    return *this;
  }

  /**
   * \brief Creates an independent resource using the same upstream allocator.
   * \code{.cpp}
   * MPSCMonotonicPoolResource select_on_container_copy_construction() const;
   * \endcode
   *
   * \return An empty resource with a copied upstream allocator.
   */
  MPSCMonotonicPoolResource select_on_container_copy_construction() const {
    return MPSCMonotonicPoolResource(m_upstream);
  }

  /**
   * \brief Allocates one block.
   * \code{.cpp}
   * [[nodiscard]] void* allocate(std::size_t size, std::size_t alignment);
   * [[nodiscard]] void* allocate();
   * \endcode
   *
   * \param size Requested size in bytes.
   * \param alignment Required alignment.
   * \return A block satisfying the request.
   * \attention 1. Allocation calls must be externally serialized.
   * \attention 2. \p size must be no greater than block_size.
   * \attention 3. \p alignment must be a nonzero divisor of block_align.
   */
  [[nodiscard]]
  void *allocate(std::size_t size, std::size_t alignment) {
    assert(size > 0);
    assert(size <= block_size);
    assert(alignment > 0);
    assert(alignment <= block_align);
    assert(std::has_single_bit(alignment));
    assert((block_align % alignment) == 0);
    (void)size;
    (void)alignment;

    if (m_freelist == nullptr) {
      // Atomically detach every concurrently returned node.
      m_freelist = m_returned.exchange(nullptr, std::memory_order_acquire);

      if (m_freelist == nullptr) {
        return reinterpret_cast<void *>(&allocate_block()->value);
      }
    }

    Node *node = m_freelist;
    m_freelist = node->free.next;

    return reinterpret_cast<void *>(&node->value);
  }

  [[nodiscard]] void *allocate() { return allocate(block_size, block_align); }

  /**
   * \brief Returns a block to the concurrent return queue.
   * \code{.cpp}
   * void deallocate(void* pointer, std::size_t size,
   *                 std::size_t alignment) noexcept;
   * void deallocate(void* pointer) noexcept;
   * \endcode
   *
   * \param pointer Block previously returned by allocate.
   * \param size Original requested size for the sized overload.
   * \param alignment Original alignment for the sized overload.
   * \attention 1. Deallocation may be called concurrently by multiple
   * threads.
   * \attention 2. \p pointer must belong to this pool and must not already
   * have been returned.
   */
  void deallocate(void *pointer, std::size_t size,
                  std::size_t alignment) noexcept {
    assert(pointer != nullptr);
    assert(size <= block_size);
    assert(size > 0);
    assert(alignment > 0);
    assert(alignment <= block_align);
    assert(std::has_single_bit(alignment));
    assert((block_align % alignment) == 0);
    (void)size;
    (void)alignment;

    deallocate(pointer);
  }

  void deallocate(void *pointer) noexcept {
    assert(pointer != nullptr);

    auto *node = reinterpret_cast<Node *>(pointer);

    Node *head = m_returned.load(std::memory_order_relaxed);

    do {
      node->free.next = head;
    } while (!m_returned.compare_exchange_weak(
        head, node, std::memory_order_release, std::memory_order_relaxed));
  }

  /**
   * \brief Compares resource identity.
   * \code{.cpp}
   * bool operator==(const MPSCMonotonicPoolResource& other) const noexcept;
   * \endcode
   *
   * \param other Resource to compare.
   * \return Whether both references denote the same resource object.
   */
  bool operator==(const MPSCMonotonicPoolResource &other) const noexcept {
    return this == &other;
  }

  /**
   * \brief Compares resource identity for inequality.
   * \code{.cpp}
   * bool operator!=(const MPSCMonotonicPoolResource& other) const noexcept;
   * \endcode
   *
   * \param other Resource to compare.
   * \return Whether the resources are different objects.
   */
  bool operator!=(const MPSCMonotonicPoolResource &other) const noexcept = default;

  /**
   * \brief Tests whether an address belongs to the pool.
   * \code{.cpp}
   * bool owns(const void* pointer) const noexcept;
   * \endcode
   *
   * \param pointer Address to test.
   * \return Whether the address lies within a pool chunk.
   * \attention 1. Do not call this while an allocation call may grow the pool.
   */
  bool owns(const void *pointer) const noexcept {
    if (pointer == nullptr) {
      return false;
    }
    const auto address = reinterpret_cast<std::uintptr_t>(pointer);

    Node *block = m_buffer;

    while (block != nullptr) {
      const std::size_t count = block->block.blockSize;

      const auto begin = reinterpret_cast<std::uintptr_t>(block);
      const auto end = reinterpret_cast<std::uintptr_t>(block + count);

      if (address >= begin && address < end) {
        return true;
      }

      block = block->block.next;
    }

    return false;
  }

private:
  union Node {
    struct {
      Node *next;
    } free;

    struct {
      Node *next;
      std::size_t blockSize;
    } block;

    alignas(block_align) std::byte value[block_size];
  };

  // No operation may race destruction/release.
  void release() noexcept {
    Node *block = std::exchange(m_buffer, nullptr);

    m_freelist = nullptr;
    m_returned.store(nullptr, std::memory_order_relaxed);

    while (block != nullptr) {
      Node *next = block->block.next;
      const std::size_t count = block->block.blockSize;

      upstream_traits::template deallocate<Node>(m_upstream, block, count);

      block = next;
    }
  }

  // Allocation/growth is externally synchronized.
  void push_block(Node *header) noexcept {
    header->block.next = m_buffer;
    m_buffer = header;
  }

  // Allocates a new chunk.
  //
  // Layout:
  //
  //   [header][unique][free][free][free]...
  //
  // `unique` is returned directly to the caller while the remaining nodes
  // are appended to the allocator-local freelist.
  Node *allocate_block() {
    ZoneScopedN("MPSCMonotonicPoolResource::allocate_block");
    std::size_t next_block_size;

    if (m_buffer == nullptr) {
      next_block_size = 2;
    } else {
      assert(m_buffer->block.blockSize <=
             std::numeric_limits<std::size_t>::max() /
                 GrowthFactor::num);
      next_block_size =
          (m_buffer->block.blockSize * GrowthFactor::num) / GrowthFactor::den;
    }

    // Need one header and at least one usable node.
    next_block_size = std::max<std::size_t>(next_block_size, 2);

    Node *block =
        upstream_traits::template allocate<Node>(m_upstream, next_block_size);
    assert(block != nullptr);

    Node *header = block;
    Node *unique = header + 1;

    Node *begin = header + 2;
    Node *end = header + next_block_size;

    // These nodes are allocation-thread-local, so there is no reason to
    // publish them through m_returned.
    if (begin < end) {
      for (Node *current = begin; current + 1 < end; ++current) {
        current->free.next = current + 1;
      }

      Node *last = end - 1;
      last->free.next = m_freelist;

      m_freelist = begin;
    }

    header->block.blockSize = next_block_size;

    push_block(header);

    return unique;
  }

private:
  [[no_unique_address]] upstream_allocator m_upstream;

  // Allocation-thread-only state.
  Node *m_buffer = nullptr;
  Node *m_freelist = nullptr;

  // Concurrent deallocations publish nodes here.
  std::atomic<Node *> m_returned{nullptr};
};

} // namespace strobe
