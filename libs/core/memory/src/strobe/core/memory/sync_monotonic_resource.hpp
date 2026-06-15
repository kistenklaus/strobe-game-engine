#pragma once

#include "strobe/core/memory/AllocatorTraits.hpp"
#include "strobe/core/memory/Mallocator.hpp"

#include <algorithm>
#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <mutex>
#include <new>

namespace strobe {

template <Allocator Upstream>
class SyncMonotonicResource {
private:
  using upstream_type =
      Upstream;

  using upstream_traits =
      AllocatorTraits<upstream_type>;

  struct Chunk {
    Chunk *next = nullptr;

    std::size_t allocation_size = 0;
    std::size_t allocation_alignment = 0;

    std::byte *payload = nullptr;
    std::size_t capacity = 0;

    std::atomic<std::size_t> offset = 0;
  };

  static_assert(
      std::atomic<std::size_t>::is_always_lock_free,
      "ConcurrentMonotonicResource requires lock-free atomic<size_t>");

  static_assert(
      std::atomic<Chunk *>::is_always_lock_free,
      "ConcurrentMonotonicResource requires lock-free atomic pointers");

public:
  static constexpr std::size_t DEFAULT_BOOTSTRAP_CHUNK_SIZE =
      4096;

  static constexpr std::size_t DEFAULT_NORMAL_CHUNK_SIZE =
      4096;

  explicit SyncMonotonicResource(
      const upstream_type &upstream,
      std::size_t bootstrap_chunk_size =
          DEFAULT_BOOTSTRAP_CHUNK_SIZE,
      std::size_t normal_chunk_size =
          DEFAULT_NORMAL_CHUNK_SIZE)
      : m_upstream(upstream),
        m_bootstrap_chunk_size(bootstrap_chunk_size),
        m_normal_chunk_size(normal_chunk_size) {
    assert(bootstrap_chunk_size > 0);
    assert(normal_chunk_size > 0);
  }

  SyncMonotonicResource(
      const SyncMonotonicResource &) = delete;

  SyncMonotonicResource &
  operator=(
      const SyncMonotonicResource &) = delete;

  SyncMonotonicResource(
      SyncMonotonicResource &&) = delete;

  SyncMonotonicResource &
  operator=(
      SyncMonotonicResource &&) = delete;

  ~SyncMonotonicResource() {
    release();
  }

  void *allocate(
      std::size_t size,
      std::size_t alignment) {
    assert(size > 0);
    assert(alignment > 0);
    assert(is_power_of_two(alignment));

    /*
     * Lock-free fast path.
     *
     * A stale current pointer is safe because chunks are never individually
     * released. They remain alive until release() or destruction.
     */
    if (Chunk *current =
            m_current.load(
                std::memory_order_acquire)) {
      if (void *ptr =
              try_allocate_from_chunk(
                  current,
                  size,
                  alignment)) {
        return ptr;
      }
    }

    return allocate_slow(
        size,
        alignment);
  }

  void deallocate(
      void *,
      std::size_t,
      std::size_t) noexcept {
    // Individual deallocation is intentionally ignored.
  }

  /*
   * Requires exclusive access:
   *
   * No allocate() call may be active or begin concurrently with release().
   */
  void release() noexcept {
    std::lock_guard lock{
        m_chunk_mutex
    };

    m_current.store(
        nullptr,
        std::memory_order_relaxed);

    Chunk *chunk =
        m_chunks;

    while (chunk != nullptr) {
      Chunk *next =
          chunk->next;

      const std::size_t allocation_size =
          chunk->allocation_size;

      const std::size_t allocation_alignment =
          chunk->allocation_alignment;

      std::destroy_at(chunk);

      upstream_traits::deallocate(
          m_upstream,
          static_cast<void *>(chunk),
          allocation_size,
          allocation_alignment);

      chunk = next;
    }

    m_chunks = nullptr;
    m_bootstrap_pending = true;
  }

  /*
   * Requires exclusive access with respect to allocate().
   *
   * The active bootstrap chunk remains owned by the resource, but its
   * remaining tail is intentionally abandoned.
   */
  void finish_bootstrap_chunk() noexcept {
    std::lock_guard lock{
        m_chunk_mutex
    };

    m_current.store(
        nullptr,
        std::memory_order_relaxed);

    m_bootstrap_pending = false;
  }

private:
  static constexpr bool is_power_of_two(
      std::size_t value) noexcept {
    return
        value != 0 &&
        (value & (value - 1)) == 0;
  }

  static constexpr std::size_t align_up(
      std::size_t value,
      std::size_t alignment) noexcept {
    assert(is_power_of_two(alignment));

    return
        (value + alignment - 1) &
        ~(alignment - 1);
  }

  static constexpr std::size_t alignment_padding(
      std::uintptr_t address,
      std::size_t alignment) noexcept {
    assert(is_power_of_two(alignment));

    const std::size_t misalignment =
        static_cast<std::size_t>(
            address &
            static_cast<std::uintptr_t>(
                alignment - 1));

    return
        misalignment == 0
            ? 0
            : alignment - misalignment;
  }

