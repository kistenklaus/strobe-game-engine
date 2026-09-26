#pragma once

#include "strobe/core/memory/allocator_traits.hpp"

#include <algorithm>
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
 * \brief Single-threaded growing pool for fixed-size allocations.
 * \code{.cpp}
 * template<std::size_t BlockSize, std::size_t BlockAlign, Allocator A,
 *          typename GrowthFactor = std::ratio<2, 1>>
 * class MonotonicPoolResource;
 * \endcode
 *
 * The resource obtains chunks from the upstream allocator and returns blocks
 * of a fixed maximum size. Deallocated blocks are reused, while chunks remain
 * owned by the resource until release() or destruction.
 *
 * \tparam BlockSize Maximum requested allocation size in bytes.
 * \tparam BlockAlign Alignment supported by the pool.
 * \tparam A Upstream allocator used for pool chunks.
 * \tparam GrowthFactor Ratio used to grow subsequent chunks.
 *
 * \attention 1. \p BlockSize and \p BlockAlign must be nonzero, and
 * \p BlockAlign must be a power of two.
 * \attention 2. \p GrowthFactor must be positive and no less than one.
 * \attention 3. Allocation, deallocation, release(), and destruction must not
 * execute concurrently.
 */
template <std::size_t BlockSize, std::size_t BlockAlign, Allocator A,
          typename GrowthFactor = std::ratio<2, 1>>
class MonotonicPoolResource {
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
   * using Self = MonotonicPoolResource<BlockSize, BlockAlign, A, GrowthFactor>;
   * \endcode
   */
  using Self = MonotonicPoolResource<BlockSize, BlockAlign, A, GrowthFactor>;

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
   * \brief Maximum allocation size in bytes.
   * \code{.cpp}
   * static constexpr std::size_t block_size;
   * \endcode
   */
  static constexpr std::size_t block_size = std::max(BlockSize, BlockAlign);

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
   * explicit MonotonicPoolResource(const A& upstream = {});
   * ~MonotonicPoolResource();
   * MonotonicPoolResource(const MonotonicPoolResource&) = delete;
   * MonotonicPoolResource& operator=(const MonotonicPoolResource&) = delete;
   * MonotonicPoolResource(MonotonicPoolResource&& other);
   * MonotonicPoolResource& operator=(MonotonicPoolResource&& other);
   * \endcode
   *
   * \param upstream Allocator used to obtain and release chunks.
   */
  explicit MonotonicPoolResource(const A &upstream = {})
      : m_upstream(upstream), m_buffer(nullptr), m_freelist(nullptr) {}

  ~MonotonicPoolResource() { release(); }

  MonotonicPoolResource(const MonotonicPoolResource &) = delete;
  MonotonicPoolResource &operator=(const MonotonicPoolResource &) = delete;

  /**
   * \brief Creates an independent resource using the same upstream allocator.
   * \code{.cpp}
   * MonotonicPoolResource select_on_container_copy_construction() const;
   * \endcode
   *
   * \return An empty resource with a copied upstream allocator.
   */
  MonotonicPoolResource select_on_container_copy_construction() const {
    return MonotonicPoolResource(m_upstream);
  }

  MonotonicPoolResource(MonotonicPoolResource &&other)
      : m_upstream(std::move(other.m_upstream)),
        m_buffer(std::exchange(other.m_buffer, nullptr)),
        m_freelist(std::exchange(other.m_freelist, nullptr)) {}
  MonotonicPoolResource &operator=(MonotonicPoolResource &&other) {
    if (this == &other) {
      return *this;
    }

    static_assert(
        upstream_traits::propagate_on_container_move_assignment,
        "Require for upstream allocators, because otherwise would "
        "have to invalidated allocated pointers on move, which breaks the "
        "core requirement of a memory resource.");

    release();
    m_upstream = std::move(other.m_upstream);
    m_buffer = std::exchange(other.m_buffer, nullptr);
    m_freelist = std::exchange(other.m_freelist, nullptr);
    return *this;
  }

  /**
   * \brief Allocates one fixed-size block.
   * \code{.cpp}
   * [[nodiscard]] void* allocate(std::size_t size = BlockSize,
   *                               std::size_t alignment = BlockAlign);
   * \endcode
   *
   * \param size Requested size in bytes.
   * \param alignment Required alignment.
   * \return A block satisfying the request.
   * \attention 1. \p size must be nonzero and no greater than block_size.
   * \attention 2. \p alignment must be a nonzero power of two no greater than
   * block_align and divide BlockAlign.
   */
  [[nodiscard]]
  void *allocate(std::size_t size = BlockSize,
                 std::size_t alignment = BlockAlign) {
    assert(size > 0);
    assert(size <= block_size);
    assert(alignment > 0);
    assert(std::has_single_bit(alignment));
    assert(alignment <= block_align);
    assert((BlockAlign % alignment) == 0);
    (void)size;
    (void)alignment;
    if (m_freelist == nullptr) {
      allocate_block();
    }
    return popFreelist();
  }

