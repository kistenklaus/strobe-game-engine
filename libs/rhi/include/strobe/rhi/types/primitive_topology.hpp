#pragma once

#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Primitive assembly topology.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{cpp} primitive_topology.hpp PrimitiveTopology
 *
 * Specifies how input vertices are assembled into primitives for rasterization.
 */
// [PrimitiveTopology]
enum class PrimitiveTopology : uint8_t {
  point_list,                    
  line_list,                     
  line_strip,                    
  triangle_list,                 
  triangle_strip,                
  triangle_fan,                  
  line_list_with_adjacency,      
  line_strip_with_adjacency,     
  triangle_list_with_adjacency,  
  triangle_strip_with_adjacency, 
  patch_list,                    
};
// [PrimitiveTopology]

} // namespace strobe::rhi
