#pragma once

#include "strobe/core/memory/AllocatorTraits.hpp"
#include "strobe/core/memory/ReferenceCounter.hpp"
#include "strobe/core/memory/align.hpp"

#include <algorithm>
#include <type_traits>
namespace strobe {

namespace memory::poly_shared_ptr::details {

struct ControlBlockHeader {
  typedef void (*Deleter)(void *self);
  memory::ReferenceCounter<std::size_t> m_referenceCounter;
  Deleter m_deleter;

  ControlBlockHeader(Deleter del) : m_deleter(del) {}
};

template <typename T> struct ControlBlock : ControlBlockHeader {
  T m_value;

  template <typename... Args>
  ControlBlock(Deleter del, Args &&...args)
      : ControlBlockHeader(del), m_value(std::forward<Args>(args)...) {}
};
} // namespace memory::poly_shared_ptr::details

template <typename T> class PolySharedPtr {
private:
  typedef void (*Deleter)(void *self);
  using ControlBlockHeader =
      memory::poly_shared_ptr::details::ControlBlockHeader;

public:
  template <Allocator A, typename... Args>
  static PolySharedPtr make(A alloc, Args &&...args) {

    using ControlBlock = memory::poly_shared_ptr::details::ControlBlock<T>;
    constexpr std::size_t headerSize = sizeof(ControlBlock);
    constexpr std::size_t allocatorOffset =
        memory::align_up(headerSize, alignof(A));
    constexpr std::size_t controlBlockSize = allocatorOffset + sizeof(A);
    constexpr std::size_t controlBlockAlign =
        std::max(alignof(A), alignof(ControlBlock));

    using ATraits = AllocatorTraits<A>;

    auto ptr = ATraits::allocate(alloc, controlBlockSize, controlBlockAlign);
    auto controlBlock = reinterpret_cast<ControlBlock *>(ptr);
    auto raw = reinterpret_cast<std::byte *>(ptr);
    auto allocPtr = reinterpret_cast<A *>(raw + allocatorOffset);
    new (allocPtr) A(std::move(alloc));

    new (controlBlock) ControlBlock(
        [](void *self) {
          constexpr std::size_t headerSize = sizeof(ControlBlock);
          constexpr std::size_t allocatorOffset =
              memory::align_up(headerSize, alignof(A));
          constexpr std::size_t controlBlockSize = allocatorOffset + sizeof(A);
          constexpr std::size_t controlBlockAlign =
              std::max(alignof(A), alignof(ControlBlock));
          auto controlBlock = reinterpret_cast<ControlBlock *>(self);
          auto raw = reinterpret_cast<std::byte *>(self);
          auto allocPtr = reinterpret_cast<A *>(raw + allocatorOffset);

          A alloc = std::move(*allocPtr);
          controlBlock.~ControlBlock();
          ATraits::deallocate(alloc, self, controlBlockSize, controlBlockAlign);
        },
        std::forward<Args>(args)...);

    return PolySharedPtr<T>(controlBlock, &controlBlock->m_value);
  }
  ~PolySharedPtr() { release(); }

  PolySharedPtr(const PolySharedPtr &o)
      : m_controlBlock(copyControlBlock(o.m_controlBlock)), m_ptr(o.m_ptr) {}

  PolySharedPtr &operator=(const PolySharedPtr &o) {
    if (m_controlBlock == o.m_controlBlock) {
      return *this;
    }
    release();
    m_controlBlock = copyControlBlock(o.m_controlBlock);
    m_ptr = o.m_ptr;
    return *this;
  }

  template <typename D>
    requires(std::is_base_of_v<T, D>)
  PolySharedPtr(const PolySharedPtr<D> &o)
      : m_controlBlock(copyControlBlock(o.m_controlBlock)),
        m_ptr(static_cast<T *>(o.m_ptr)) {}

  template <typename D>
    requires(std::is_base_of_v<T, D>)
  PolySharedPtr &operator=(const PolySharedPtr<D> &o) {
    if (m_controlBlock == o.m_controlBlock) {
      return *this;
    }
    release();
    m_controlBlock = copyControlBlock(o.m_controlBlock);
    m_ptr = static_cast<T *>(o.m_ptr);
    return *this;
  }

  PolySharedPtr(PolySharedPtr &&o)
      : m_controlBlock(std::exchange(o.m_controlBlock, nullptr)),
        m_ptr(std::exchange(o.m_ptr, nullptr)) {}

  PolySharedPtr &operator=(PolySharedPtr &&o) {
    if (m_controlBlock == o.m_controlBlock) {
      return *this;
    }
    release();
    m_controlBlock = std::exchange(o.m_controlBlock, nullptr);
    m_ptr = std::exchange(o.m_ptr, nullptr);
    return *this;
  }

  template <typename D>
    requires(std::is_base_of_v<T, D>)
  PolySharedPtr(PolySharedPtr<D> &&o)
      : m_controlBlock(std::exchange(o.m_controlBlock, nullptr)),
        m_ptr(static_cast<T*>(std::exchange(o.m_ptr, nullptr))) {}

  template <typename D>
    requires(std::is_base_of_v<T, D>)
  PolySharedPtr &operator=(PolySharedPtr<D> &&o) {
    if (m_controlBlock == o.m_controlBlock) {
      return *this;
    }
    release();
    m_controlBlock = std::exchange(o.m_controlBlock, nullptr);
    m_ptr = static_cast<T*>(std::exchange(o.m_ptr, nullptr));
    return *this;
  }

  T &operator*() const noexcept {
    assert(valid());
    return *m_ptr;
  }

  T *operator->() const noexcept {
    assert(valid());
    return m_ptr;
  }

  T *get() const noexcept {
    return m_controlBlock == nullptr ? nullptr : m_ptr;
  }

  operator bool() const noexcept { return m_controlBlock != nullptr; }

  bool valid() const noexcept { return m_controlBlock != nullptr; }

  void release() {
    if (m_controlBlock != nullptr && m_controlBlock->m_referenceCounter.dec()) {
      m_controlBlock->m_deleter(m_controlBlock);
      m_controlBlock = nullptr;
      m_ptr = nullptr;
    }
  }

private:
  static inline ControlBlockHeader *copyControlBlock(ControlBlockHeader *o) {
    if (o->m_referenceCounter.inc()) {
      return o;
    } else {
      return nullptr;
    }
  }

  explicit PolySharedPtr(ControlBlockHeader *controlBlock, T *ptr)
      : m_controlBlock(controlBlock), m_ptr(ptr) {}

  ControlBlockHeader *m_controlBlock;
  T *m_ptr;
};

template <typename T, Allocator A, typename... Args>
static PolySharedPtr<T> makeSharedPtr(A alloc, Args &&...args) {
  return PolySharedPtr<T>::template make<A, Args...>(std::move(alloc),
                                              std::forward<Args>(args)...);
}

} // namespace strobe
