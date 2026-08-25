#pragma once

#include <cstdint>

namespace strobe::rhi {

enum class CompareOp : uint8_t {
  never,
  less,
  equal,
  less_or_equal,
  greater,
  not_equal,
  greater_or_equal,
  always,
};

} // namespace strobe::rhi
