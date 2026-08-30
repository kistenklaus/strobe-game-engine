#pragma once

#include "strobe/rhi/swapchain/swapchain_generation.hpp"
namespace strobe::rhi {

struct SwapchainImageImpl {
  explicit SwapchainImageImpl(SwapchainGeneration generation,
                              uint32_t imageIndex)
      : generation(std::move(generation)), imageIndex(imageIndex) {}

  const SwapchainGeneration generation;
  const uint32_t imageIndex;
};

} // namespace strobe::rhi
