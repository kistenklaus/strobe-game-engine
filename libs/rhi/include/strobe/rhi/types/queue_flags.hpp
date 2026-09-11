#pragma once

#include <cstdint>
#include <type_traits>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Queue capability mask.
 *
 * Specifies required queue capabilities and selection constraints.
 * QueueFlags values may be combined and queried using the provided bitwise
 * operators.
 */
enum class QueueFlags : uint32_t {
  none = 0,             ///< No queue capabilities requested.
  graphics = 1u << 0,   ///< Graphics command support.
  transfer = 1u << 1,   ///< Transfer command support.
  compute = 1u << 2,    ///< Compute command support.
  present = 1u << 3,    ///< Presentation support.
  exclusive = 1u << 4,  ///< Require an exclusively selected queue.
};

[[nodiscard]]
constexpr QueueFlags operator|(QueueFlags lhs, QueueFlags rhs) noexcept {
  using T = std::underlying_type_t<QueueFlags>;

  return static_cast<QueueFlags>(static_cast<T>(lhs) | static_cast<T>(rhs));
}

[[nodiscard]]
constexpr QueueFlags operator&(QueueFlags lhs, QueueFlags rhs) noexcept {
  using T = std::underlying_type_t<QueueFlags>;

  return static_cast<QueueFlags>(static_cast<T>(lhs) & static_cast<T>(rhs));
}

[[nodiscard]]
constexpr QueueFlags operator^(QueueFlags lhs, QueueFlags rhs) noexcept {
  using T = std::underlying_type_t<QueueFlags>;

  return static_cast<QueueFlags>(static_cast<T>(lhs) ^ static_cast<T>(rhs));
}

[[nodiscard]]
constexpr QueueFlags operator~(QueueFlags value) noexcept {
  using T = std::underlying_type_t<QueueFlags>;

  return static_cast<QueueFlags>(~static_cast<T>(value));
}

constexpr QueueFlags &operator|=(QueueFlags &lhs, QueueFlags rhs) noexcept {
  lhs = lhs | rhs;
  return lhs;
}

constexpr QueueFlags &operator&=(QueueFlags &lhs, QueueFlags rhs) noexcept {
  lhs = lhs & rhs;
  return lhs;
}

constexpr QueueFlags &operator^=(QueueFlags &lhs, QueueFlags rhs) noexcept {
  lhs = lhs ^ rhs;
  return lhs;
}

[[nodiscard]]
constexpr bool operator==(QueueFlags lhs,
                          std::underlying_type_t<QueueFlags> rhs) noexcept {
  return static_cast<std::underlying_type_t<QueueFlags>>(lhs) == rhs;
}

[[nodiscard]]
constexpr bool operator==(std::underlying_type_t<QueueFlags> lhs,
                          QueueFlags rhs) noexcept {
  return lhs == static_cast<std::underlying_type_t<QueueFlags>>(rhs);
}

[[nodiscard]]
constexpr bool operator!=(QueueFlags lhs,
                          std::underlying_type_t<QueueFlags> rhs) noexcept {
  return !(lhs == rhs);
}

[[nodiscard]]
constexpr bool operator!=(std::underlying_type_t<QueueFlags> lhs,
                          QueueFlags rhs) noexcept {
  return !(lhs == rhs);
}

} // namespace strobe::rhi
