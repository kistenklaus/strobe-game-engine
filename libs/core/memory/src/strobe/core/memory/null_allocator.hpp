#pragma once

#include <cstddef>

namespace strobe {

struct NullAllocator {
  static constexpr bool is_always_equals = true;
  void *allocate(size_t, size_t) noexcept { return nullptr; }
  void deallocate(void *, std::size_t, std::size_t) noexcept {}
  void deallocate(void *) noexcept {}
};

} // namespace strobe::rhi
