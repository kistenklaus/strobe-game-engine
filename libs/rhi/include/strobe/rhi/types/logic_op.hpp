#pragma once

#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Logical color operation.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{cpp} logic_op.hpp LogicOp
 *
 * Specifies the bitwise operation applied between source and destination color
 * values when logic operations are enabled.
 */
// [LogicOp]
enum class LogicOp : uint8_t {
  clear,
  and_,
  and_reverse,
  copy,
  and_inverted,
  no_op,
  xor_,
  or_,
  nor,
  equivalent,
  invert,
  or_reverse,
  copy_inverted,
  or_inverted,
  nand,
  set,
};
// [LogicOp]

} // namespace strobe::rhi
