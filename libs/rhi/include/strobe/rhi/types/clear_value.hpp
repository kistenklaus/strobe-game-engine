#pragma once

#include "strobe/core/lina/vec.hpp"
#include "strobe/rhi/types/clear_depth_stencil.hpp"

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Attachment clear value.
 *
 * Stores a color or depth-stencil value used when clearing an attachment.
 */
union ClearValue {
  /** Floating-point color clear value. */
  vec4 float4;

  /** Unsigned integer color clear value. */
  uvec4 unsigned4;

  /** Depth-stencil clear value. */
  ClearDepthStencil depthStencil;

  /**
   * \brief Constructs zero clear.
   *
   * Initializes the clear value as a zero floating-point color.
   */
  constexpr ClearValue() noexcept : float4{} {}

  /**
   * \brief Constructs float clear.
   *
   * \param value Floating-point color clear value.
   */
  constexpr ClearValue(vec4 value) noexcept : float4(value) {}

  /**
   * \brief Constructs integer clear.
   *
   * \param value Unsigned integer color clear value.
   */
  constexpr ClearValue(uvec4 value) noexcept : unsigned4(value) {}

  /**
   * \brief Constructs depth-stencil clear.
   *
   * \param value Depth-stencil clear value.
   */
  constexpr ClearValue(ClearDepthStencil value) noexcept
      : depthStencil(value) {}
};

} // namespace strobe::rhi
