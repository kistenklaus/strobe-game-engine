#pragma once

#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Front face orientation.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{cpp} front_face.hpp FrontFace
 *
 * Specifies which polygon winding order is considered front-facing.
 */
// [FrontFace]
enum class FrontFace : uint8_t {
  counter_clockwise, 
  clockwise,         
};
// [FrontFace]

} // namespace strobe::rhi
