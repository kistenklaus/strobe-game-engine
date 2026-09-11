#pragma once

#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Image and buffer format.
 *
 * Specifies the data representation used by images, image views, and formatted
 * buffer accesses.
 */
enum class Format : uint16_t {
  undefined, ///< No format specified.

  r8_unorm, ///< Single-channel 8-bit unsigned normalized.
  r8_snorm, ///< Single-channel 8-bit signed normalized.
  r8_uint,  ///< Single-channel 8-bit unsigned integer.
  r8_sint,  ///< Single-channel 8-bit signed integer.

  rg8_unorm, ///< Two-channel 8-bit unsigned normalized.
  rg8_snorm, ///< Two-channel 8-bit signed normalized.
  rg8_uint,  ///< Two-channel 8-bit unsigned integer.
  rg8_sint,  ///< Two-channel 8-bit signed integer.

  rgba8_unorm, ///< Four-channel 8-bit unsigned normalized.
  rgba8_srgb,  ///< Four-channel 8-bit sRGB encoded.
  rgba8_uint,  ///< Four-channel 8-bit unsigned integer.
  rgba8_sint,  ///< Four-channel 8-bit signed integer.

  bgra8_unorm, ///< BGRA 8-bit unsigned normalized.
  bgra8_srgb,  ///< BGRA 8-bit sRGB encoded.

  r16_float,    ///< Single-channel 16-bit floating point.
  rg16_float,   ///< Two-channel 16-bit floating point.
  rgba16_float, ///< Four-channel 16-bit floating point.

  r32_float,    ///< Single-channel 32-bit floating point.
  rg32_float,   ///< Two-channel 32-bit floating point.
  rgb32_float,  ///< Three-channel 32-bit floating point.
  rgba32_float, ///< Four-channel 32-bit floating point.

  r32_uint,    ///< Single-channel 32-bit unsigned integer.
  rg32_uint,   ///< Two-channel 32-bit unsigned integer.
  rgba32_uint, ///< Four-channel 32-bit unsigned integer.

  r32_sint,    ///< Single-channel 32-bit signed integer.
  rg32_sint,   ///< Two-channel 32-bit signed integer.
  rgba32_sint, ///< Four-channel 32-bit signed integer.

  d16_unorm,         ///< 16-bit unsigned normalized depth.
  d32_float,         ///< 32-bit floating-point depth.
  d24_unorm_s8_uint, ///< 24-bit depth with 8-bit stencil.
  d32_float_s8_uint, ///< 32-bit floating-point depth with 8-bit stencil.

  unsupported, ///< Format is not supported by the RHI.
};

} // namespace strobe::rhi
