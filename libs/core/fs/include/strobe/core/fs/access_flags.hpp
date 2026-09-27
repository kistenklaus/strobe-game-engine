#pragma once

#include <cstdint>
#include <type_traits>

namespace strobe::fs {

/**
 * \brief File opening and access options.
 * \ingroup core
 *
 * Values may be combined with the bitwise operators.
 * \snippet{.cpp} access_flags.hpp FileAccess
 */
// [FileAccess]
enum class FileAccess : std::uint32_t {
  none = 0,
  read = 1u << 0,
  write = 1u << 1,
  read_write = (1u << 0) | (1u << 1),
  create = 1u << 2,
  trunc = 1u << 3,
  append = 1u << 4,
  exclusive = 1u << 5,
  sync = 1u << 6,
};
// [FileAccess]

[[nodiscard]] constexpr FileAccess operator|(FileAccess lhs,
                                              FileAccess rhs) noexcept {
  using T = std::underlying_type_t<FileAccess>;
  return static_cast<FileAccess>(static_cast<T>(lhs) | static_cast<T>(rhs));
}

[[nodiscard]] constexpr FileAccess operator&(FileAccess lhs,
                                              FileAccess rhs) noexcept {
  using T = std::underlying_type_t<FileAccess>;
  return static_cast<FileAccess>(static_cast<T>(lhs) & static_cast<T>(rhs));
}

[[nodiscard]] constexpr FileAccess operator^(FileAccess lhs,
                                              FileAccess rhs) noexcept {
  using T = std::underlying_type_t<FileAccess>;
  return static_cast<FileAccess>(static_cast<T>(lhs) ^ static_cast<T>(rhs));
}

[[nodiscard]] constexpr FileAccess operator~(FileAccess value) noexcept {
  using T = std::underlying_type_t<FileAccess>;
  return static_cast<FileAccess>(~static_cast<T>(value));
}

constexpr FileAccess &operator|=(FileAccess &lhs, FileAccess rhs) noexcept {
  lhs = lhs | rhs;
  return lhs;
}

constexpr FileAccess &operator&=(FileAccess &lhs, FileAccess rhs) noexcept {
  lhs = lhs & rhs;
  return lhs;
}

constexpr FileAccess &operator^=(FileAccess &lhs, FileAccess rhs) noexcept {
  lhs = lhs ^ rhs;
  return lhs;
}

} // namespace strobe::fs
