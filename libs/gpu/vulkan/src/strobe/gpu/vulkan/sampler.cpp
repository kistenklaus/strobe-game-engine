#include "strobe/gpu/vulkan/sampler.hpp"
#include <stdexcept>

namespace strobe::gpu::vulkan {

Sampler create_sampler(Context *context, const SamplerInfo &info) {
  assert(context != nullptr);

  if (info.anisotropyEnable) {
    assert(context->deviceInfo().features.samplerAnisotropy);
    assert(info.maxAnisotropy >= 1.0f);
    assert(info.maxAnisotropy <=
           context->deviceInfo().properties.limits.maxSamplerAnisotropy);
  }

  assert(info.minLod <= info.maxLod);

  VkSamplerCreateInfo createInfo{
      .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .magFilter = info.magFilter,
      .minFilter = info.minFilter,
      .mipmapMode = info.mipmapMode,
      .addressModeU = info.addressModeU,
      .addressModeV = info.addressModeV,
      .addressModeW = info.addressModeW,
      .mipLodBias = info.mipLodBias,
      .anisotropyEnable = info.anisotropyEnable ? VK_TRUE : VK_FALSE,
      .maxAnisotropy = info.maxAnisotropy,
      .compareEnable = info.compareEnable ? VK_TRUE : VK_FALSE,
      .compareOp = info.compareOp,
      .minLod = info.minLod,
      .maxLod = info.maxLod,
      .borderColor = info.borderColor,
      .unnormalizedCoordinates =
          info.unnormalizedCoordinates ? VK_TRUE : VK_FALSE,
  };
  Sampler sampler{};
  VkResult result = vkCreateSampler(context->device(), &createInfo,
                                    context->driver_alloc(), &sampler.handle);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to create sampler");
  }
  return sampler;
}
void destroy_sampler(Context *context, Sampler sampler) noexcept {
  assert(context != nullptr);
  assert(sampler);
  vkDestroySampler(context->device(), sampler.handle, context->driver_alloc());
}

} // namespace strobe::gpu::vulkan
