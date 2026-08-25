#pragma once

#include <cstdint>

namespace strobe::rhi {

enum class AttachmentLoadOp : uint8_t {
  load,
  clear,
  dont_care,
  none,
};

} // namespace strobe::rhi
