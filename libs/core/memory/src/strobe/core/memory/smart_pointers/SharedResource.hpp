#pragma once

#include "strobe/core/memory/AllocatorTraits.hpp"
#include "strobe/core/memory/ReferenceCounter.hpp"
#include "strobe/core/memory/align.hpp"
namespace strobe {

namespace memory {

template <typename T>
class ObjectDestructor {
  void operator()(T& v) { v.~T(); }
};

}  // namespace memory

template <typename T>
class SharedResource {
 private:
  typedef void (*Deleter)(void* self);
  struct ControlBlock {
    memory::ReferenceCounter<std::size_t> m_referenceCounter;
    Deleter m_deleter;
    T* m_resource;

    ControlBlock(Deleter del, T* resource)
        : m_referenceCounter(), m_deleter(del), m_resource(resource) {}
  };

 public:
  template <Allocator A, typename D = memory::ObjectDestructor<T>>
  static SharedResource make(A alloc, T* resource, const D& destructor = {}) {
    constexpr std::size_t headerSize = sizeof(ControlBlock);
    constexpr std::size_t allocatorOffset =
        memory::align_up(headerSize, alignof(A));
    constexpr std::size_t destructorOffset =
        memory::align_up(allocatorOffset + sizeof(A), alignof(D));

    constexpr std::size_t controlBlockSize = destructorOffset + sizeof(A);
    constexpr std::size_t controlBlockAlign =
        std::max(alignof(A), std::max(alignof(ControlBlock), alignof(D)));

    using ATraits = AllocatorTraits<A>;

    auto ptr = ATraits::allocate(alloc, controlBlockSize, controlBlockAlign);
    auto controlBlock = reinterpret_cast<ControlBlock*>(ptr);
    auto raw = reinterpret_cast<std::byte*>(ptr);
    auto allocPtr = reinterpret_cast<A*>(raw + allocatorOffset);
    auto destructorPtr = reinterpret_cast<D*>(raw + destructorOffset);

    new (controlBlock) ControlBlock(
        [](void* self) {
          constexpr std::size_t headerSize = sizeof(ControlBlock);
          constexpr std::size_t allocatorOffset =
              memory::align_up(headerSize, alignof(A));
          constexpr std::size_t destructorOffset =
              memory::align_up(allocatorOffset + sizeof(A), alignof(D));

          constexpr std::size_t controlBlockSize = destructorOffset + sizeof(A);
          constexpr std::size_t controlBlockAlign =
              std::max(alignof(A), std::max(alignof(ControlBlock), alignof(D)));

          auto controlBlock = reinterpret_cast<ControlBlock*>(self);
          auto raw = reinterpret_cast<std::byte*>(self);
          auto allocPtr = reinterpret_cast<A*>(raw + allocatorOffset);
          auto destructorPtr = reinterpret_cast<D*>(raw + destructorOffset);

          (*destructorPtr)(*controlBlock->m_resource);  // call destructor.
          A alloc = std::move(*allocPtr);
          controlBlock.~ControlBlock();
          ATraits::deallocate(alloc, self, controlBlockSize, controlBlockAlign);
        },
        resource);

    new (allocPtr) A(std::move(alloc));
    new (destructorPtr) D(std::move(destructor));
  }

  ~SharedResource() { release(); }

  SharedResource(const SharedResource& o) {
    if (o.m_controlBlock != nullptr &&
        o.m_controlBlock->m_referenceCounter.inc()) {
      m_controlBlock = o.m_controlBlock;
      m_resource = o.m_resource;
    } else {
      m_controlBlock = nullptr;
      m_resource = nullptr;
    }
  }

  SharedResource& operator=(const SharedResource& o) {
    if (m_controlBlock == o.m_controlBlock) {
      return *this;
    }
    release();
    if (o.m_controlBlock != nullptr &&
        o.m_controlBlock->m_referenceCounter.inc()) {
      m_controlBlock = o.m_controlBlock;
      m_resource = o.m_resource;
    }
    return *this;
  }

  SharedResource(SharedResource&& o)
      : m_controlBlock(std::exchange(o.m_controlBlock, nullptr)),
        m_resource(std::exchange(o.m_resource, nullptr)) {}

  SharedResource& operator=(SharedResource&& o) {
    if (m_controlBlock == o.m_controlBlock) {
      return *this;
    }
    release();
    m_controlBlock = std::exchange(o.m_controlBlock, nullptr);
    m_resource = std::exchange(o.m_resource, nullptr);
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
      m_resource = nullptr;
    }
  }

 private:
  explicit SharedResource(ControlBlock* controlBlock)
      : m_controlBlock(controlBlock) {}

  ControlBlock* m_controlBlock;
  T* m_resource;
};

template <typename T, Allocator A,
          typename Destructor = memory::ObjectDestructor<T>>
static SharedResource<T> makeSharedResource(T* resource, A alloc,
                                            const Destructor& destructor = {}) {
  return SharedResource<T>::template make<A, Destructor>(alloc, resource,
                                                         destructor);
}

}  // namespace strobe
