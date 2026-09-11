#pragma once

#include <cstdint>
#include <type_traits>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Buffer usage mask.
 *
 * Specifies the operations for which a buffer may be used. BufferUsage values
 * may be combined and queried using the provided bitwise operators.
 */
enum class BufferUsage : uint64_t {
  none = 0, ///< No buffer usage.

  transfer_src = 1ull << 0, ///< Transfer source.
  transfer_dst = 1ull << 1, ///< Transfer destination.

  uniform_texel = 1ull << 2, ///< Uniform texel buffer.
  storage_texel = 1ull << 3, ///< Storage texel buffer.

  uniform = 1ull << 4, ///< Uniform buffer.
  storage = 1ull << 5, ///< Storage buffer.

  index = 1ull << 6, ///< Index buffer.
  vertex = 1ull << 7, ///< Vertex buffer.
  indirect = 1ull << 8, ///< Indirect command buffer.

  shader_device_address = 1ull << 9, ///< Shader device address access.

  shader_binding_table = 1ull << 10, ///< Ray tracing shader binding table.
  acceleration_structure_build_input =
      1ull << 11, ///< Acceleration structure build input.
  acceleration_structure_storage =
      1ull << 12, ///< Acceleration structure storage.

  descriptor_heap = 1ull << 13, ///< Resource descriptor heap storage.
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
