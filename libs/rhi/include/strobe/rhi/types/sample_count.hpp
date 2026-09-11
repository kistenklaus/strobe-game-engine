#pragma once

#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Multisample sample count.
 *
 * Specifies the number of samples used for multisampled rendering and images.
 */
enum class SampleCount : uint8_t {
  x1  = 1,  ///< One sample.
  x2  = 2,  ///< Two samples.
  x4  = 4,  ///< Four samples.
  x8  = 8,  ///< Eight samples.
  x16 = 16, ///< Sixteen samples.
  x32 = 32, ///< Thirty-two samples.
  x64 = 64, ///< Sixty-four samples.
};

} // namespace strobe::rhi