  /**
   * \brief Returns a block to the freelist.
   * \code{.cpp}
   * void deallocate(void* pointer, std::size_t size,
   *                 std::size_t alignment) noexcept;
   * void deallocate(void* pointer) noexcept;
   * \endcode
   *
   * \param pointer Block previously returned by allocate.
   * \param size Original requested size when using the sized overload.
   * \param alignment Original alignment when using the sized overload.
   * \attention 1. \p pointer must belong to this resource and must not already
   * have been deallocated.
   * \attention 2. The object using the block must no longer be alive.
   */
  void deallocate(void *pointer, std::size_t size,
                  std::size_t alignment) noexcept {
    assert(pointer != nullptr);
    assert(size > 0);
    assert(size <= block_size);
    assert(alignment > 0);
    assert(std::has_single_bit(alignment));
    assert(alignment <= block_align);
    assert((BlockAlign % alignment) == 0);
    assert(owns(pointer));
    (void)size;
    (void)alignment;
    deallocate(pointer);
  }

  void deallocate(void *pointer) noexcept {
    assert(pointer != nullptr);
    assert(owns(pointer));
    pushFreelist(pointer);
  }

  /**
   * \brief Compares resource identity.
   * \code{.cpp}
   * bool operator==(const MonotonicPoolResource& other) const noexcept;
   * \endcode
   *
   * \param other Resource to compare.
   * \return Whether both references denote the same resource object.
   */
  bool operator==(const MonotonicPoolResource &other) const noexcept {
    return this == &other;
  }

  /**
   * \brief Compares resource identity for inequality.
   * \code{.cpp}
   * bool operator!=(const MonotonicPoolResource& other) const noexcept;
   * \endcode
   *
   * \param other Resource to compare.
   * \return Whether the resources are different objects.
   */
  bool operator!=(const MonotonicPoolResource &other) const noexcept = default;

  /**
   * \brief Tests whether a block address belongs to this resource.
   * \code{.cpp}
   * bool owns(const void* pointer) const noexcept;
   * \endcode
   *
   * \param pointer Address to test.
   * \return Whether \p pointer identifies a block owned by this resource.
   */
  bool owns(const void *pointer) const noexcept {
    if (pointer == nullptr) {
      return false;
    }

    const auto address = reinterpret_cast<std::uintptr_t>(pointer);
    Node *block = m_buffer;
    while (block != nullptr) {
      const auto begin = reinterpret_cast<std::uintptr_t>(block + 1);
      const auto end = reinterpret_cast<std::uintptr_t>(
          block + block->block.blockSize);
      if (address >= begin && address < end &&
          (address - begin) % sizeof(Node) == 0) {
        return true;
      }
      block = block->block.next;
    }
    return false;
  }

public:
  /**
   * \brief Releases every chunk and resets the pool.
   * \code{.cpp}
   * void release() noexcept;
   * \endcode
   *
   * \attention 1. All outstanding blocks become invalid.
   */
  void release() noexcept {
    Node *block = m_buffer;
    while (block != nullptr) {
      Node *nextBlock = block->block.next;
      // Deallocate the entire block
      upstream_traits::template deallocate<Node>(m_upstream, block,
                                                 block->block.blockSize);
      block = nextBlock;
    }
    // Clear all state
    m_buffer = nullptr;
    m_freelist = nullptr;
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
    alignas(BlockAlign) std::byte value[block_size];
  };

  void *popFreelist() {
    void *ptr = reinterpret_cast<void *>(m_freelist);
    m_freelist = m_freelist->free.next;
    return ptr;
  }

  void pushFreelist(void *ptr) {
    Node *node = reinterpret_cast<Node *>(ptr);
    node->free.next = m_freelist;
    m_freelist = node;
  }

  void allocate_block() {
    ZoneScopedN("MonotonicPoolResource::allocate_block");
    std::size_t nextBlockSize;
    if (m_buffer == nullptr) {
      // block size is +1 the capacity.
      nextBlockSize = 2;
    } else {
      assert(m_buffer->block.blockSize <=
             std::numeric_limits<std::size_t>::max() /
                 GrowthFactor::num);
      nextBlockSize =
          (m_buffer->block.blockSize * GrowthFactor::num) / GrowthFactor::den;
    }
    assert(nextBlockSize >= 2);
    Node *block =
        upstream_traits::template allocate<Node>(m_upstream, nextBlockSize);
    assert(block != nullptr);
    Node *header = block;

    // Initialize the free list in the remaining nodes
    Node *const begin = header + 1;
    Node *const end = header + nextBlockSize;
    for (Node *current = begin; current < end - 1; ++current) {
      current->free.next = current + 1;
    }
    (end - 1)->free.next = nullptr; // Last node in the free list

    (end - 1)->free.next = m_freelist;
    m_freelist = begin;

    block->block.blockSize = nextBlockSize;
    block->block.next = m_buffer;
    m_buffer = block;
  }

private:
  [[no_unique_address]] upstream_allocator m_upstream;
  Node *m_buffer;   // fwd list of allocated chunks
  Node *m_freelist; // freelist.
};

} // namespace strobe
