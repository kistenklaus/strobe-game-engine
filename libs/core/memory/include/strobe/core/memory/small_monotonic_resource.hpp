#pragma once

#include "strobe/core/memory/allocator_traits.hpp"
#include "strobe/core/memory/mallocator.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <new>

namespace strobe {

/**
 * \ingroup core
 * \brief Monotonic resource with inline bootstrap storage.
 * \code{.cpp}
 * template<Allocator Upstream, std::size_t InlineBytes>
 * class SmallMonotonicResource;
 * \endcode
 *
 * Allocations use the embedded buffer first, then obtain upstream-backed
 * chunks. Individual deallocation is ignored; all allocations are released
 * together by destruction or \p release().
 *
 * \tparam Upstream Allocator used for storage beyond the inline buffer.
 * \tparam InlineBytes Number of bytes embedded in the resource.
 */
template <Allocator Upstream, std::size_t InlineBytes>
class SmallMonotonicResource {
private:
  using upstream_type = Upstream;
  using upstream_traits = AllocatorTraits<upstream_type>;

  struct Chunk {
    Chunk *next = nullptr;

    std::size_t allocation_size = 0;
    std::size_t allocation_alignment = 0;

    std::byte *current = nullptr;
    std::byte *end = nullptr;
  };

public:
  /**
   * \brief Default size of an upstream-backed regular chunk.
   * \ingroup core
   * \code{.cpp}
   * static constexpr std::size_t DEFAULT_NORMAL_CHUNK_SIZE = 4096;
   * \endcode
   */
  static constexpr std::size_t DEFAULT_NORMAL_CHUNK_SIZE = 4096;

  /**
   * \brief Constructs, moves, assigns, and destroys the resource.
   * \ingroup core
   * \code{.cpp}
   * explicit SmallMonotonicResource(
   *     const Upstream& upstream = {},
   *     std::size_t normal_chunk_size = DEFAULT_NORMAL_CHUNK_SIZE);
   * SmallMonotonicResource(const SmallMonotonicResource&) = delete;
   * SmallMonotonicResource(SmallMonotonicResource&&) = delete;
   * SmallMonotonicResource& operator=(const SmallMonotonicResource&) = delete;
   * SmallMonotonicResource& operator=(SmallMonotonicResource&&) = delete;
   * ~SmallMonotonicResource();
   * \endcode
   *
   * \param upstream Allocator used after inline storage is exhausted.
   * \param normal_chunk_size Capacity of regular upstream-backed chunks.
   * \attention \p normal_chunk_size must be greater than zero.
   */
  explicit SmallMonotonicResource(
      const upstream_type &upstream = {},
      std::size_t normal_chunk_size = DEFAULT_NORMAL_CHUNK_SIZE)
      : m_upstream(upstream), m_normal_chunk_size(normal_chunk_size) {
    assert(normal_chunk_size > 0);

    reset_inline_storage();
  }

  SmallMonotonicResource(const SmallMonotonicResource &) = delete;

  SmallMonotonicResource &
  operator=(const SmallMonotonicResource &) = delete;

  SmallMonotonicResource(SmallMonotonicResource &&) = delete;

  SmallMonotonicResource &operator=(SmallMonotonicResource &&) = delete;

  ~SmallMonotonicResource() { release(); }

  /**
   * \brief Allocates monotonically from the inline or upstream storage.
   * \ingroup core
   * \code{.cpp}
   * void* allocate(std::size_t size, std::size_t alignment);
   * \endcode
   *
   * \param size Number of bytes to allocate.
   * \param alignment Required power-of-two alignment.
   * \return Aligned storage for the allocation.
   * \throws std::bad_alloc If upstream storage cannot be obtained.
   * \attention Individual allocations cannot be reclaimed.
   */
  [[nodiscard]]
  void *allocate(std::size_t size, std::size_t alignment) {
    assert(size > 0);
    assert(alignment > 0);
    assert(is_power_of_two(alignment));

    /*
     * First use the storage embedded directly in this object.
     */
    if (m_inline_active) {
      if (void *ptr =
              try_allocate(m_inline_current, m_inline_end, size, alignment)) {
        return ptr;
      }

      /*
       * Once an allocation does not fit, abandon the remaining
       * inline tail. We intentionally do not search it again.
       */
      m_inline_active = false;
    }

    /*
     * Then try the current upstream-backed bump chunk.
     */
    if (m_current != nullptr) {
      if (void *ptr = try_allocate(m_current->current, m_current->end, size,
                                   alignment)) {
        return ptr;
      }

      m_current = nullptr;
    }

    /*
     * Large allocations get their own chunk and do not become
     * the active regular chunk.
     */
    if (size > m_normal_chunk_size) {
      return allocate_dedicated(size, alignment);
    }

    allocate_regular_chunk(m_normal_chunk_size, alignment);

    void *ptr =
        try_allocate(m_current->current, m_current->end, size, alignment);

    assert(ptr != nullptr);

    return ptr;
  }

  /**
   * \brief Ignores an individual deallocation.
   * \ingroup core
   * \code{.cpp}
   * void deallocate(void* ptr, std::size_t size, std::size_t alignment) noexcept;
   * \endcode
   *
   * \param ptr Allocation previously returned by this resource.
   * \param size Original allocation size.
   * \param alignment Original allocation alignment.
   */
  void deallocate(void *, std::size_t, std::size_t) noexcept {
    /*
     * Individual deallocation is intentionally ignored.
     */
  }

