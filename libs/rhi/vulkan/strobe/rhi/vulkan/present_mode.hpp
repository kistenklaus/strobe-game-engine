#pragma once

#include <cstdint>

namespace strobe::rhi::vulkan {

enum class PresentMode : uint8_t {
  immediate,
  mailbox,
  fifo,
  fifo_relaxed,
};

} // namespace strobe::rhi::vulkan
