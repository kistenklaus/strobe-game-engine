#pragma once

#include <cstdint>
#include <type_traits>

namespace strobe::rhi {

enum class BufferUsage : uint64_t {
  none = 0,

  transfer_src = 1ull << 0,
  transfer_dst = 1ull << 1,

  uniform_texel = 1ull << 2,
  storage_texel = 1ull << 3,

  uniform = 1ull << 4,
  storage = 1ull << 5,

  index = 1ull << 6,
  vertex = 1ull << 7,
  indirect = 1ull << 8,

  shader_device_address = 1ull << 9,

  shader_binding_table = 1ull << 10,
  acceleration_structure_build_input = 1ull << 11,
  acceleration_structure_storage = 1ull << 12,

  descriptor_heap = 1ull << 13,
};

[[nodiscard]] constexpr BufferUsage
operator|(BufferUsage lhs, BufferUsage rhs) noexcept {
  return static_cast<BufferUsage>(
      static_cast<std::underlying_type_t<BufferUsage>>(lhs) |
      static_cast<std::underlying_type_t<BufferUsage>>(rhs));
}

[[nodiscard]] constexpr BufferUsage
operator&(BufferUsage lhs, BufferUsage rhs) noexcept {
  return static_cast<BufferUsage>(
      static_cast<std::underlying_type_t<BufferUsage>>(lhs) &
      static_cast<std::underlying_type_t<BufferUsage>>(rhs));
}

[[nodiscard]] constexpr BufferUsage
operator^(BufferUsage lhs, BufferUsage rhs) noexcept {
  return static_cast<BufferUsage>(
      static_cast<std::underlying_type_t<BufferUsage>>(lhs) ^
      static_cast<std::underlying_type_t<BufferUsage>>(rhs));
}

[[nodiscard]] constexpr BufferUsage
operator~(BufferUsage value) noexcept {
  return static_cast<BufferUsage>(
      ~static_cast<std::underlying_type_t<BufferUsage>>(value));
}

constexpr BufferUsage &
operator|=(BufferUsage &lhs, BufferUsage rhs) noexcept {
  lhs = lhs | rhs;
  return lhs;
}

constexpr BufferUsage &
operator&=(BufferUsage &lhs, BufferUsage rhs) noexcept {
  lhs = lhs & rhs;
  return lhs;
}

constexpr BufferUsage &
operator^=(BufferUsage &lhs, BufferUsage rhs) noexcept {
  lhs = lhs ^ rhs;
  return lhs;
}

[[nodiscard]] constexpr bool
operator==(BufferUsage lhs,
           std::underlying_type_t<BufferUsage> rhs) noexcept {
  return static_cast<std::underlying_type_t<BufferUsage>>(lhs) == rhs;
}

[[nodiscard]] constexpr bool
operator!=(BufferUsage lhs,
           std::underlying_type_t<BufferUsage> rhs) noexcept {
  return static_cast<std::underlying_type_t<BufferUsage>>(lhs) != rhs;
}

[[nodiscard]] constexpr bool
operator==(std::underlying_type_t<BufferUsage> lhs,
           BufferUsage rhs) noexcept {
  return lhs == static_cast<std::underlying_type_t<BufferUsage>>(rhs);
}

[[nodiscard]] constexpr bool
operator!=(std::underlying_type_t<BufferUsage> lhs,
           BufferUsage rhs) noexcept {
  return lhs != static_cast<std::underlying_type_t<BufferUsage>>(rhs);
}

} // namespace strobe::rhi
