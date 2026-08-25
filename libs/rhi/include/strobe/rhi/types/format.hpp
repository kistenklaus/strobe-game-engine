#pragma once

#include <cstdint>

namespace strobe::rhi {

enum class Format : uint16_t {
  undefined,

  r8_unorm,
  r8_snorm,
  r8_uint,
  r8_sint,

  rg8_unorm,
  rg8_snorm,
  rg8_uint,
  rg8_sint,

  rgba8_unorm,
  rgba8_srgb,
  rgba8_uint,
  rgba8_sint,

  bgra8_unorm,
  bgra8_srgb,

  r16_float,
  rg16_float,
  rgba16_float,

  r32_float,
  rg32_float,
  rgb32_float,
  rgba32_float,

  r32_uint,
  rg32_uint,
  rgba32_uint,

  r32_sint,
  rg32_sint,
  rgba32_sint,

  d16_unorm,
  d32_float,
  d24_unorm_s8_uint,
  d32_float_s8_uint,

  unsupported,
};

}
