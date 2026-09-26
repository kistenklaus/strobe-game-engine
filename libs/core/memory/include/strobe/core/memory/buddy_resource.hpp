#pragma once

#include "strobe/core/memory/allocator_traits.hpp"
#include "strobe/core/memory/page_allocator.hpp"
#include <array>
#include <bit>
#include <bitset>
#include <cassert>
#include <cstddef>
#include <utility>

namespace strobe {

/**
 * \ingroup core
 * \brief Allocates power-of-two blocks from a fixed backing region.
 * \code{.cpp}
 * template<std::size_t Capacity, std::size_t BlockSize,
 *          typename UpstreamAllocator = PageAllocator>
 * class BuddyResource;
 * \endcode
 *
 * The resource obtains \p Capacity bytes from the upstream allocator during
 * construction and returns blocks whose sizes are rounded up to a power of
 * two. Storage is returned to the resource by calling deallocate with the
 * original request size.
 *
 * \tparam Capacity Total backing-region size in bytes.
 * \tparam BlockSize Smallest allocation size in bytes.
 * \tparam UpstreamAllocator Allocator used for the backing region.
 *
 * \attention 1. \p Capacity and \p BlockSize must be powers of two.
 * \attention 2. \p BlockSize must be less than \p Capacity.
 * \attention 3. The resource is not thread-safe.
 */
template <std::size_t Capacity, std::size_t BlockSize,
          typename UpstreamAllocator = PageAllocator>
class BuddyResource {
  using UpstreamTraits = AllocatorTraits<UpstreamAllocator>;
  static_assert(std::has_single_bit(Capacity));
  static_assert(std::has_single_bit(BlockSize));
  static_assert(BlockSize < Capacity);

  static constexpr std::size_t LogCapacity = std::bit_width(Capacity - 1);

  static constexpr std::size_t LogBlockSize = std::bit_width(BlockSize - 1);
  static constexpr std::size_t LogBlockCount = LogCapacity - LogBlockSize;
  static constexpr std::size_t BlockCount = 1ull << LogBlockCount;

  struct FreelistNode {
    FreelistNode *next;
    FreelistNode *prev;
  };

public:
  /**
   * \brief Constructs a resource with one backing allocation.
   * \code{.cpp}
   * explicit BuddyResource(const UpstreamAllocator& upstream = {});
   * ~BuddyResource();
   * BuddyResource(const BuddyResource&) = delete;
   * BuddyResource& operator=(const BuddyResource&) = delete;
   * BuddyResource(BuddyResource&&) = delete;
   * BuddyResource& operator=(BuddyResource&&) = delete;
   * \endcode
   *
   * \param upstream Allocator used to obtain and release the backing region.
   *
   * \attention 1. The upstream allocator must remain valid until destruction
   * of the resource.
   * \attention 2. Failure to obtain the backing region triggers an assertion.
   */
  explicit BuddyResource(const UpstreamAllocator &upstream = {})
      : m_upstream(upstream) {
    m_buffer = reinterpret_cast<std::byte *>(UpstreamTraits::allocate(
        m_upstream, Capacity, alignof(std::max_align_t)));
    assert(m_buffer != nullptr);
    for (FreelistNode &node : m_freelistStorage) {
      node.next = nullptr;
      node.prev = nullptr;
    }
    for (FreelistNode *&ptr : m_freelists) {
      ptr = nullptr;
    }
    pushFreelist(0, &getFreelistNode(0, 0));
  }
  ~BuddyResource() {
    if (m_buffer != nullptr) {
      UpstreamTraits::deallocate(m_upstream, m_buffer, Capacity,
                                 alignof(std::max_align_t));
      m_buffer = nullptr;
    }
  }
  BuddyResource(const BuddyResource &) = delete;
  BuddyResource &operator=(const BuddyResource &) = delete;
  BuddyResource(BuddyResource &&o) = delete;

  BuddyResource &operator=(BuddyResource &&o) = delete;

