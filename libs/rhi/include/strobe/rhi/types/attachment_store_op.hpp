#pragma once

#include <cstdint>

namespace strobe::rhi {

enum class AttachmentStoreOp : uint8_t {
  store,
  dont_care,
  none,
};

} // namespace strobe::rhi
