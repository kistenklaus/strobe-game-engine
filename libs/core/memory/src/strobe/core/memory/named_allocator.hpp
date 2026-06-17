#pragma once

#include "strobe/core/memory/AllocatorTraits.hpp"
#include "strobe/core/type_traits/fixed_string.hpp"
#include <fmt/printf.h>
#include <tracy/Tracy.hpp>
#include <type_traits>

namespace strobe {

template <Allocator Alloc, fixed_string Name> class NamedAllocator {
public:
  using allocator = std::remove_cvref_t<Alloc>;
  using allocator_traits = AllocatorTraits<allocator>;
  static constexpr auto name = Name;

  explicit NamedAllocator(const allocator &alloc = {}) : m_upstream(alloc) {}

  void *allocate(size_t size, size_t alignment) {
    void *ptr = allocator_traits::allocate(m_upstream, size, alignment);
    assert(ptr != nullptr);
    TracyAllocN(ptr, size, name.data());
    return ptr;
  }

  void deallocate(void *ptr, size_t size, size_t alignment) {
    assert(ptr != nullptr);
    TracyFreeN(ptr, name.data());
    allocator_traits::deallocate(m_upstream, ptr, size, alignment);
  }

private:
  [[no_unique_address]] allocator m_upstream;
};

} // namespace strobe
