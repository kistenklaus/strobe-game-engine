#pragma once

#include "strobe/core/memory/AllocatorTraits.hpp"

#include <algorithm>
#include <atomic>
#include <cassert>
#include <cstddef>
#include <ratio>
#include <tracy/Tracy.hpp>
#include <utility>

namespace strobe {

template <std::size_t BlockSize, std::size_t BlockAlign, Allocator A,
          typename GrowthFactor = std::ratio<2, 1>>
class MPSCMonotonicPoolResource {
public:
  using Self =
      MPSCMonotonicPoolResource<BlockSize, BlockAlign, A, GrowthFactor>;

  using upstream_allocator = A;
  using upstream_traits = AllocatorTraits<upstream_allocator>;

  static constexpr std::size_t block_size = std::max(
      std::max(BlockSize, BlockAlign), sizeof(void *) + sizeof(std::size_t));

  static constexpr std::size_t block_align = BlockAlign;

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

  MPSCMonotonicPoolResource select_on_container_copy_construction() const {
    return MPSCMonotonicPoolResource(m_upstream);
  }

  // Allocation must be externally synchronized.
  void *allocate(std::size_t size, std::size_t align) {
    assert(size <= block_size);
    assert(align <= block_align);
    assert((block_align % align) == 0);

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

  void *allocate() { return allocate(block_size, block_align); }

  // Deallocation may be called concurrently.
  void deallocate(void *ptr, std::size_t size, std::size_t align) noexcept {
    assert(size <= block_size);
    assert(align <= block_align);
    assert((block_align % align) == 0);

    deallocate(ptr);
  }

  // Deallocation may be called concurrently.
  void deallocate(void *ptr) noexcept {
    assert(ptr != nullptr);

    auto *node = reinterpret_cast<Node *>(ptr);

    Node *head = m_returned.load(std::memory_order_relaxed);

    do {
      node->free.next = head;
    } while (!m_returned.compare_exchange_weak(
        head, node, std::memory_order_release, std::memory_order_relaxed));
  }

  bool operator==(const MPSCMonotonicPoolResource &o) const noexcept {
    return this == &o;
  }

  bool operator!=(const MPSCMonotonicPoolResource &o) const noexcept = default;

  bool owns(const void *p) const noexcept {
    const auto *ptr = reinterpret_cast<const std::byte *>(p);

    Node *block = m_buffer;

    while (block != nullptr) {
      const std::size_t count = block->block.blockSize;

      const auto *begin = reinterpret_cast<const std::byte *>(block);

      const auto *end = reinterpret_cast<const std::byte *>(block + count);

      if (ptr >= begin && ptr < end) {
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
      next_block_size =
          (m_buffer->block.blockSize * GrowthFactor::num) / GrowthFactor::den;
    }

    // Need one header and at least one usable node.
    next_block_size = std::max<std::size_t>(next_block_size, 2);

    Node *block =
        upstream_traits::template allocate<Node>(m_upstream, next_block_size);

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