  /**
   * \brief Tests whether an address lies within the backing region.
   * \code{.cpp}
   * bool owns(const void* pointer) const;
   * \endcode
   *
   * \param pointer Address to test.
   * \return Whether \p pointer lies within this resource's backing region.
   */
  bool owns(const void *pointer) const {
    if (pointer == nullptr || m_buffer == nullptr) {
      return false;
    }

    const auto address = reinterpret_cast<std::uintptr_t>(pointer);
    const auto begin = reinterpret_cast<std::uintptr_t>(m_buffer);
    return address >= begin && address < begin + Capacity;
  }

  /**
   * \brief Allocates a block from the resource.
   * \code{.cpp}
   * [[nodiscard]] void* allocate(std::size_t size, std::size_t alignment);
   * \endcode
   *
   * \param size Minimum number of bytes requested.
   * \param alignment Required alignment.
   * \return Pointer to an available block, or \c nullptr when no suitable
   * block exists, the aligned allocation does not fit, or \p size exceeds
   * \p Capacity. Requests smaller than BlockSize consume one minimum block.
   * \attention 1. \p size must be nonzero.
   * \attention 2. \p alignment must be a power of two and no greater than
   * \p size.
   */
  void *allocate(std::size_t size, std::size_t alignment) {
    assert(size != 0);
    assert(alignment != 0);
    assert(alignment <= size);
    assert(std::has_single_bit(alignment));
    if (size > Capacity) {
      return nullptr;
    }

    const std::size_t reserved = reserved_size(size, alignment);
    if (reserved == 0) {
      return nullptr;
    }

    const std::size_t block = reserved >> LogBlockSize;
    const int log2Block = floorLog2(block);
    const int order = LogBlockCount - log2Block;
    void *raw = allocateFromFreelist(order);
    return raw == nullptr ? nullptr : align_pointer(raw, alignment);
  }

  /**
   * \brief Allocates a block and reports its actual size.
   * \code{.cpp}
   * [[nodiscard]] std::pair<void*, std::size_t>
   * allocate_at_least(std::size_t size, std::size_t alignment);
   * \endcode
   *
   * \param size Minimum number of bytes requested.
   * \param alignment Required alignment.
   * \return The allocated block and its power-of-two size, or
   * \c {nullptr, 0} when no suitable block exists. Requests smaller than
   * BlockSize report BlockSize available bytes.
   * \attention 1. \p size must be nonzero.
   * \attention 2. \p alignment must be a power of two and no greater than
   * \p size.
   */
  [[nodiscard]] std::pair<void *, std::size_t>
  allocate_at_least(std::size_t size, std::size_t alignment) {
    assert(size != 0);
    assert(alignment != 0);
    assert(alignment <= size);
    assert(std::has_single_bit(alignment));
    if (size > Capacity) {
      return {nullptr, 0};
    }

    const std::size_t actualSize = rounded_size(size);
    const std::size_t reserved = reserved_size(size, alignment);
    if (reserved == 0) {
      return {nullptr, 0};
    }

    const std::size_t block = reserved >> LogBlockSize;
    const int log2Block = floorLog2(block);
    const int order = LogBlockCount - log2Block;
    void *raw = allocateFromFreelist(order);
    void *pointer = raw == nullptr ? nullptr : align_pointer(raw, alignment);
    return {pointer, pointer == nullptr ? 0 : actualSize};
  }

