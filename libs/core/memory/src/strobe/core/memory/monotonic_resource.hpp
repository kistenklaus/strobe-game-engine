#pragma once

#include "strobe/core/memory/AllocatorTraits.hpp"
#include "strobe/core/memory/Mallocator.hpp"
#include <fmt/printf.h>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <new>

namespace strobe {

template <Allocator Upstream> class MonotonicResource {
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
  static constexpr std::size_t DEFAULT_BOOTSTRAP_CHUNK_SIZE = 4096;
  static constexpr std::size_t DEFAULT_NORMAL_CHUNK_SIZE = 4096;

  explicit MonotonicResource(
      const upstream_type &upstream = {},
      std::size_t bootstrap_chunk_size = DEFAULT_BOOTSTRAP_CHUNK_SIZE,
      std::size_t normal_chunk_size = DEFAULT_NORMAL_CHUNK_SIZE)
      : m_upstream(upstream), m_bootstrap_chunk_size(bootstrap_chunk_size),
        m_normal_chunk_size(normal_chunk_size) {
    assert(bootstrap_chunk_size > 0);
    assert(normal_chunk_size > 0);
  }

  MonotonicResource(const MonotonicResource &) = delete;
  MonotonicResource &operator=(const MonotonicResource &) = delete;

  MonotonicResource(MonotonicResource &&) = delete;
  MonotonicResource &operator=(MonotonicResource &&) = delete;

  ~MonotonicResource() { release(); }

  void *allocate(std::size_t size, std::size_t alignment) {
    assert(size > 0);
    assert(alignment > 0);
    assert(is_power_of_two(alignment));

    if (m_current != nullptr) {
      if (void *ptr = try_allocate_from_current(size, alignment)) {
        return ptr;
      }

      // Current chunk is full for this request. Do not search old chunks.
      // It remains linked in m_chunks for later release().
      m_current = nullptr;
    }

    const std::size_t regular_capacity = next_regular_chunk_capacity();

    if (size > regular_capacity) {
      return allocate_dedicated(size, alignment);
    }

    allocate_regular_chunk(regular_capacity, alignment);

    void *ptr = try_allocate_from_current(size, alignment);
    assert(ptr != nullptr);

    return ptr;
  }

  void deallocate(void *, std::size_t, std::size_t) noexcept {
    // Monotonic resource: individual frees are intentionally ignored.
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
    m_bootstrap_pending = true;
  }

  void finish_bootstrap_chunk() noexcept {
    // Intentionally abandon the active chunk tail and force the next regular
    // allocation to use normal_chunk_size instead of bootstrap_chunk_size.
    m_current = nullptr;
    m_bootstrap_pending = false;
  }

private:
  static constexpr bool is_power_of_two(std::size_t x) noexcept {
    return x != 0 && (x & (x - 1)) == 0;
  }

  static constexpr std::size_t align_up(std::size_t value,
                                        std::size_t alignment) noexcept {
    assert(is_power_of_two(alignment));

    return (value + alignment - 1) & ~(alignment - 1);
  }

  void *try_allocate_from_current(std::size_t size,
                                  std::size_t alignment) noexcept {
    assert(m_current != nullptr);

    const std::uintptr_t raw =
        reinterpret_cast<std::uintptr_t>(m_current->current);

    const std::uintptr_t aligned = align_up(raw, alignment);

    std::byte *const result = reinterpret_cast<std::byte *>(aligned);

    if (result > m_current->end) {
      return nullptr;
    }

    const std::size_t remaining =
        static_cast<std::size_t>(m_current->end - result);

    if (remaining < size) {
      return nullptr;
    }

    m_current->current = result + size;
    return result;
  }

  std::size_t next_regular_chunk_capacity() const noexcept {
    return m_bootstrap_pending ? m_bootstrap_chunk_size : m_normal_chunk_size;
  }

  void allocate_regular_chunk(std::size_t payload_capacity,
                              std::size_t required_alignment) {
    Chunk *chunk = allocate_chunk(payload_capacity, required_alignment);

    link_chunk(chunk);
    m_current = chunk;

    m_bootstrap_pending = false;
  }

  void *allocate_dedicated(std::size_t size, std::size_t alignment) {
    Chunk *chunk = allocate_chunk(size, alignment);

    link_chunk(chunk);

    void *ptr = allocate_from_chunk(chunk, size, alignment);
    assert(ptr != nullptr);

    // Dedicated chunks are not made active. The next small allocation creates
    // or uses a regular bump chunk.
    return ptr;
  }

  void *allocate_from_chunk(Chunk *chunk, std::size_t size,
                            std::size_t alignment) noexcept {
    assert(chunk != nullptr);

    const std::uintptr_t raw = reinterpret_cast<std::uintptr_t>(chunk->current);

    const std::uintptr_t aligned = align_up(raw, alignment);

    std::byte *const result = reinterpret_cast<std::byte *>(aligned);

    if (result > chunk->end) {
      return nullptr;
    }

    const std::size_t remaining = static_cast<std::size_t>(chunk->end - result);

    if (remaining < size) {
      return nullptr;
    }

    chunk->current = result + size;
    return result;
  }

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

    Chunk *chunk = static_cast<Chunk *>(memory);

    try {
      std::construct_at(chunk);
    } catch (...) {
      upstream_traits::deallocate(m_upstream, memory, allocation_size,
                                  allocation_alignment);
      throw;
    }

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
  [[no_unique_address]] upstream_type m_upstream;

  Chunk *m_chunks = nullptr;
  Chunk *m_current = nullptr;

  std::size_t m_bootstrap_chunk_size = DEFAULT_BOOTSTRAP_CHUNK_SIZE;
  std::size_t m_normal_chunk_size = DEFAULT_NORMAL_CHUNK_SIZE;

  bool m_bootstrap_pending = true;
};

static_assert(Allocator<MonotonicResource<strobe::Mallocator>>);

static_assert(Allocator<MonotonicResource<strobe::Mallocator>>);

} // namespace strobe
