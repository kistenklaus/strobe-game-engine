#pragma once

#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Multisample sample count.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{cpp} sample_count.hpp SampleCount
 *
 * Specifies the number of samples used for multisampled rendering and images.
 */
// [SampleCount]
enum class SampleCount : uint8_t {
  x1  = 1,  
  x2  = 2,  
  x4  = 4,  
  x8  = 8,  
  x16 = 16, 
  x32 = 32, 
  x64 = 64, 
};
// [SampleCount]

} // namespace strobe::rhi
