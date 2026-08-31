#pragma once

#include "strobe/rhi/swapchain/swapchain_generation.hpp"
#include <cstdint>

namespace strobe::rhi {

struct SwapchainImageImpl {
  explicit SwapchainImageImpl(SwapchainGeneration generation,
                              uint32_t index) noexcept
      : generation(std::move(generation)), index(index) {}

  SwapchainGeneration generation;
  uint32_t index;
};

} // namespace strobe::rhi
