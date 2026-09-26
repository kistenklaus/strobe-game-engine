#pragma once

#include "strobe/core/memory/allocator_traits.hpp"
#include <atomic>
#include <bit>
#include <cassert>
#include <cstdint>
#include <memory>
#include <new>

namespace strobe {

/**
 * \ingroup core
 * \brief Concurrent fixed-size node pool with monotonic chunk growth.
 * \code{.cpp}
 * template<typename T, Allocator A, std::size_t ChunkSize = 64>
 * class MPMCMonotonicPoolResource;
 * \endcode
 *
 * The pool allocates chunks from the upstream allocator and reuses individual
 * nodes after deallocation. Chunks remain owned by the pool until destruction.
 * Allocation and deallocation may be called concurrently from multiple
 * threads.
 *
 * \tparam T Object type whose storage each node provides.
 * \tparam A Upstream allocator used for chunks.
 * \tparam ChunkSize Number of nodes in each chunk.
 *
 * \attention 1. \p ChunkSize must be between 1 and 64.
 * \attention 2. The upstream allocator must support concurrent allocation
 * calls when multiple threads grow the pool and must remain valid until
 * destruction.
 * \attention 3. Node storage is raw storage; callers must construct and
 * destroy \p T objects themselves.
 */
template <typename T, Allocator A, std::size_t ChunkSize = 64>
class MPMCMonotonicPoolResource {
  static_assert(ChunkSize > 0 && ChunkSize <= 64);

  struct Chunk;

public:
  /**
   * \brief Represents one reusable node in the pool.
   * \code{.cpp}
   * struct Node;
   * \endcode
   */
  struct Node {
    /**
     * \brief Pool-owned chunk metadata.
     * \code{.cpp}
     * Chunk* owner;
     * \endcode
     *
     * \attention 1. Callers must not modify this member.
     */
    Chunk *owner;

    /**
     * \brief Pool-owned linkage metadata.
     * \code{.cpp}
     * Node* next;
     * \endcode
     *
     * \attention 1. Callers must not modify this member.
     */
    Node *next;

    /**
     * \brief Raw, aligned storage for one object.
     * \code{.cpp}
     * alignas(T) std::byte storage[sizeof(T)];
     * \endcode
     *
     * \attention 1. Construct a \p T object in this storage before using
     * value().
     */
    alignas(T) std::byte storage[sizeof(T)];

    /**
     * \brief Accesses the object storage as a mutable object pointer.
     * \code{.cpp}
     * T* value() noexcept;
     * \endcode
     *
     * \return Pointer to the object storage.
     * \attention 1. A live \p T object must have been constructed in storage.
     */
    T *value() noexcept { return std::launder(reinterpret_cast<T *>(storage)); }

    /**
     * \brief Accesses the object storage as a read-only object pointer.
     * \code{.cpp}
     * const T* value() const noexcept;
     * \endcode
     *
     * \return Pointer to the object storage.
     * \attention 1. A live \p T object must have been constructed in storage.
     */
    const T *value() const noexcept {
      return std::launder(reinterpret_cast<const T *>(storage));
    }
  };

private:
  struct Chunk {
    Chunk *next = nullptr;

    // bit = 1 -> free
    // bit = 0 -> allocated
    std::atomic<uint64_t> freeMask{0};

    // Generation in which this chunk was the only known source of free nodes.
    std::atomic<uint64_t> soleAvailabilityGeneration{UINT64_MAX};

    Node nodes[ChunkSize];
  };

  using UpstreamTraits = AllocatorTraits<A>;

  static constexpr uint64_t FULL_MASK = [] {
    if constexpr (ChunkSize == 64)
      return UINT64_MAX;
    else
      return (uint64_t{1} << ChunkSize) - 1;
  }();

public:
  /**
   * \brief Constructs an empty pool.
   * \code{.cpp}
   * explicit MPMCMonotonicPoolResource(const A& upstream = {});
   * ~MPMCMonotonicPoolResource();
   * MPMCMonotonicPoolResource(const MPMCMonotonicPoolResource&) = delete;
   * MPMCMonotonicPoolResource& operator=(const MPMCMonotonicPoolResource&) = delete;
   * \endcode
   *
   * \param upstream Allocator used to obtain pool chunks.
   */
  explicit MPMCMonotonicPoolResource(const A &upstream = {})
      : m_upstream(upstream) {}

  MPMCMonotonicPoolResource(const MPMCMonotonicPoolResource &) = delete;
  MPMCMonotonicPoolResource &
  operator=(const MPMCMonotonicPoolResource &) = delete;

  ~MPMCMonotonicPoolResource() {
    // No concurrent operations may exist during destruction.
    Chunk *chunk = m_chunks.load(std::memory_order_relaxed);

    while (chunk) {
      Chunk *next = chunk->next;

      std::destroy_at(chunk);
      UpstreamTraits::template deallocate<Chunk>(m_upstream, chunk, 1);

      chunk = next;
    }
  }