  /**
   * \brief Returns an allocated block to the resource.
   * \code{.cpp}
   * void deallocate(void* pointer, std::size_t size, std::size_t alignment);
   * \endcode
   *
   * \param pointer Block returned by allocate.
   * \param size Original size passed to allocate.
   * \param alignment Alignment passed to allocate.
   * \attention 1. \p pointer must refer to a live allocation from this
   * resource.
   * \attention 2. \p size must be nonzero and no greater than \p Capacity.
   * Invalid size arguments trigger an assertion failure.
   * \attention 3. \p size and \p alignment must describe the original
   * allocation.
   */
  void deallocate(void *pointer, std::size_t size, std::size_t alignment) {
    assert(pointer != nullptr);
    assert(size > 0 && size <= Capacity);
    assert(alignment != 0);
    assert(alignment <= size);
    assert(std::has_single_bit(alignment));
    assert(owns(pointer));
    const std::size_t reserved = reserved_size(size, alignment);
    assert(reserved != 0);
    [[maybe_unused]] const auto offset =
        static_cast<std::size_t>(static_cast<std::byte *>(pointer) - m_buffer);
    assert(reinterpret_cast<std::uintptr_t>(pointer) % alignment == 0);

    const std::size_t blockOffset = (offset / reserved) * reserved;
    void *raw = m_buffer + blockOffset;
    assert(static_cast<std::byte *>(pointer) >= raw);
    assert(static_cast<std::byte *>(pointer) + rounded_size(size) <=
           static_cast<std::byte *>(raw) + reserved);

    const std::size_t block = reserved >> LogBlockSize;
    const int log2Block = floorLog2(block);
    const int order = LogBlockCount - log2Block;
    deallocateToFreelist(raw, order);
  }

private:
  static constexpr std::size_t rounded_size(std::size_t size) noexcept {
    return std::max(BlockSize, std::bit_ceil(size));
  }

  std::size_t reserved_size(std::size_t size,
                            std::size_t alignment) const noexcept {
    const std::size_t rounded = rounded_size(size);
    const std::size_t padding = alignment_padding(alignment);

    if (rounded > Capacity || padding > Capacity - rounded) {
      return 0;
    }

    return std::bit_ceil(rounded + padding);
  }

  std::size_t alignment_padding(std::size_t alignment) const noexcept {
    const auto address = reinterpret_cast<std::uintptr_t>(m_buffer);
    return (std::size_t{0} - address) & (alignment - 1);
  }

  static void *align_pointer(void *pointer, std::size_t alignment) noexcept {
    const auto address = reinterpret_cast<std::uintptr_t>(pointer);
    const auto aligned = (address + alignment - 1) & ~(alignment - 1);
    return reinterpret_cast<void *>(aligned);
  }

  static constexpr int floorLog2(const std::size_t n) {
    return std::bit_width(n) - 1;
  }

  static std::size_t indexOffsetOfOrder(const int order) {
    return (std::size_t{1} << static_cast<std::size_t>(order)) - 1;
  }
  static std::size_t rankOfNodeIndex(const std::size_t nodeIndex,
                                     const int order) {
    const std::size_t offset = indexOffsetOfOrder(order);
    return nodeIndex - offset;
  }
  static std::size_t leftChild(const std::size_t index) {
    return 2 * index + 1;
  }
  static std::size_t leftChildN(const std::size_t index, const std::size_t n) {
    return ((index + 1) << n) - 1;
  }
  static std::size_t rightChild(const std::size_t index) {
    return 2 * index + 2;
  }
  static std::size_t parentOfIndex(const std::size_t index) {
    return (index - 1) / 2;
  }

  FreelistNode &getFreelistNode(const std::size_t index, const int order) {
    if (order == LogBlockCount) {
      const std::size_t orderOffset =
          (std::size_t{1} << static_cast<std::size_t>(order)) - 1;
      assert(orderOffset <= index);
      const std::size_t block = index - orderOffset;
      return m_freelistStorage[block / 2];
    }
    const std::size_t rank = rankOfNodeIndex(index, order);
    std::size_t idx = rank << (LogBlockCount - order - 1);
    return m_freelistStorage[idx];
  }

  FreelistNode *popFreelist(int order) {
    FreelistNode *head = m_freelists[order];
    if (head == nullptr) {
      return nullptr;
    }
    FreelistNode *next = head->next;
    m_freelists[order] = next;
    if (next != nullptr) {
      next->prev = nullptr;
    }
    head->next = nullptr;
    head->prev = nullptr;
    return head;
  }

  void eraseNodeFromFreelist(FreelistNode *node, int order) {
    assert(node != nullptr);

    FreelistNode *next = node->next;

    if (node->prev == nullptr) {
      assert(m_freelists[order] == node);
      m_freelists[order] = next;
    } else {
      FreelistNode *prev = node->prev;
      prev->next = next;
    }

    if (next != nullptr) {
      next->prev = node->prev;
    }

    node->next = nullptr;
    node->prev = nullptr;
  }

