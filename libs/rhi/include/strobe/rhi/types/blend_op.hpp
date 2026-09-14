#pragma once

#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Color blend operation.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{.cpp} blend_op.hpp BlendOp
 *
 * Specifies how source and destination blend terms are combined.
 */
// [BlendOp] 
enum class BlendOp : uint8_t {
  add,              
  subtract,         
  reverse_subtract, 
  min,              
  max,              
};
// [BlendOp] 

} // namespace strobe::rhi
