#pragma once

#include <cstdint>
#include <type_traits>

namespace strobe::rhi {

enum class Access : uint64_t {
  none = 0,
  indirect_command_read = 1ull << 0,
  index_read = 1ull << 1,
  vertex_attribute_read = 1ull << 2,
  uniform_read = 1ull << 3,
  shader_sampled_read = 1ull << 4,
  shader_storage_read = 1ull << 5,
  shader_storage_write = 1ull << 6,
  color_attachment_read = 1ull << 7,
  color_attachment_write = 1ull << 8,
  depth_stencil_attachment_read = 1ull << 9,
  depth_stencil_attachment_write = 1ull << 10,
  transfer_read = 1ull << 11,
  transfer_write = 1ull << 12,
  host_read = 1ull << 13,
  host_write = 1ull << 14,
  memory_read = 1ull << 15,
  memory_write = 1ull << 16,
};

[[nodiscard]] constexpr Access operator|(Access lhs, Access rhs) noexcept {
  return static_cast<Access>(static_cast<std::underlying_type_t<Access>>(lhs) |
                             static_cast<std::underlying_type_t<Access>>(rhs));
}

[[nodiscard]] constexpr Access operator&(Access lhs, Access rhs) noexcept {
  return static_cast<Access>(static_cast<std::underlying_type_t<Access>>(lhs) &
                             static_cast<std::underlying_type_t<Access>>(rhs));
}

[[nodiscard]] constexpr Access operator^(Access lhs, Access rhs) noexcept {
  return static_cast<Access>(static_cast<std::underlying_type_t<Access>>(lhs) ^
                             static_cast<std::underlying_type_t<Access>>(rhs));
}

[[nodiscard]] constexpr Access operator~(Access value) noexcept {
  return static_cast<Access>(
      ~static_cast<std::underlying_type_t<Access>>(value));
}

constexpr Access &operator|=(Access &lhs, Access rhs) noexcept {
  lhs = lhs | rhs;
  return lhs;
}

constexpr Access &operator&=(Access &lhs, Access rhs) noexcept {
  lhs = lhs & rhs;
  return lhs;
}

constexpr Access &operator^=(Access &lhs, Access rhs) noexcept {
  lhs = lhs ^ rhs;
  return lhs;
}

[[nodiscard]] constexpr bool
operator==(Access lhs, std::underlying_type_t<Access> rhs) noexcept {
  return static_cast<std::underlying_type_t<Access>>(lhs) == rhs;
}

[[nodiscard]] constexpr bool
operator!=(Access lhs, std::underlying_type_t<Access> rhs) noexcept {
  return static_cast<std::underlying_type_t<Access>>(lhs) != rhs;
}

[[nodiscard]] constexpr bool operator==(std::underlying_type_t<Access> lhs,
                                        Access rhs) noexcept {
  return lhs == static_cast<std::underlying_type_t<Access>>(rhs);
}

[[nodiscard]] constexpr bool operator!=(std::underlying_type_t<Access> lhs,
                                        Access rhs) noexcept {
  return lhs != static_cast<std::underlying_type_t<Access>>(rhs);
}

} // namespace strobe::rhi