  /**
   * \brief Allocates one node.
   * \code{.cpp}
   * [[nodiscard]] Node* allocate();
   * \endcode
   *
   * \return A free node, allocating a new chunk when necessary.
   */
  [[nodiscard]] Node *allocate() {
    for (;;) {
      const uint64_t generation =
          m_availabilityGeneration.load(std::memory_order_acquire);

      if (m_exhaustedGeneration.load(std::memory_order_acquire) == generation) {
        return grow();
      }

      // Start near the last useful chunk and wrap once.
      Chunk *start = m_allocationCursor.load(std::memory_order_acquire);
      Chunk *head = m_chunks.load(std::memory_order_acquire);

      if (head == nullptr) {
        return grow();
      }

      if (start == nullptr) {
        start = head;
      }

      Chunk *chunk = start;

      do {
        if (Node *node = try_allocate(chunk)) {
          return node;
        }

        Chunk *next = chunk->next != nullptr ? chunk->next : head;
        Chunk *expected = chunk;
        m_allocationCursor.compare_exchange_weak(
            expected, next, std::memory_order_release,
            std::memory_order_relaxed);
        chunk = next;
      } while (chunk != start);

      // A chunk was published or a full chunk became reusable while it was
      // being scanned. Retry so that the newly available nodes are observed.
      if (m_availabilityGeneration.load(std::memory_order_acquire) !=
          generation) {
        continue;
      }

      m_exhaustedGeneration.store(generation, std::memory_order_release);

      if (m_availabilityGeneration.load(std::memory_order_acquire) ==
          generation) {
        return grow();
      }
    }
  }

  /**
   * \brief Returns a node to the pool.
   * \code{.cpp}
   * void deallocate(Node* node) noexcept;
   * \endcode
   *
   * \param node Node previously returned by allocate.
   * \attention 1. \p node must belong to this pool and must not already have
   * been deallocated.
   * \attention 2. The object in \p node must be destroyed before this call.
   */
  void deallocate(Node *node) noexcept {
    Chunk *chunk = node->owner;

    const std::size_t index = static_cast<std::size_t>(node - chunk->nodes);

    assert(index < ChunkSize);

    const uint64_t bit = uint64_t{1} << index;

    [[maybe_unused]] const uint64_t old =
        chunk->freeMask.fetch_or(bit, std::memory_order_release);

    // Catch double-free.
    assert((old & bit) == 0);

    if (old == 0) {
      m_availabilityGeneration.fetch_add(1, std::memory_order_release);
    }
  }

private:
  Node *try_allocate(Chunk *chunk) noexcept {
    uint64_t mask = chunk->freeMask.load(std::memory_order_relaxed);

    while (mask != 0) {
      const unsigned index = std::countr_zero(mask);
      const uint64_t bit = uint64_t{1} << index;

      const uint64_t desired = mask & ~bit;

      if (chunk->freeMask.compare_exchange_weak(mask, desired,
                                                std::memory_order_acquire,
                                                std::memory_order_relaxed)) {
        if (desired == 0) {
          const uint64_t generation = chunk->soleAvailabilityGeneration.load(
              std::memory_order_acquire);

          if (m_availabilityGeneration.load(std::memory_order_acquire) ==
              generation) {
            m_exhaustedGeneration.store(generation,
                                        std::memory_order_release);
          }
        }

        return &chunk->nodes[index];
      }

      // `mask` was updated by compare_exchange_weak.
    }

    return nullptr;
  }

  Node *grow() {
    Chunk *chunk = UpstreamTraits::template allocate<Chunk>(m_upstream, 1);
    assert(chunk != nullptr);

    std::construct_at(chunk);

    for (Node &node : chunk->nodes) {
      node.owner = chunk;
      node.next = nullptr;
    }

    // Give node 0 directly to this thread.
    // All remaining nodes start out free.
    chunk->freeMask.store(FULL_MASK & ~uint64_t{1}, std::memory_order_relaxed);

    // Chunks themselves are monotonic, so this is only a push.
    // There is no ABA problem here because chunks are never removed
    // until destruction.
    Chunk *head = m_chunks.load(std::memory_order_relaxed);

    do {
      chunk->next = head;
    } while (!m_chunks.compare_exchange_weak(
        head, chunk, std::memory_order_release, std::memory_order_relaxed));

    const uint64_t generation =
        m_availabilityGeneration.fetch_add(1, std::memory_order_acq_rel) + 1;
    chunk->soleAvailabilityGeneration.store(generation,
                                            std::memory_order_release);
    m_allocationCursor.store(chunk, std::memory_order_release);

    return &chunk->nodes[0];
  }

private:
  [[no_unique_address]] A m_upstream;
  std::atomic<Chunk *> m_chunks{nullptr};
  std::atomic<Chunk *> m_allocationCursor{nullptr};
  std::atomic<uint64_t> m_availabilityGeneration{0};
  std::atomic<uint64_t> m_exhaustedGeneration{UINT64_MAX};
};

} // namespace strobe
