#pragma once

#include "strobe/core/lina/vec.hpp"
#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief todo
 */
struct ClearDepthStencil {
  float depth;
  uint32_t stencil;
};

/**
 * \ingroup rhi
 * \brief todo
 */
union ClearValue {
  vec4 float4;
  uvec4 unsigned4;
  ClearDepthStencil depthStencil;

  constexpr ClearValue() noexcept : float4{} {}
  constexpr ClearValue(vec4 value) noexcept : float4(value) {}
  constexpr ClearValue(uvec4 value) noexcept : unsigned4(value) {}
  constexpr ClearValue(ClearDepthStencil value) noexcept
      : depthStencil(value) {}
};

} // namespace strobe::rhi
