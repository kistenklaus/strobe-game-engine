#pragma once

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Sampler mipmap mode.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{cpp} sampler_mipmap_mode.hpp SamplerMipmapMode
 *
 */
// [SamplerMipmapMode]
enum class SamplerMipmapMode {
  nearest,
  linear,
};
// [SamplerMipmapMode]

} // namespace strobe::rhi
