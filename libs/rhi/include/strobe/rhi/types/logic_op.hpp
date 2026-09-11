#pragma once

#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Logical color operation.
 *
 * Specifies the bitwise operation applied between source and destination color
 * values when logic operations are enabled.
 */
enum class LogicOp : uint8_t {
  clear,         ///< Set the result to zero.
  and_,          ///< Source AND destination.
  and_reverse,   ///< Source AND NOT destination.
  copy,          ///< Copy the source value.
  and_inverted,  ///< NOT source AND destination.
  no_op,         ///< Keep the destination value.
  xor_,          ///< Source XOR destination.
  or_,           ///< Source OR destination.
  nor,           ///< NOT source AND NOT destination.
  equivalent,    ///< NOT source XOR destination.
  invert,        ///< Invert the destination value.
  or_reverse,    ///< Source OR NOT destination.
  copy_inverted, ///< Invert the source value.
  or_inverted,   ///< NOT source OR destination.
  nand,          ///< NOT source OR NOT destination.
  set,           ///< Set all result bits to one.
};

} // namespace strobe::rhi
