#pragma once

#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Color blend factor.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{.cpp} blend_factor.hpp BlendFactor
 *
 * Specifies the factor applied to a source or destination color during
 * blending.
 */
// [BlendFactor]
enum class BlendFactor : uint8_t {
  zero,                    
  one,                     
  src_color,               
  one_minus_src_color,     
  dst_color,               
  one_minus_dst_color,     
  src_alpha,               
  one_minus_src_alpha,     
  dst_alpha,               
  one_minus_dst_alpha,     
  constant_color,          
  one_minus_constant_color,
  constant_alpha,          
  one_minus_constant_alpha,
  src_alpha_saturate,      
  src1_color,              
  one_minus_src1_color,    
  src1_alpha,              
  one_minus_src1_alpha,    
};
// [BlendFactor]

} // namespace strobe::rhi
