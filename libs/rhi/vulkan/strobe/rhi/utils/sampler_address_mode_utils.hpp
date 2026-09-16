#pragma once

#include "strobe/rhi/types/sampler_address_mode.hpp"
#include <exception>
#include <fmt/printf.h>
#include <utility>
#include <vulkan/vulkan_core.h>

namespace strobe::rhi {

static inline VkSamplerAddressMode
to_vk_sampler_address_mode(SamplerAddressMode mode) noexcept {
  switch (mode) {
  case SamplerAddressMode::repeat:
    return VK_SAMPLER_ADDRESS_MODE_REPEAT;
  case SamplerAddressMode::mirrored_repeat:
    return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
  case SamplerAddressMode::clamp_to_edge:
    return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  case SamplerAddressMode::clamp_to_border:
    return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
  }
  std::unreachable();
}

static inline SamplerAddressMode
from_vk_sampler_address_mode(VkSamplerAddressMode mode) {
  switch (mode) {
  case VK_SAMPLER_ADDRESS_MODE_REPEAT:
    return SamplerAddressMode::repeat;
  case VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT:
    return SamplerAddressMode::mirrored_repeat;
  case VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE:
    return SamplerAddressMode::clamp_to_edge;
  case VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER:
    return SamplerAddressMode::clamp_to_border;
  case VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE:
  case VK_SAMPLER_ADDRESS_MODE_MAX_ENUM:
    fmt::println("Invalid VkSamplerAddressMode");
    std::terminate();
  }
  std::unreachable();
}

} // namespace strobe::rhi
