#pragma once

#include <strobe/core/memory/AllocatorTraits.hpp>

#include "strobe/core/memory/ReferenceCounter.hpp"

namespace strobe {

template <typename T, Allocator A>
class SharedBlock {
 private:
  using ATraits = AllocatorTraits<A>;
  struct ControlBlock {
    T m_value;
    memory::ReferenceCounter<std::size_t> m_referenceCounter;
    [[no_unique_address]] A m_allocator;

    template <typename... Args>
    explicit ControlBlock(A alloc, Args&&... args)
        : m_value(std::forward<Args>(args)...),
          m_referenceCounter(),
          m_allocator(std::move(alloc)) {}
  };

 public:
  template <typename... Args>
  SharedBlock make(A alloc, Args&&... args) {
    ControlBlock* controlBlock =
        ATraits::template allocate<ControlBlock>(alloc);
    new (controlBlock)
        ControlBlock(std::move(alloc), std::forward<Args>(args)...);
    return SharedBlock(controlBlock);
  }

  ~SharedBlock() { release(); }

  SharedBlock(const SharedBlock& o)
      : m_controlBlock(copyControlBlock(o.m_controlBlock)) {}

  SharedBlock& operator=(const SharedBlock& o) {
    if (m_controlBlock == o.m_controlBlock) {
      return *this;
    }
    release();
    m_controlBlock = copyControlBlock(o.m_controlBlock);
    return *this;
  }

  SharedBlock(SharedBlock&& o)
      : m_controlBlock(std::exchange(o.m_controlBlock, nullptr)) {}

  SharedBlock& operator=(SharedBlock&& o) {
    if (m_controlBlock == o.m_controlBlock) {
      return *this;
    }
    release();
    m_controlBlock = std::exchange(o.m_controlBlock, nullptr);
    return *this;
  }

  T& operator*() const noexcept {
    assert(m_controlBlock);
    return m_controlBlock->m_value;
  }

  T* operator->() const noexcept {
    assert(m_controlBlock);
    return &m_controlBlock->m_value;
  }

  T* get() const noexcept {
    return m_controlBlock == nullptr ? nullptr : &m_controlBlock->m_value;
  }

  operator bool() const noexcept { return m_controlBlock != nullptr; }

  bool valid() const noexcept { return m_controlBlock != nullptr; }

  void release() {
    if (m_controlBlock != nullptr && m_controlBlock->m_referenceCounter.dec()) {
      A alloc = m_controlBlock->m_allocator;
      m_controlBlock.~ControlBlock();
      ATraits::template deallocate<ControlBlock>(m_controlBlock);
      m_controlBlock = nullptr;
    }
  }

 private:
  static inline ControlBlock* copyControlBlock(ControlBlock* o) {
    if (o->m_referenceCounter.inc()) {
      return o;
    } else {
      return nullptr;
    }
  }
  explicit SharedBlock(ControlBlock* controlBlock)
      : m_controlBlock(controlBlock) {
    assert(m_controlBlock != nullptr);
  }
  ControlBlock* m_controlBlock;
};

template <typename T, Allocator A, typename... Args>
static SharedBlock<T, A> makeSharedBlock(A alloc, Args&&... args) {
  return SharedBlock<T, A>::template make<Args...>(std::move(alloc),
                                                   std::forward<Args>(args)...);
}

}  // namespace strobe
