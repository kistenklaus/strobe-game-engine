#pragma once
#include <cassert>
#include <ratio>

#include "strobe/core/memory/AllocatorTraits.hpp"

namespace strobe {

template <std::size_t BlockSize, std::size_t BlockAlign, Allocator A,
          typename GrowthFactor = std::ratio<2, 1>>
class MonotonicPoolResource {
 public:
  using Self = MonotonicPoolResource<BlockSize, BlockAlign, A>;
  using upstream_allocator = A;
  using upstream_traits = AllocatorTraits<upstream_allocator>;
  static constexpr std::size_t block_size = std::max(BlockSize, BlockAlign);
  static constexpr std::size_t block_align = BlockAlign;

  explicit MonotonicPoolResource(const A& upstream = {})
      : m_upstream(upstream), m_buffer(nullptr), m_freelist(nullptr) {}

  ~MonotonicPoolResource() { release(); }

  MonotonicPoolResource(const MonotonicPoolResource&) = delete;
  MonotonicPoolResource& operator=(const MonotonicPoolResource&) = delete;

  MonotonicPoolResource select_on_container_copy_construction() {
    return MonotonicPoolResource(m_upstream);
  }

  MonotonicPoolResource(MonotonicPoolResource&& o)
      : m_upstream(std::move(o)),
        m_buffer(std::exchange(o.m_buffer)),
        m_freelist(std::exchange(m_freelist)) {}
  MonotonicPoolResource& operator=(MonotonicPoolResource&& o) {
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
    assert(size != 0);
    assert(size <= block_size);
    assert((BlockAlign % align) == 0);
    if (m_freelist == nullptr) {
      grow();
    }
    return popFreelist();
  }

  // generalized interface, but for pool allocation we already now the size.
  void deallocate(void* ptr, std::size_t size, std::size_t align) {
    assert(size <= block_size);
    // we can safely assume that all alignments are nice powers of 2.
    assert((BlockAlign % align) == 0);
    deallocate(ptr);
  }

  void deallocate(void* ptr) { pushFreelist(ptr); }

  bool operator==(const MonotonicPoolResource& o) const noexcept {
    return m_buffer == o.m_buffer && m_freelist == o.m_freelist;
  }

  bool operator!=(const MonotonicPoolResource& o) const noexcept = default;

  bool owns(const void* p) const {
    Node* n = reinterpret_cast<Node*>(p);
    Node* block = m_buffer;
    while (block != nullptr) {
      if (block + 1 <= n && n < block + block->block.blockSize) {
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
    alignas(BlockAlign) std::byte value[block_size];
  };

  void release() {
    Node* block = m_buffer;
    while (block != nullptr) {
      Node* nextBlock = block->block.next;
      // Deallocate the entire block
      upstream_traits::template deallocate<Node>(m_upstream, block,
                                                 block->block.blockSize);
      block = nextBlock;
    }
    // Clear all state
    m_buffer = nullptr;
    m_freelist = nullptr;
  }

  void* popFreelist() {
    void* ptr = reinterpret_cast<void*>(m_freelist);
    m_freelist = m_freelist->free.next;
    return ptr;
  }

  void pushFreelist(void* ptr) {
    Node* node = reinterpret_cast<Node*>(ptr);
    node->free.next = m_freelist;
    m_freelist = node;
  }

  void grow() {
    std::size_t nextBlockSize;
    if (m_buffer == nullptr) {
      // block size is +1 the capacity.
      nextBlockSize = 2;
    } else {
      nextBlockSize =
          (m_buffer->block.blockSize * GrowthFactor::num) / GrowthFactor::den;
    }
    Node* block =
        upstream_traits::template allocate<Node>(m_upstream, nextBlockSize);
    Node* header = block;

    // Initialize the free list in the remaining nodes
    Node* const begin = header + 1;
    Node* const end = header + nextBlockSize;
    for (Node* current = begin; current < end - 1; ++current) {
      current->free.next = current + 1;
    }
    (end - 1)->free.next = nullptr;  // Last node in the free list

    (end - 1)->free.next = m_freelist;
    m_freelist = begin;

    block->block.blockSize = nextBlockSize;
    block->block.next = m_buffer;
    m_buffer = block;
  }

 private:
  [[no_unique_address]] upstream_allocator m_upstream;
  Node* m_buffer;    // fwd list of allocated chunks
  Node* m_freelist;  // freelist.
};

}  // namespace strobe
