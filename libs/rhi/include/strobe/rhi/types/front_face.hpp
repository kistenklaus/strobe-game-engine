#pragma once

#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Front face orientation.
 *
 * Specifies which polygon winding order is considered front-facing.
 */
enum class FrontFace : uint8_t {
  counter_clockwise, ///< Counter-clockwise winding is front-facing.
  clockwise,         ///< Clockwise winding is front-facing.
};

} // namespace strobe::rhi
