#pragma once

#include "strobe/core/memory/AllocatorTraits.hpp"
#include "strobe/core/memory/Mallocator.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <new>
#include <type_traits>
#include <utility>

namespace strobe {

template <Allocator Upstream> class MonotonicResource {
private:
  using upstream_type = Upstream;
  using upstream_traits = AllocatorTraits<upstream_type>;

  struct Chunk {
    Chunk *next = nullptr;

    void *memory = nullptr;
    std::size_t capacity = 0;
    std::size_t alignment = 0;

    std::byte *current = nullptr;
    std::byte *end = nullptr;
  };

public:
  static constexpr std::size_t DEFAULT_INITIAL_CHUNK_SIZE = 4096;
  static constexpr std::size_t DEFAULT_MAX_CHUNK_SIZE =
      std::numeric_limits<std::size_t>::max() / 2;

  explicit MonotonicResource(
      const upstream_type &upstream,
      std::size_t initial_chunk_size = DEFAULT_INITIAL_CHUNK_SIZE,
      std::size_t max_chunk_size = DEFAULT_MAX_CHUNK_SIZE)
      : m_upstream(upstream), m_next_chunk_size(initial_chunk_size),
        m_max_chunk_size(max_chunk_size) {
    assert(initial_chunk_size > 0);
    assert(max_chunk_size >= initial_chunk_size);
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

    if (void *ptr = try_allocate_from_current(size, alignment)) {
      return ptr;
    }

    allocate_chunk_for(size, alignment);

    void *ptr = try_allocate_from_current(size, alignment);
    assert(ptr != nullptr);

    return ptr;
  }

  void deallocate(void *, std::size_t, std::size_t) noexcept {
    // Monotonic resource: individual frees are intentionally ignored.
  }

  void reset() noexcept {
    for (Chunk *chunk = m_chunks; chunk != nullptr; chunk = chunk->next) {
      chunk->current = static_cast<std::byte *>(chunk->memory);
    }
  }

  void release() noexcept {
    Chunk *chunk = m_chunks;

    while (chunk != nullptr) {
      Chunk *next = chunk->next;

      upstream_traits::deallocate(m_upstream, chunk->memory, chunk->capacity,
                                  chunk->alignment);

      std::destroy_at(chunk);

      upstream_traits::template deallocate<Chunk>(m_upstream, chunk);

      chunk = next;
    }

    m_chunks = nullptr;
    m_current = nullptr;
    m_total_allocated = 0;
  }

  std::size_t total_allocated() const noexcept { return m_total_allocated; }

  std::size_t next_chunk_size() const noexcept { return m_next_chunk_size; }

private:
  static constexpr bool is_power_of_two(std::size_t x) noexcept {
    return x != 0 && (x & (x - 1)) == 0;
  }

  static std::uintptr_t align_up(std::uintptr_t value,
                                 std::size_t alignment) noexcept {
    assert(is_power_of_two(alignment));
    return (value + alignment - 1) &
           ~(static_cast<std::uintptr_t>(alignment) - 1);
  }

  void *try_allocate_from_current(std::size_t size,
                                  std::size_t alignment) noexcept {
    if (m_current == nullptr) {
      return nullptr;
    }

    const std::uintptr_t raw =
        reinterpret_cast<std::uintptr_t>(m_current->current);

    const std::uintptr_t aligned = align_up(raw, alignment);

    std::byte *const result = reinterpret_cast<std::byte *>(aligned);
    std::byte *const next = result + size;

    if (next > m_current->end) {
      return nullptr;
    }

    m_current->current = next;
    return result;
  }

  void allocate_chunk_for(std::size_t required_size,
                          std::size_t required_alignment) {
    const std::size_t chunk_size = std::max(required_size, m_next_chunk_size);

    const std::size_t chunk_alignment =
        std::max(required_alignment, alignof(std::max_align_t));

    void *memory =
        upstream_traits::allocate(m_upstream, chunk_size, chunk_alignment);

    Chunk *chunk = upstream_traits::template allocate<Chunk>(m_upstream);

    try {
      std::construct_at(chunk);
    } catch (...) {
      upstream_traits::deallocate(m_upstream, memory, chunk_size,
                                  chunk_alignment);

      upstream_traits::template deallocate<Chunk>(m_upstream, chunk);
      throw;
    }

    chunk->next = m_chunks;
    chunk->memory = memory;
    chunk->capacity = chunk_size;
    chunk->alignment = chunk_alignment;
    chunk->current = static_cast<std::byte *>(memory);
    chunk->end = static_cast<std::byte *>(memory) + chunk_size;

    m_chunks = chunk;
    m_current = chunk;

    m_total_allocated += chunk_size;

    grow_next_chunk_size(chunk_size);
  }

  void grow_next_chunk_size(std::size_t last_chunk_size) noexcept {
    const std::size_t grown = last_chunk_size + last_chunk_size / 2;

    if (grown < last_chunk_size || grown > m_max_chunk_size) {
      m_next_chunk_size = m_max_chunk_size;
      return;
    }

    m_next_chunk_size = std::max(grown, m_next_chunk_size);
  }

private:
  [[no_unique_address]] upstream_type m_upstream;

  Chunk *m_chunks = nullptr;
  Chunk *m_current = nullptr;

  std::size_t m_next_chunk_size = DEFAULT_INITIAL_CHUNK_SIZE;
  std::size_t m_max_chunk_size = DEFAULT_MAX_CHUNK_SIZE;
  std::size_t m_total_allocated = 0;
};

static_assert(Allocator<MonotonicResource<strobe::Mallocator>>);

} // namespace strobe
