#pragma once

#include "strobe/rhi/swapchain/swapchain_generation.hpp"
#include <cstdint>
#include <limits>

namespace strobe::rhi {

struct SwapchainImageImpl {
  explicit SwapchainImageImpl(SwapchainGeneration generation,
                              uint32_t index) noexcept
      : generation(std::move(generation)), index(index) {}

  ~SwapchainImageImpl() noexcept {
    if (index != std::numeric_limits<uint32_t>::max()) {
      generation.release(index);
    }
  }

  void consume() noexcept { 
    assert(index != std::numeric_limits<uint32_t>::max());
    index = std::numeric_limits<uint32_t>::max(); 
  }

  SwapchainGeneration generation;
  uint32_t index;
};

} // namespace strobe::rhi
