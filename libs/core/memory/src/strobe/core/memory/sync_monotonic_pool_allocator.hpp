#pragma once
#include <atomic>
#include <cassert>
#include <ratio>
#include <algorithm>

#include "strobe/core/memory/AllocatorTraits.hpp"

namespace strobe {

template <std::size_t BlockSize, std::size_t BlockAlign, Allocator A,
          typename GrowthFactor = std::ratio<2, 1>>
class LockFreeMonotonicPoolResource {
 public:
  using Self = LockFreeMonotonicPoolResource<BlockSize, BlockAlign, A>;
  using upstream_allocator = A;
  using upstream_traits = AllocatorTraits<upstream_allocator>;
  static constexpr std::size_t block_size = std::max(
      std::max(BlockSize, BlockAlign), sizeof(void*) + sizeof(std::size_t));
  static constexpr std::size_t block_align = BlockAlign;

  explicit LockFreeMonotonicPoolResource(const A& upstream = {})
      : m_upstream(upstream), m_buffer(nullptr), m_freelist(nullptr) {}

  ~LockFreeMonotonicPoolResource() { release(); }

  LockFreeMonotonicPoolResource(const LockFreeMonotonicPoolResource&) = delete;
  LockFreeMonotonicPoolResource& operator=(
      const LockFreeMonotonicPoolResource&) = delete;

  LockFreeMonotonicPoolResource select_on_container_copy_construction() {
    return LockFreeMonotonicPoolResource(m_upstream);
  }

  LockFreeMonotonicPoolResource(LockFreeMonotonicPoolResource&& o)
      : m_upstream(std::move(o)),
        m_buffer(std::exchange(o.m_buffer)),
        m_freelist(std::exchange(m_freelist)) {}
  LockFreeMonotonicPoolResource& operator=(LockFreeMonotonicPoolResource&& o) {
    if (this == &o) {
      return *this;
    }

    static_assert(
        upstream_traits::propagate_on_container_move_assignment,
        "Require for upstream allocators, because otherwise would "
        "have to invalidated allocated pointers on move, which breaks the "
        "core requirement of a memory resource.");

    release();
    m_upstream = std::move(o.m_upstream);
    m_buffer = std::exchange(o.m_buffer, nullptr);
    m_freelist = std::exchange(o.m_freelist, nullptr);
    return *this;
  }

  void* allocate(std::size_t size, std::size_t align) {
    Node* freelistHead = m_freelist.load(std::memory_order_relaxed);

    while (freelistHead != nullptr) {
      Node* next = freelistHead->free.next;
      if (m_freelist.compare_exchange_weak(freelistHead, next,
                                           std::memory_order_acquire,
                                           std::memory_order_relaxed)) {
        return reinterpret_cast<void*>(&freelistHead->value);
      }
    }
    // freelistHead was nullptr,
    // grow and allocate at least one unique node, which is returned here.
    return reinterpret_cast<void*>(&allocateBlock()->value);
  }

  // generalized interface, but for pool allocation we already now the size.
  void deallocate(void* ptr, std::size_t size, std::size_t align) {
    assert(size <= block_size);
    // we can safely assume that all alignments are nice powers of 2.
    assert((BlockAlign % align) == 0);
    deallocate(ptr);
  }

  void deallocate(void* ptr) {
    assert(ptr != nullptr);
    Node* node = reinterpret_cast<Node*>(ptr);

    Node* freelist = m_freelist.load(std::memory_order_relaxed);

    do {
      node->free.next = freelist;
    } while (!m_freelist.compare_exchange_weak(
        freelist, node, std::memory_order_release, std::memory_order_relaxed));
  }

  bool operator==(const LockFreeMonotonicPoolResource& o) const noexcept {
    return this == &o;
  }

  bool operator!=(const LockFreeMonotonicPoolResource& o) const noexcept =
      default;

  bool owns(const void* p) const {
    Node* block = m_buffer.load(std::memory_order_acquire);
    while (block) {
      if (p >= &block->value[0] &&
          p < reinterpret_cast<void*>(block + block->block.blockSize)) {
        return true;
      }
      block = block->block.next;
    }
    return false;
  }

 private:
  union Node {
    struct {
      Node* next;
    } free;
    struct {
      Node* next;
      std::size_t blockSize;
    } block;
    alignas(block_align) std::byte value[block_size];
  };

  void release() {
    // TODO probably not thread safe.
  }

  void pushBlock(Node* header) {
    Node* head = m_buffer.load(std::memory_order_relaxed);
    do {
      header->block.next = head;
    } while (!m_buffer.compare_exchange_weak(
        head, header, std::memory_order_release, std::memory_order_relaxed));
  }

  // Allocates a new block and appends it to the freelist.
  // Returns a unique node, which is not part of the freelist.
  Node* allocateBlock() {
    std::size_t nextBlockSize;
    Node* buffer = m_buffer.load(std::memory_order_relaxed);
    if (buffer == nullptr) {
      // block size is +1 the capacity.
      nextBlockSize = 2;
    } else {
      nextBlockSize =
          (buffer->block.blockSize * GrowthFactor::num) / GrowthFactor::den;
    }
    Node* block =
        upstream_traits::template allocate<Node>(m_upstream, nextBlockSize);
    Node* header = block;

    // Initialize the free list in the remaining nodes
    Node* unique = header + 1;
    Node* begin = header + 2;
    Node* end = header + nextBlockSize;
    for (Node* current = begin; current < end - 1; ++current) {
      current->free.next = current + 1;
    }
    (end - 1)->free.next = nullptr;  // Last node in the free list

    // lock free prepend new freelist to m_freelist.
    Node* freelistHead = m_freelist.load();
    do {
      (end - 1)->free.next = freelistHead;
    } while (!m_freelist.compare_exchange_weak(freelistHead, begin));

    // lock free push of new block.
    block->block.blockSize = nextBlockSize;
    pushBlock(block);

    return unique;
  }

 private:
  [[no_unique_address]] upstream_allocator m_upstream;
  std::atomic<Node*> m_buffer;    // fwd list of allocated chunks
  std::atomic<Node*> m_freelist;  // freelist.
};

}  // namespace strobe
