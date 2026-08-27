#pragma once

#include <cassert>
#include <cstddef>
namespace strobe::memory {

inline constexpr std::size_t align_up(std::size_t offset,
                            std::size_t alignment) noexcept {
  return (offset + alignment - 1) & ~(alignment - 1);
}

inline constexpr std::size_t align_down(std::size_t offset,
                              std::size_t alignment) noexcept {
  return offset & ~(alignment - 1);
}

} // namespace strobe::memory
