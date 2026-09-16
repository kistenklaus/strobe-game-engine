#pragma once

#include "strobe/rhi/types/sampler_mipmap_mode.hpp"
#include <exception>
#include <fmt/printf.h>
#include <utility>
#include <vulkan/vulkan_core.h>
namespace strobe::rhi {

static inline VkSamplerMipmapMode
to_vk_sampler_mipmap_mode(SamplerMipmapMode mode) noexcept {
  switch (mode) {
  case SamplerMipmapMode::nearest:
    return VK_SAMPLER_MIPMAP_MODE_NEAREST;
  case SamplerMipmapMode::linear:
    return VK_SAMPLER_MIPMAP_MODE_LINEAR;
  }
  std::unreachable();
}

static inline SamplerMipmapMode
from_vk_sampler_mipmap_mode(VkSamplerMipmapMode mode) noexcept {
  switch (mode) {
  case VK_SAMPLER_MIPMAP_MODE_NEAREST:
    return SamplerMipmapMode::nearest;
  case VK_SAMPLER_MIPMAP_MODE_LINEAR:
    return SamplerMipmapMode::linear;
  case VK_SAMPLER_MIPMAP_MODE_MAX_ENUM:
    fmt::println("Invalid VkSamplerMipmapMode");
    std::terminate();
  }
  std::unreachable();
}

} // namespace strobe::rhi
