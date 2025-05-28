#pragma once

#include "strobe/core/memory/AllocatorTraits.hpp"
#include "strobe/core/memory/ReferenceCounter.hpp"
#include "strobe/core/memory/align.hpp"

#include <algorithm>
namespace strobe {

template <typename T>
class SharedPtr {
 private:
  typedef void (*Deleter)(void* self);
  struct ControlBlock {
    T m_value;
    memory::ReferenceCounter<std::size_t> m_referenceCounter;
    Deleter m_deleter;

    template <typename... Args>
    ControlBlock(Deleter del, Args&&... args)
        : m_value(std::forward<Args>(args)...),
          m_referenceCounter(),
          m_deleter(del) {}
  };

 public:
  template <Allocator A, typename... Args>
  static SharedPtr make(A alloc, Args&&... args) {
    constexpr std::size_t headerSize = sizeof(ControlBlock);
    constexpr std::size_t allocatorOffset =
        memory::align_up(headerSize, alignof(A));
    constexpr std::size_t controlBlockSize = allocatorOffset + sizeof(A);
    constexpr std::size_t controlBlockAlign =
        std::max(alignof(A), alignof(ControlBlock));

    using ATraits = AllocatorTraits<A>;

    auto ptr = ATraits::allocate(alloc, controlBlockSize, controlBlockAlign);
    auto controlBlock = reinterpret_cast<ControlBlock*>(ptr);
    auto raw = reinterpret_cast<std::byte*>(ptr);
    auto allocPtr = reinterpret_cast<A*>(raw + allocatorOffset);
    new (allocPtr) A(std::move(alloc));

    new (controlBlock) ControlBlock(
        [](void* self) {
          constexpr std::size_t headerSize = sizeof(ControlBlock);
          constexpr std::size_t allocatorOffset =
              memory::align_up(headerSize, alignof(A));
          constexpr std::size_t controlBlockSize = allocatorOffset + sizeof(A);
          constexpr std::size_t controlBlockAlign =
              std::max(alignof(A), alignof(ControlBlock));
          auto controlBlock = reinterpret_cast<ControlBlock*>(self);
          auto raw = reinterpret_cast<std::byte*>(self);
          auto allocPtr = reinterpret_cast<A*>(raw + allocatorOffset);

          A alloc = std::move(*allocPtr);
          controlBlock.~ControlBlock();
          ATraits::deallocate(alloc, self, controlBlockSize, controlBlockAlign);
        },
        std::forward<Args>(args)...);
  }
  ~SharedPtr() { release(); }

  SharedPtr(const SharedPtr& o)
      : m_controlBlock(copyControlBlock(o.m_controlBlock)) {}
  SharedPtr& operator=(const SharedPtr& o) {
    if (m_controlBlock == o.m_controlBlock) {
      return *this;
    }
    release();
    m_controlBlock = copyControlBlock(o.m_controlBlock);
    return *this;
  }

  SharedPtr(SharedPtr&& o)
      : m_controlBlock(std::exchange(o.m_controlBlock, nullptr)) {}

  SharedPtr& operator=(SharedPtr&& o) {
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
      m_controlBlock->m_deleter(m_controlBlock);
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

  explicit SharedPtr(ControlBlock* controlBlock)
      : m_controlBlock(controlBlock) {}

  ControlBlock* m_controlBlock;
};

template <typename T, Allocator A, typename... Args>
static SharedPtr<T> makeSharedPtr(A alloc, Args&&... args) {
  SharedPtr<T>::template make<A, Args...>(std::move(alloc),
                                          std::forward<Args>(args)...);
}

}  // namespace strobe
