#pragma once

#include "strobe/core/memory/AllocatorTraits.hpp"
#include <algorithm>
#include <atomic>
#include <cassert>
#include <cstddef>
#include <type_traits>
namespace strobe {

template <std::size_t BlockSize, std::size_t BlockAlign, std::size_t BlockCount,
          Allocator Upstream>
class LockFreeInplacePoolAllocator {
private:
  static constexpr std::size_t block_size = std::max(BlockSize, BlockAlign);
  static constexpr std::size_t block_align = BlockAlign;
  using upstream = Upstream;
  using upstream_traits = AllocatorTraits<upstream>;

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
  static_assert(std::is_standard_layout_v<Node>,
                "Node must be standard layout.");

public:
  LockFreeInplacePoolAllocator() {
    m_memory[BlockCount - 1].free.next = nullptr;
    for (std::size_t i = BlockCount - 1; i > 0; --i) {
      m_memory[i - 1].free.next = m_memory + i;
    }
    m_freelist.store(m_memory, std::memory_order_release);
  }

  LockFreeInplacePoolAllocator(const LockFreeInplacePoolAllocator &o) = delete;
  LockFreeInplacePoolAllocator &
  operator=(const LockFreeInplacePoolAllocator &o) = delete;
  LockFreeInplacePoolAllocator(LockFreeInplacePoolAllocator &&o) = delete;
  LockFreeInplacePoolAllocator &
  operator=(LockFreeInplacePoolAllocator &&o) = delete;

  void *allocate() {
    Node *freelistHead = m_freelist.load(std::memory_order_relaxed);

    while (freelistHead != nullptr) {
      Node *next = freelistHead->free.next;
      if (m_freelist.compare_exchange_weak(freelistHead, next,
                                           std::memory_order_acquire,
                                           std::memory_order_relaxed)) {
        return reinterpret_cast<void *>(&freelistHead->value);
      }
    }
    return nullptr;
  }

  void *allocate(std::size_t size, std::size_t align) {
    assert(size <= BlockSize);
    assert(size <= block_size);
    assert(align <= BlockAlign);
    assert(BlockAlign % align == 0);
    assert(block_align % align == 0);
    return allocate();
  }

  void deallocate(void *p, std::size_t size, std::size_t align) {
    assert(size <= BlockSize);
    assert(size <= block_size);
    assert(align <= BlockAlign);
    assert(BlockAlign % align == 0);
    assert(block_align % align == 0);
    deallocate(p);
  }

  void deallocate(void *ptr) {
    assert(ptr != nullptr);
    Node *node = reinterpret_cast<Node *>(reinterpret_cast<std::byte *>(ptr) -
                                          offsetof(Node, value));

    Node *freelist = m_freelist.load(std::memory_order_relaxed);

    do {
      node->free.next = freelist;
    } while (!m_freelist.compare_exchange_weak(
        freelist, node, std::memory_order_release, std::memory_order_relaxed));
  }

  bool operator==(const LockFreeInplacePoolAllocator &o) const noexcept {
    return this == &o;
  }

  bool
  operator!=(const LockFreeInplacePoolAllocator &o) const noexcept = default;

  bool owns(const void *p) const {
    const Node *nodePtr = reinterpret_cast<const Node *>(
        reinterpret_cast<const std::byte *>(p) - offsetof(Node, value));
    return nodePtr >= m_memory && nodePtr < (m_memory + BlockCount);
  }

private:
  [[no_unique_address]] upstream m_upstream;
  Node m_memory[BlockCount];
  std::atomic<Node *> m_freelist;
};
} // namespace strobe
