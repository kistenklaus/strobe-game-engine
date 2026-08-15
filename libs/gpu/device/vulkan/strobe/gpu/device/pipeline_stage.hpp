#pragma once

#include <cstdint>
#include <type_traits>

namespace strobe::gpu {

enum class PipelineStage : uint64_t {
  none = 0,

  indirect_command = 1ull << 0,

  index_input = 1ull << 1,
  vertex_attribute_input = 1ull << 2,

  vertex_shader = 1ull << 3,
  task_shader = 1ull << 4,
  mesh_shader = 1ull << 5,
  fragment_shader = 1ull << 6,
  compute_shader = 1ull << 7,
  ray_tracing_shader = 1ull << 8,

  early_fragment_tests = 1ull << 9,
  late_fragment_tests = 1ull << 10,

  color_attachment_output = 1ull << 11,

  transfer = 1ull << 12,

  host = 1ull << 13,

  all_graphics = 1ull << 14,
  all_commands = 1ull << 15,
};

[[nodiscard]] constexpr PipelineStage
operator|(PipelineStage lhs, PipelineStage rhs) noexcept {
  return static_cast<PipelineStage>(
      static_cast<std::underlying_type_t<PipelineStage>>(lhs) |
      static_cast<std::underlying_type_t<PipelineStage>>(rhs));
}

[[nodiscard]] constexpr PipelineStage
operator&(PipelineStage lhs, PipelineStage rhs) noexcept {
  return static_cast<PipelineStage>(
      static_cast<std::underlying_type_t<PipelineStage>>(lhs) &
      static_cast<std::underlying_type_t<PipelineStage>>(rhs));
}

[[nodiscard]] constexpr PipelineStage
operator^(PipelineStage lhs, PipelineStage rhs) noexcept {
  return static_cast<PipelineStage>(
      static_cast<std::underlying_type_t<PipelineStage>>(lhs) ^
      static_cast<std::underlying_type_t<PipelineStage>>(rhs));
}

[[nodiscard]] constexpr PipelineStage
operator~(PipelineStage value) noexcept {
  return static_cast<PipelineStage>(
      ~static_cast<std::underlying_type_t<PipelineStage>>(value));
}

constexpr PipelineStage &
operator|=(PipelineStage &lhs, PipelineStage rhs) noexcept {
  lhs = lhs | rhs;
  return lhs;
}

constexpr PipelineStage &
operator&=(PipelineStage &lhs, PipelineStage rhs) noexcept {
  lhs = lhs & rhs;
  return lhs;
}

constexpr PipelineStage &
operator^=(PipelineStage &lhs, PipelineStage rhs) noexcept {
  lhs = lhs ^ rhs;
  return lhs;
}

[[nodiscard]] constexpr bool
operator==(PipelineStage lhs,
           std::underlying_type_t<PipelineStage> rhs) noexcept {
  return static_cast<std::underlying_type_t<PipelineStage>>(lhs) == rhs;
}

[[nodiscard]] constexpr bool
operator!=(PipelineStage lhs,
           std::underlying_type_t<PipelineStage> rhs) noexcept {
  return static_cast<std::underlying_type_t<PipelineStage>>(lhs) != rhs;
}

[[nodiscard]] constexpr bool
operator==(std::underlying_type_t<PipelineStage> lhs,
           PipelineStage rhs) noexcept {
  return lhs ==
         static_cast<std::underlying_type_t<PipelineStage>>(rhs);
}

[[nodiscard]] constexpr bool
operator!=(std::underlying_type_t<PipelineStage> lhs,
           PipelineStage rhs) noexcept {
  return lhs !=
         static_cast<std::underlying_type_t<PipelineStage>>(rhs);
}

} // namespace strobe::gpu
