#pragma once

#include <atomic>
#include <concepts>
#include <cstddef>
#include <memory>
#include <strobe/core/memory/AllocatorTraits.hpp>
#include <utility>

#include "strobe/core/memory/ReferenceCounter.hpp"

namespace strobe {

namespace block::details {

template <typename T, Allocator A>
struct ControlBlock {
  T m_value;
  memory::ReferenceCounter<std::size_t> m_referenceCounter;
  [[no_unique_address]] A m_allocator;

  template <typename... Args>
  explicit ControlBlock(A alloc, Args&&... args)
      : m_value(std::forward<Args>(args)...),
        m_referenceCounter(std::size_t(0)),
        m_allocator(std::move(alloc)) {}
};
}  // namespace block::details

template <typename T, Allocator A>
class Block;

template <typename T, Allocator A>
class BlockRef {
  using ControlBlock = block::details::ControlBlock<T, A>;
  using ATraits = AllocatorTraits<A>;

 public:
  friend class Block<T, A>;

  BlockRef() noexcept : m_controlBlock(nullptr) {}
  ~BlockRef() { release(); }

  BlockRef(const BlockRef& o)
      : m_controlBlock(copyControlBlock(o.m_controlBlock)) {}
  BlockRef& operator=(const BlockRef& o) {
    if (this == &o) {
      return *this;
    }
    release();
    m_controlBlock = copyControlBlock(o.m_controlBlock);
    return *this;
  }
  BlockRef(BlockRef&& o)
      : m_controlBlock(std::exchange(o.m_controlBlock, nullptr)) {}
  BlockRef& operator=(BlockRef&& o) {
    if (this == &o) {
      return *this;
    }
    release();
    m_controlBlock = std::exchange(o.m_controlBlock, nullptr);
    return *this;
  }

  T& operator*() const noexcept {
    assert(valid());
    return m_controlBlock->m_value;
  }

  T* operator->() const noexcept {
    assert(valid());
    return &m_controlBlock->m_value;
  }

  T* get() const noexcept {
    return m_controlBlock == nullptr ? nullptr : &m_controlBlock->m_value;
  }

  operator bool() const noexcept { return valid(); }

  bool valid() const { return m_controlBlock != nullptr; }

  void release() {
    if (m_controlBlock != nullptr) {
      m_controlBlock->m_referenceCounter.dec();
      m_controlBlock = nullptr;
    }
  }

 private:
  BlockRef(ControlBlock* controlBlock) : m_controlBlock(controlBlock) {}

  static inline ControlBlock* copyControlBlock(ControlBlock* o) {
    if (o != nullptr && o->m_referenceCounter.inc()) {
      return o;
    } else {
      return nullptr;
    }
  }
  ControlBlock* m_controlBlock;
};

template <typename T, Allocator A>
class Block {
 private:
  friend class BlockRef<T, A>;
  using ATraits = AllocatorTraits<A>;
  using ControlBlock = block::details::ControlBlock<T, A>;

 public:
  using Ref = BlockRef<T, A>;

  Block() noexcept : m_controlBlock(nullptr) {}

  template <typename... Args>
  static Block make(A alloc, Args&&... args) {
    ControlBlock* controlBlock =
        ATraits::template allocate<ControlBlock>(alloc);
    new (controlBlock)
        ControlBlock(std::move(alloc), std::forward<Args>(args)...);
    return Block(controlBlock);
  }

  ~Block() { release(); }

  Block(const Block& o) = delete;
  Block& operator=(const Block& o) = delete;

  Block(Block&& o) : m_controlBlock(std::exchange(o.m_controlBlock, nullptr)) {}

  Block& operator=(Block&& o) {
    if (m_controlBlock == o.m_controlBlock) {
      return *this;
    }
    release();
    m_controlBlock = std::exchange(o.m_controlBlock, nullptr);
    return *this;
  }

  T& operator*() const noexcept {
    assert(valid());
    return m_controlBlock->m_value;
  }

  T* operator->() const noexcept {
    assert(valid());
    return &m_controlBlock->m_value;
  }

  T* get() const noexcept {
    return m_controlBlock == nullptr ? nullptr : &m_controlBlock->m_value;
  }

  operator bool() const noexcept { return valid(); }

  bool isReferenced() const {
    assert(valid());
    return !m_controlBlock->m_referenceCounter.isZero();
  }

  Ref ref() const {
    if (valid()) {
      m_controlBlock->m_referenceCounter.resurrect();
      return Ref(m_controlBlock);
    } else {
      return Ref();
    }
  }

  bool valid() const noexcept { return m_controlBlock != nullptr; }

  void release() {
    if (m_controlBlock != nullptr) {
      assert(!isReferenced());
      A alloc = std::move(m_controlBlock->m_allocator);
      std::destroy_at(m_controlBlock);
      ATraits::template deallocate<ControlBlock>(alloc, m_controlBlock);
      m_controlBlock = nullptr;
    }
  }

 private:
  explicit Block(ControlBlock* controlBlock) : m_controlBlock(controlBlock) {
    assert(m_controlBlock != nullptr);
  }
  ControlBlock* m_controlBlock;
};

template <typename T, Allocator A, typename... Args>
  requires std::constructible_from<T, Args...>
static Block<T, A> makeBlock(A alloc, Args&&... args) {
  return Block<T, A>::template make<Args...>(std::move(alloc),
                                             std::forward<Args>(args)...);
}

}  // namespace strobe
