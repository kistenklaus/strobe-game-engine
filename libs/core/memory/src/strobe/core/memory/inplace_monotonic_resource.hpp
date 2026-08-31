#pragma once

#include "strobe/core/memory/AllocatorTraits.hpp"
#include "strobe/core/memory/Mallocator.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <new>

namespace strobe {

template <Allocator Upstream, std::size_t InlineBytes>
class InplaceMonotonicResource {
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
  static constexpr std::size_t DEFAULT_NORMAL_CHUNK_SIZE = 4096;

  explicit InplaceMonotonicResource(
      const upstream_type &upstream = {},
      std::size_t normal_chunk_size = DEFAULT_NORMAL_CHUNK_SIZE)
      : m_upstream(upstream), m_normal_chunk_size(normal_chunk_size) {
    assert(normal_chunk_size > 0);

    reset_inline_storage();
  }

  InplaceMonotonicResource(const InplaceMonotonicResource &) = delete;

  InplaceMonotonicResource &
  operator=(const InplaceMonotonicResource &) = delete;

  InplaceMonotonicResource(InplaceMonotonicResource &&) = delete;

  InplaceMonotonicResource &operator=(InplaceMonotonicResource &&) = delete;

  ~InplaceMonotonicResource() { release(); }

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

  void deallocate(void *, std::size_t, std::size_t) noexcept {
    /*
     * Individual deallocation is intentionally ignored.
     */
  }

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

  /*
   * Abandon the unused part of the inline/bootstrap buffer.
   *
   * The next allocation will use an upstream-backed regular or
   * dedicated chunk.
   */
  void finish_bootstrap_chunk() noexcept { m_inline_active = false; }

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

static_assert(Allocator<InplaceMonotonicResource<strobe::Mallocator, 4096>>);

} // namespace strobe
