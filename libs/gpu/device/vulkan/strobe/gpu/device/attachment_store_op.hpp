#pragma once

#include <cstdint>

namespace strobe::gpu {

enum class AttachmentStoreOp : uint8_t {
  store,
  dont_care,
  none,
};

} // namespace strobe::gpu
