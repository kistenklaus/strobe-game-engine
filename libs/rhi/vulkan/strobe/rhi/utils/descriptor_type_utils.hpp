#pragma once

#include "strobe/rhi/types/descriptor_type.hpp"
#include <fmt/printf.h>
#include <utility>
#include <vulkan/vulkan_core.h>

namespace strobe::rhi {

static inline VkDescriptorType
to_vk_descriptor_type(DescriptorType type) noexcept {
  switch (type) {
  case DescriptorType::storage_buffer:
    return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  }
  std::unreachable();
}

static inline DescriptorType
from_vk_descriptor_type(VkDescriptorType type) noexcept {
  switch (type) {
  case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
    return DescriptorType::storage_buffer;
  case VK_DESCRIPTOR_TYPE_SAMPLER:
  case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
  case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
  case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
  case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
  case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
  case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
  case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
  case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
  case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
  case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK:
  case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
  case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV:
  case VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM:
  case VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM:
  case VK_DESCRIPTOR_TYPE_TENSOR_ARM:
  case VK_DESCRIPTOR_TYPE_MUTABLE_EXT:
  case VK_DESCRIPTOR_TYPE_PARTITIONED_ACCELERATION_STRUCTURE_NV:
  case VK_DESCRIPTOR_TYPE_MAX_ENUM:
    fmt::println("INVALID descriptor type");
    std::terminate();
  }
  std::unreachable();
}

} // namespace strobe::rhi
