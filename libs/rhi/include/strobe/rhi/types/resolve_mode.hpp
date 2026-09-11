#pragma once

#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Attachment resolve mode.
 *
 * Specifies how multisampled attachment values are resolved into a
 * single-sampled attachment.
 */
enum class ResolveMode : uint8_t {
  none,        ///< Disable resolve operations.
  sample_zero, ///< Use sample zero as the resolved value.
  average,     ///< Average all samples.
  min,         ///< Use the minimum sample value.
  max,         ///< Use the maximum sample value.
};

} // namespace strobe::rhi