  /*
   * Lock-free reservation inside one chunk.
   *
   * Different requests may use different alignments, so the reservation must
   * CAS the aligned end offset rather than using fetch_add().
   */
  static void *try_allocate_from_chunk(
      Chunk *chunk,
      std::size_t size,
      std::size_t alignment) noexcept {
    assert(chunk != nullptr);
    assert(size > 0);
    assert(is_power_of_two(alignment));

    std::size_t current =
        chunk->offset.load(
            std::memory_order_relaxed);

    for (;;) {
      assert(current <= chunk->capacity);

      const std::size_t remaining =
          chunk->capacity - current;

      const std::uintptr_t address =
          reinterpret_cast<std::uintptr_t>(
              chunk->payload + current);

      const std::size_t padding =
          alignment_padding(
              address,
              alignment);

      if (padding > remaining) {
        return nullptr;
      }

      if (size > remaining - padding) {
        return nullptr;
      }

      const std::size_t aligned_offset =
          current + padding;

      const std::size_t next =
          aligned_offset + size;

      if (chunk->offset.compare_exchange_weak(
              current,
              next,
              std::memory_order_relaxed,
              std::memory_order_relaxed)) {
        return
            static_cast<void *>(
                chunk->payload +
                aligned_offset);
      }

      /*
       * compare_exchange_weak updates current on failure. Recompute alignment
       * and capacity from the new offset.
       */
    }
  }

  void *allocate_slow(
      std::size_t size,
      std::size_t alignment) {
    std::lock_guard lock{
        m_chunk_mutex
    };

    /*
     * Another thread may have installed a new current chunk while this thread
     * was waiting for the mutex.
     */
    if (Chunk *current =
            m_current.load(
                std::memory_order_acquire)) {
      if (void *ptr =
              try_allocate_from_chunk(
                  current,
                  size,
                  alignment)) {
        return ptr;
      }
    }

    const std::size_t regular_capacity =
        next_regular_chunk_capacity_locked();

    /*
     * Dedicated chunks are linked for release(), but are not installed as the
     * active regular chunk.
     */
    if (size > regular_capacity) {
      return allocate_dedicated_locked(
          size,
          alignment);
    }

    Chunk *chunk =
        allocate_chunk_locked(
            regular_capacity,
            alignment);

    link_chunk_locked(chunk);

    m_bootstrap_pending = false;

    m_current.store(
        chunk,
        std::memory_order_release);

    void *ptr =
        try_allocate_from_chunk(
            chunk,
            size,
            alignment);

    assert(ptr != nullptr);

    return ptr;
  }

  std::size_t
  next_regular_chunk_capacity_locked() const noexcept {
    return
        m_bootstrap_pending
            ? m_bootstrap_chunk_size
            : m_normal_chunk_size;
  }

  void *allocate_dedicated_locked(
      std::size_t size,
      std::size_t alignment) {
    Chunk *chunk =
        allocate_chunk_locked(
            size,
            alignment);

    link_chunk_locked(chunk);

    void *ptr =
        try_allocate_from_chunk(
            chunk,
            size,
            alignment);

    assert(ptr != nullptr);

    return ptr;
  }

  Chunk *allocate_chunk_locked(
      std::size_t payload_capacity,
      std::size_t required_alignment) {
    assert(payload_capacity > 0);
    assert(required_alignment > 0);
    assert(is_power_of_two(required_alignment));

    const std::size_t payload_alignment =
        std::max(
            required_alignment,
            alignof(std::max_align_t));

    const std::size_t allocation_alignment =
        std::max(
            payload_alignment,
            alignof(Chunk));

    static_assert(
        is_power_of_two(
            alignof(Chunk)));

    assert(
        is_power_of_two(
            payload_alignment));

    assert(
        is_power_of_two(
            allocation_alignment));

    const std::size_t payload_offset =
        align_up(
            sizeof(Chunk),
            payload_alignment);

    if (payload_capacity >
        std::numeric_limits<std::size_t>::max() -
            payload_offset) {
      throw std::bad_alloc{};
    }

    const std::size_t allocation_size =
        payload_offset +
        payload_capacity;

    void *memory =
        upstream_traits::allocate(
            m_upstream,
            allocation_size,
            allocation_alignment);

    Chunk *chunk =
        static_cast<Chunk *>(memory);

    try {
      std::construct_at(chunk);
    } catch (...) {
      upstream_traits::deallocate(
          m_upstream,
          memory,
          allocation_size,
          allocation_alignment);

      throw;
    }

    std::byte *bytes =
        static_cast<std::byte *>(memory);

    chunk->allocation_size =
        allocation_size;

    chunk->allocation_alignment =
        allocation_alignment;

    chunk->payload =
        bytes + payload_offset;

    chunk->capacity =
        payload_capacity;

    chunk->offset.store(
        0,
        std::memory_order_relaxed);

    return chunk;
  }

  void link_chunk_locked(
      Chunk *chunk) noexcept {
    assert(chunk != nullptr);

    chunk->next =
        m_chunks;

    m_chunks =
        chunk;
  }

private:
  [[no_unique_address]]
  upstream_type m_upstream;

  /*
   * Protects:
   *
   * - upstream chunk allocation;
   * - m_chunks;
   * - m_bootstrap_pending;
   * - installation of a new current chunk.
   *
   * It is never acquired on the successful bump-allocation path.
   */
  std::mutex m_chunk_mutex;

  /*
   * Published with release after a chunk has been fully initialized.
   * Loaded with acquire by concurrent allocators.
   */
  std::atomic<Chunk *> m_current = nullptr;

  /*
   * Only accessed while holding m_chunk_mutex, or during exclusive teardown.
   */
  Chunk *m_chunks = nullptr;

  std::size_t m_bootstrap_chunk_size =
      DEFAULT_BOOTSTRAP_CHUNK_SIZE;

  std::size_t m_normal_chunk_size =
      DEFAULT_NORMAL_CHUNK_SIZE;

  bool m_bootstrap_pending = true;
};

static_assert(
    Allocator<
        SyncMonotonicResource<
            strobe::Mallocator>>);

} // namespace strobe
