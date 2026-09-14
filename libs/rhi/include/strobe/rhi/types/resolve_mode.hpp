#pragma once

#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Attachment resolve mode.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{cpp} resolve_mode.hpp ResolveMode
 *
 * Specifies how multisampled attachment values are resolved into a
 * single-sampled attachment.
 */
// [ResolveMode]
enum class ResolveMode : uint8_t {
  none,
  sample_zero,
  average,
  min,
  max,
};
// [ResolveMode]

} // namespace strobe::rhi
