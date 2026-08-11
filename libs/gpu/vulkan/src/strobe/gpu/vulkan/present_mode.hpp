#pragma once

#include <compare>
#include <cstdint>
#include <type_traits>

namespace strobe::gpu::vulkan {

enum class PresentMode : uint8_t {
  immediate,
  mailbox,
  fifo,
  fifo_relaxed,
};

} // namespace strobe::gpu::vulkan
