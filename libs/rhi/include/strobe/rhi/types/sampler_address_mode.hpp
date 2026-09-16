#pragma once

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief sampler address mode
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{cpp} sampler_address_mode.hpp SamplerAddressMode
 */
// [SamplerAddressMode]
enum class SamplerAddressMode {
  repeat,
  mirrored_repeat,
  clamp_to_edge,
  clamp_to_border,
};
// [SamplerAddressMode]

}
