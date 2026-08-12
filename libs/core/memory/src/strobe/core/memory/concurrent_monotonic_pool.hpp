
#include "strobe/core/memory/AllocatorTraits.hpp"
#include <atomic>
#include <bit>
#include <cassert>
#include <cstdint>
#include <memory>
#include <new>
#include <new>

namespace strobe {

template <typename T, Allocator A, std::size_t ChunkSize = 64>
class ConcurrentMonotonicPool {
  static_assert(ChunkSize > 0 && ChunkSize <= 64);

  struct Chunk;

public:
  struct Node {
    Chunk *owner;
    Node *next;

    alignas(T) std::byte storage[sizeof(T)];

    T *value() noexcept { return std::launder(reinterpret_cast<T *>(storage)); }

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
  explicit ConcurrentMonotonicPool(const A &upstream = {})
      : m_upstream(upstream) {}

  ConcurrentMonotonicPool(const ConcurrentMonotonicPool &) = delete;
  ConcurrentMonotonicPool &
  operator=(const ConcurrentMonotonicPool &) = delete;

  ~ConcurrentMonotonicPool() {
    // No concurrent operations may exist during destruction.
    Chunk *chunk = m_chunks.load(std::memory_order_relaxed);

    while (chunk) {
      Chunk *next = chunk->next;

      std::destroy_at(chunk);
      UpstreamTraits::template deallocate<Chunk>(m_upstream, chunk, 1);

      chunk = next;
    }
  }

  Node *allocate() {
    // Try existing chunks first.
    Chunk *chunk = m_chunks.load(std::memory_order_acquire);

    while (chunk) {
      if (Node *node = try_allocate(chunk)) {
        return node;
      }

      chunk = chunk->next;
    }

    // All currently observed chunks are full.
    return grow();
  }

  void deallocate(Node *node) noexcept {
    Chunk *chunk = node->owner;

    const std::size_t index = static_cast<std::size_t>(node - chunk->nodes);

    assert(index < ChunkSize);

    const uint64_t bit = uint64_t{1} << index;

    [[maybe_unused]] const uint64_t old =
        chunk->freeMask.fetch_or(bit, std::memory_order_release);

    // Catch double-free.
    assert((old & bit) == 0);
  }

private:
  static Node *try_allocate(Chunk *chunk) noexcept {
    uint64_t mask = chunk->freeMask.load(std::memory_order_relaxed);

    while (mask != 0) {
      const unsigned index = std::countr_zero(mask);
      const uint64_t bit = uint64_t{1} << index;

      const uint64_t desired = mask & ~bit;

      if (chunk->freeMask.compare_exchange_weak(mask, desired,
                                                std::memory_order_acquire,
                                                std::memory_order_relaxed)) {
        return &chunk->nodes[index];
      }

      // `mask` was updated by compare_exchange_weak.
    }

    return nullptr;
  }

  Node *grow() {
    Chunk *chunk = UpstreamTraits::template allocate<Chunk>(m_upstream, 1);

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

    return &chunk->nodes[0];
  }

private:
  [[no_unique_address]] A m_upstream;
  std::atomic<Chunk *> m_chunks{nullptr};
};

} // namespace strobe
