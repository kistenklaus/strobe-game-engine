#pragma once

#include <concepts>
#include <memory>
#include <strobe/core/memory/AllocatorTraits.hpp>
#include <utility>

#include "strobe/core/memory/ReferenceCounter.hpp"

namespace strobe {

namespace shared_block_ref::details {

template <typename T, Allocator A>
struct ControlBlock {
  T m_value;
  memory::ReferenceCounter<std::size_t> m_strongCounter;
  memory::ReferenceCounter<std::size_t> m_weakCounter;
  [[no_unique_address]] A m_allocator;

  template <typename... Args>
  explicit ControlBlock(A alloc, Args&&... args)
      : m_value(std::forward<Args>(args)...),
        m_strongCounter(),
        m_weakCounter(),
        m_allocator(std::move(alloc)) {}
};
}  // namespace shared_block_ref::details

template <typename T, Allocator A>
class WeakBlockRef;

template <typename T, Allocator A>
class SharedBlockRef {
 private:
  friend class WeakBlockRef<T, A>;
  using ATraits = AllocatorTraits<A>;
  using ControlBlock = shared_block_ref::details::ControlBlock<T, A>;

 public:
  SharedBlockRef() noexcept : m_controlBlock(nullptr) {}
  template <typename... Args>
  static SharedBlockRef make(A alloc, Args&&... args) {
    ControlBlock* controlBlock =
        ATraits::template allocate<ControlBlock>(alloc);
    new (controlBlock)
        ControlBlock(std::move(alloc), std::forward<Args>(args)...);
    return SharedBlockRef(controlBlock);
  }

  ~SharedBlockRef() { release(); }

  SharedBlockRef(const SharedBlockRef& o)
      : m_controlBlock(strongCopyControlBlock(o.m_controlBlock)) {}

  SharedBlockRef& operator=(const SharedBlockRef& o) {
    if (m_controlBlock == o.m_controlBlock) {
      return *this;
    }
    release();
    m_controlBlock = strongCopyControlBlock(o.m_controlBlock);
    return *this;
  }

  SharedBlockRef(SharedBlockRef&& o)
      : m_controlBlock(std::exchange(o.m_controlBlock, nullptr)) {}

  SharedBlockRef& operator=(SharedBlockRef&& o) {
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
    if (m_controlBlock != nullptr) {
      if (m_controlBlock->m_strongCounter.dec()) {
        std::destroy_at(&m_controlBlock->m_value);
        if (m_controlBlock->m_weakCounter.dec()) {
          A alloc = std::move(m_controlBlock->m_allocator);
          std::destroy_at(&m_controlBlock->m_strongCounter);
          std::destroy_at(&m_controlBlock->m_weakCounter);
          std::destroy_at(&m_controlBlock->m_allocator);
          ATraits::template deallocate<ControlBlock>(alloc, m_controlBlock);
        }
      }
      m_controlBlock = nullptr;
    }
  }

 private:
  static inline ControlBlock* strongCopyControlBlock(ControlBlock* o) {
    if (o != nullptr && o->m_strongCounter.inc()) {
      return o;
    } else {
      return nullptr;
    }
  }
  explicit SharedBlockRef(ControlBlock* controlBlock)
      : m_controlBlock(controlBlock) {
    assert(m_controlBlock != nullptr);
  }
  ControlBlock* m_controlBlock;
};

template <typename T, Allocator A>
class WeakBlockRef {
  using ControlBlock = shared_block_ref::details::ControlBlock<T, A>;
  using ATraits = AllocatorTraits<A>;

 public:
  WeakBlockRef(const SharedBlockRef<T, A>& sharedBlockRef)
      : m_controlBlock(weakCopyControlBlock(sharedBlockRef.m_controlBlock)) {}

  WeakBlockRef() noexcept : m_controlBlock(nullptr) {}
  ~WeakBlockRef() { release(); }

  WeakBlockRef(const WeakBlockRef& o)
      : m_controlBlock(weakCopyControlBlock(o.m_controlBlock)) {}
  WeakBlockRef& operator=(const WeakBlockRef& o) {
    if (this == &o) {
      return *this;
    }
    release();
    m_controlBlock = weakCopyControlBlock(o.m_controlBlock);
    return *this;
  }
  WeakBlockRef(WeakBlockRef&& o)
      : m_controlBlock(std::exchange(o.m_controlBlock, nullptr)) {}
  WeakBlockRef& operator=(WeakBlockRef&& o) {
    if (this == &o) {
      return *this;
    }
    release();
    m_controlBlock = std::exchange(o.m_controlBlock, nullptr);
    return *this;
  }

  bool expired() const {
    return m_controlBlock != nullptr &&
           m_controlBlock->m_strongCounter.isZero();
  }

  SharedBlockRef<T, A> lock() const {
    if (!m_controlBlock->m_strongCounter.inc()) {
      return SharedBlockRef<T, A>();
    }
    assert(!expired());
    return SharedBlockRef<T, A>(m_controlBlock);
  }

  void release() {
    if (m_controlBlock != nullptr && m_controlBlock->m_weakCounter.dec()) {
      A alloc = std::move(m_controlBlock->m_allocator);
      std::destroy_at(&m_controlBlock->m_strongCounter);
      std::destroy_at(&m_controlBlock->m_weakCounter);
      std::destroy_at(&m_controlBlock->m_allocator);
      ATraits::template deallocate<ControlBlock>(alloc, m_controlBlock);
    }
  }

 private:
  static inline ControlBlock* weakCopyControlBlock(ControlBlock* o) {
    if (o != nullptr && o->m_weakCounter.inc()) {
      return o;
    } else {
      return nullptr;
    }
  }
  ControlBlock* m_controlBlock;
};

template <typename T, Allocator A, typename... Args>
  requires std::constructible_from<T, Args...>
static SharedBlockRef<T, A> makeSharedBlockRef(A alloc, Args&&... args) {
  return SharedBlockRef<T, A>::template make<Args...>(
      std::move(alloc), std::forward<Args>(args)...);
}

}  // namespace strobe
