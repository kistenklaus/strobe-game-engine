#pragma once

#include <cstdint>
#include <type_traits>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Pipeline stage mask.
 *
 * Specifies pipeline stages participating in synchronization operations.
 * PipelineStage values may be combined and queried using the provided bitwise
 * operators.
 */
enum class PipelineStage : uint64_t {
  none = 0, ///< No pipeline stages.

  indirect_command = 1ull << 0, ///< Indirect command processing.

  index_input = 1ull << 1, ///< Index input.
  vertex_attribute_input = 1ull << 2, ///< Vertex attribute input.

  vertex_shader = 1ull << 3, ///< Vertex shader execution.
  task_shader = 1ull << 4, ///< Task shader execution.
  mesh_shader = 1ull << 5, ///< Mesh shader execution.
  fragment_shader = 1ull << 6, ///< Fragment shader execution.
  compute_shader = 1ull << 7, ///< Compute shader execution.
  ray_tracing_shader = 1ull << 8, ///< Ray tracing shader execution.

  early_fragment_tests = 1ull << 9, ///< Early fragment tests.
  late_fragment_tests = 1ull << 10, ///< Late fragment tests.

  color_attachment_output = 1ull << 11, ///< Color attachment output.

  transfer = 1ull << 12, ///< Transfer operations.

  host = 1ull << 13, ///< Host operations.

  all_graphics = 1ull << 14, ///< All graphics pipeline stages.
  all_commands = 1ull << 15, ///< All device command stages.
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

} // namespace strobe::rhi
