#pragma once

#include "strobe/gpu/vulkan/context/context.hpp"
#include <vulkan/vulkan_core.h>

namespace strobe::gpu::vulkan {

struct Sampler {
  VkSampler handle = VK_NULL_HANDLE;
  explicit operator bool() const noexcept { return handle != VK_NULL_HANDLE; }
};

struct SamplerInfo {
  VkFilter magFilter = VK_FILTER_NEAREST;
  VkFilter minFilter = VK_FILTER_NEAREST;
  VkSamplerMipmapMode mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
  VkSamplerAddressMode addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
  VkSamplerAddressMode addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
  VkSamplerAddressMode addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
  float mipLodBias = 0.0f;
  bool anisotropyEnable = false;
  float maxAnisotropy = 1.0f;
  bool compareEnable = false;
  VkCompareOp compareOp = VK_COMPARE_OP_ALWAYS;
  float minLod = 0.0f;
  float maxLod = VK_LOD_CLAMP_NONE;
  VkBorderColor borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
  bool unnormalizedCoordinates = VK_FALSE;
};

Sampler create_sampler(Context *context, const SamplerInfo &info = {});

void destroy_sampler(Context *context, Sampler sampler) noexcept;

} // namespace strobe::gpu::vulkan
