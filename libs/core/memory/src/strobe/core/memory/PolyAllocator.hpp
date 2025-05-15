#pragma once

#include <cstddef>

#include "strobe/core/memory/AllocatorReference.hpp"
#include "strobe/core/memory/AllocatorTraits.hpp"

namespace strobe {

class PolyMemoryResource {
 public:
  virtual ~PolyMemoryResource() = default;
  virtual void* allocate(std::size_t size, std::size_t align) = 0;
  virtual void deallocate(void* ptr, std::size_t size, std::size_t align) = 0;
};

template <Allocator A>
class MemoryResource : public PolyMemoryResource {
  using ATraits = AllocatorTraits<A>;

 public:
  MemoryResource(const A& alloc) : m_allocator(alloc) {}

  void* allocate(std::size_t size, std::size_t align) final override {
    return ATraits::allocate(m_allocator, size, align);
  }
  void deallocate(void* ptr, std::size_t size,
                  std::size_t align) final override {
    ATraits::deallocate(m_allocator, ptr, size, align);
  }

 private:
  [[no_unique_address]] A m_allocator;
};

using PolyAllocatorReference = AllocatorReference<PolyMemoryResource>;

}  // namespace strobe
