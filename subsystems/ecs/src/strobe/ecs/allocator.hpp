#pragma once

#include "strobe/core/memory/AllocatorReference.hpp"
#include "strobe/core/memory/Mallocator.hpp"
#include <atomic>
#include <fmt/printf.h>

namespace strobe::ecs {

struct EcsAllocator {
  void *allocate(std::size_t size, std::size_t align) {
    m_allocated += size;
    return strobe::Mallocator{}.allocate(size, align);
  }

  void deallocate(void *ptr, std::size_t size, std::size_t) {
    m_deallocated += size;
    strobe::Mallocator{}.deallocate(ptr);
  }

  size_t allocated() const { return m_allocated; }

  size_t deallocated() const { return m_deallocated; }

private:
  std::atomic<size_t> m_allocated;
  std::atomic<size_t> m_deallocated;
};

using allocator = EcsAllocator;
using allocator_ref = AllocatorReference<allocator>;

using event_queue_allocator = allocator_ref;

} // namespace strobe::ecs
