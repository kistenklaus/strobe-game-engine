#pragma once

#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Color blend operation.
 *
 * Specifies how source and destination blend terms are combined.
 */
enum class BlendOp : uint8_t {
  add,              ///< Adds source and destination terms.
  subtract,         ///< Subtracts destination from source.
  reverse_subtract, ///< Subtracts source from destination.
  min,              ///< Selects the component-wise minimum.
  max,              ///< Selects the component-wise maximum.
};

} // namespace strobe::rhi