  /**
   * \brief Releases all upstream-backed chunks and resets inline storage.
   * \ingroup core
   * \code{.cpp}
   * void release() noexcept;
   * \endcode
   *
   * \attention All pointers returned by \p allocate() are invalidated.
   */
  void release() noexcept {
    Chunk *chunk = m_chunks;

    while (chunk != nullptr) {
      Chunk *next = chunk->next;

      const std::size_t allocation_size = chunk->allocation_size;

      const std::size_t allocation_alignment = chunk->allocation_alignment;

      std::destroy_at(chunk);

      upstream_traits::deallocate(m_upstream, static_cast<void *>(chunk),
                                  allocation_size, allocation_alignment);

      chunk = next;
    }

    m_chunks = nullptr;
    m_current = nullptr;

    reset_inline_storage();
  }

  /**
   * \brief Abandons the remaining inline bootstrap storage.
   * \ingroup core
   * \code{.cpp}
   * void finish_bootstrap_chunk() noexcept;
   * \endcode
   *
   * Subsequent allocations use upstream-backed regular or dedicated chunks.
   */
  void finish_bootstrap_chunk() noexcept { m_inline_active = false; }

  /**
   * \brief Returns the inline storage capacity.
   * \ingroup core
   * \code{.cpp}
   * static constexpr std::size_t inline_capacity() noexcept;
   * \endcode
   *
   * \return Number of bytes embedded in the resource.
   */
  [[nodiscard]]
  static constexpr std::size_t inline_capacity() noexcept {
    return InlineBytes;
  }

private:
  [[nodiscard]]
  static constexpr bool is_power_of_two(std::size_t value) noexcept {
    return value != 0 && (value & (value - 1)) == 0;
  }

  [[nodiscard]]
  static constexpr std::size_t align_up(std::size_t value,
                                        std::size_t alignment) noexcept {
    assert(is_power_of_two(alignment));

    return (value + alignment - 1) & ~(alignment - 1);
  }

  [[nodiscard]]
  static void *try_allocate(std::byte *&current, std::byte *end,
                            std::size_t size, std::size_t alignment) noexcept {
    assert(current != nullptr);
    assert(end >= current);
    assert(size > 0);
    assert(is_power_of_two(alignment));

    void *candidate = static_cast<void *>(current);

    std::size_t available = static_cast<std::size_t>(end - current);

    void *result = std::align(alignment, size, candidate, available);

    if (result == nullptr) {
      return nullptr;
    }

    current = static_cast<std::byte *>(result) + size;

    return result;
  }

  void reset_inline_storage() noexcept {
    m_inline_current = m_inline_storage.data();

    m_inline_end = m_inline_storage.data() + InlineBytes;

    m_inline_active = InlineBytes != 0;
  }

  void allocate_regular_chunk(std::size_t payload_capacity,
                              std::size_t required_alignment) {
    Chunk *chunk = allocate_chunk(payload_capacity, required_alignment);

    link_chunk(chunk);
    m_current = chunk;
  }

  [[nodiscard]]
  void *allocate_dedicated(std::size_t size, std::size_t alignment) {
    Chunk *chunk = allocate_chunk(size, alignment);

    link_chunk(chunk);

    void *ptr = try_allocate(chunk->current, chunk->end, size, alignment);

    assert(ptr != nullptr);

    return ptr;
  }

  [[nodiscard]]
  Chunk *allocate_chunk(std::size_t payload_capacity,
                        std::size_t required_alignment) {
    assert(payload_capacity > 0);
    assert(required_alignment > 0);
    assert(is_power_of_two(required_alignment));

    const std::size_t payload_alignment =
        std::max(required_alignment, alignof(std::max_align_t));

    const std::size_t allocation_alignment =
        std::max(payload_alignment, alignof(Chunk));

    static_assert(is_power_of_two(alignof(Chunk)));

    assert(is_power_of_two(payload_alignment));

    assert(is_power_of_two(allocation_alignment));

    const std::size_t payload_offset =
        align_up(sizeof(Chunk), payload_alignment);

    if (payload_capacity >
        std::numeric_limits<std::size_t>::max() - payload_offset) {
      throw std::bad_alloc{};
    }

    const std::size_t allocation_size = payload_offset + payload_capacity;

    void *memory = upstream_traits::allocate(m_upstream, allocation_size,
                                             allocation_alignment);

    if (memory == nullptr) {
      throw std::bad_alloc{};
    }

    Chunk *chunk = static_cast<Chunk *>(memory);

    std::construct_at(chunk);

    std::byte *const bytes = static_cast<std::byte *>(memory);

    std::byte *const payload_begin = bytes + payload_offset;

    chunk->allocation_size = allocation_size;

    chunk->allocation_alignment = allocation_alignment;

    chunk->current = payload_begin;

    chunk->end = payload_begin + payload_capacity;

    return chunk;
  }

  void link_chunk(Chunk *chunk) noexcept {
    assert(chunk != nullptr);

    chunk->next = m_chunks;
    m_chunks = chunk;
  }

private:
  [[no_unique_address]]
  upstream_type m_upstream;

  /*
   * This storage lives inside the resource object. It is on the
   * stack only when the MonotonicResource itself is on the stack.
   */
  alignas(
      std::max_align_t) std::array<std::byte, InlineBytes> m_inline_storage{};

  std::byte *m_inline_current = nullptr;
  std::byte *m_inline_end = nullptr;

  Chunk *m_chunks = nullptr;
  Chunk *m_current = nullptr;

  std::size_t m_normal_chunk_size = DEFAULT_NORMAL_CHUNK_SIZE;

  bool m_inline_active = InlineBytes != 0;
};

static_assert(Allocator<SmallMonotonicResource<strobe::Mallocator, 4096>>);

} // namespace strobe
