#pragma once

#include <cstdint>

namespace strobe::gpu::vulkan {

enum class PresentMode : uint8_t {
  immediate,
  mailbox,
  fifo,
  fifo_relaxed,
};

} // namespace strobe::gpu::vulkan