  static FreelistNode *rightChildOfFreelistNode(FreelistNode *node,
                                                const int order) {
    if (order == LogBlockCount - 1) {
      return node;
    }
    assert(order != LogBlockCount - 1);
    const std::size_t shift = LogBlockCount - order - 2;
    return node + (std::size_t{1} << shift);
  }

  std::size_t freelistPtrToIndex(FreelistNode *node, const int order) {
    if (order == LogBlockCount) {
      // NOTE: Requires bitset because freelist pointers only give us half
      // resolution!
      const std::size_t location = (node - m_freelistStorage.data());
      const std::size_t left = location * 2;
      const std::size_t offset = indexOffsetOfOrder(order);
      std::size_t leftIndex = offset + left;
      if (m_bitset[leftIndex]) {
        return leftIndex + 1;
      }
      if (m_bitset[leftIndex + 1]) {
        return leftIndex;
      }
    }
    const std::size_t location = (node - m_freelistStorage.data());
    const std::size_t shift = LogBlockCount - order - 1;
    const std::size_t idx = location >> shift;
    const std::size_t offset = indexOffsetOfOrder(order);
    return offset + idx;
  }

  std::size_t ptrToIndex(void *ptr, const int order) const {
    const auto *raw = static_cast<std::byte *>(ptr);
    const std::ptrdiff_t diff = raw - m_buffer;
    const std::size_t block = diff >> LogBlockSize;
    const std::size_t rank = block >> (LogBlockCount - order);
    const std::size_t offset = indexOffsetOfOrder(order);
    return offset + rank;
  }

  static constexpr std::size_t buddyOfIndex(const std::size_t index) {
    assert(index != 0);
    if (index & 0x1) { // is left child.
      return index + 1;
    }
    // is right child.
    return index - 1;
  }

  void pushFreelist(const int order, FreelistNode *node) {
    assert(node != nullptr);
    FreelistNode *head = m_freelists[order];
    m_freelists[order] = node;
    if (head != nullptr) {
      head->prev = node;
    }
    node->next = head;
    node->prev = nullptr;
  }

  void *allocateFromFreelist(const int order) {
    FreelistNode *node = nullptr;
    int o = order;
    while (o >= 0) {
      node = popFreelist(o);
      if (node == nullptr) {
        --o;
      } else {
        break;
      }
    }
    if (node == nullptr) {
      return nullptr;
    }

    // Break up blocks
    std::size_t index = freelistPtrToIndex(node, o);
    const std::size_t rank = rankOfNodeIndex(index, o);
    const std::size_t ptrOffset = (rank << (LogBlockCount - o)) << LogBlockSize;
    m_bitset.set(index);
    while (o != order) {
      index = index * 2 + 1;
      m_bitset.set(index);
      FreelistNode *right = rightChildOfFreelistNode(node, o);
      // assert(right >= &m_freelistStorage.front());
      // assert(right < &m_freelistStorage.back());
      ++o;
      pushFreelist(o, right);
    }
    return m_buffer + ptrOffset;
  }

  void deallocateToFreelist(void *ptr, const int order) {
    std::size_t index = ptrToIndex(ptr, order);
    assert(m_bitset[index]);

    std::size_t o = order;
    while (index != 0) {
      m_bitset.reset(index);
      std::size_t buddy = buddyOfIndex(index);
      if (m_bitset[buddy]) {
        break;
      }
      // NOTE: Coalesce buddies
      FreelistNode &node = getFreelistNode(buddy, o);
      eraseNodeFromFreelist(&node, o);
      index = parentOfIndex(index);
      --o;
    }
    if (index == 0) {
      m_bitset.reset(0);
    }
    FreelistNode &node = getFreelistNode(index, o);
    pushFreelist(o, &node);
  }

  UpstreamAllocator m_upstream;

  std::bitset<BlockCount * 2> m_bitset;
  std::byte *m_buffer;
  std::array<FreelistNode, BlockCount / 2> m_freelistStorage;
  std::array<FreelistNode *, LogBlockCount + 1> m_freelists;
};

} // namespace strobe

static_assert(strobe::OverAllocator<strobe::BuddyResource<1024, 16>>);
