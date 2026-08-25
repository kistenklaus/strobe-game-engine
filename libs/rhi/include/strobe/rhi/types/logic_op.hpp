#pragma once

#include <cstdint>

namespace strobe::rhi {

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

} // namespace strobe::rhi
