#pragma once

#include "strobe/gpu/device/swapchain_generation.hpp"
#include <cstdint>
#include <limits>
#include <utility>

namespace strobe::gpu {

class SwapchainImage {
  friend class Swapchain;
  friend class Queue;

public:
  SwapchainImage()
      : m_generation(), m_index(std::numeric_limits<uint32_t>::max()) {}

  explicit operator bool() const noexcept {
    return static_cast<bool>(m_generation);
  }

private:
  explicit SwapchainImage(SwapchainGeneration gen, uint32_t index)
      : m_generation(std::move(gen)), m_index(index) {}

  SwapchainGeneration m_generation;
  uint32_t m_index;
};

} // namespace strobe::gpu
