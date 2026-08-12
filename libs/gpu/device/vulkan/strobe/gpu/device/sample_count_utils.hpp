#pragma once

#include "strobe/gpu/device/sample_count.hpp"
#include <utility>
#include <vulkan/vulkan_core.h>
namespace strobe::gpu  {

static inline VkSampleCountFlagBits to_vk_sample_count(SampleCount count) {
  switch (count) {
  case SampleCount::x1:
    return VK_SAMPLE_COUNT_1_BIT;
  case SampleCount::x2:
    return VK_SAMPLE_COUNT_2_BIT;
  case SampleCount::x4:
    return VK_SAMPLE_COUNT_4_BIT;
  case SampleCount::x8:
    return VK_SAMPLE_COUNT_8_BIT;
  case SampleCount::x16:
    return VK_SAMPLE_COUNT_16_BIT;
  case SampleCount::x32:
    return VK_SAMPLE_COUNT_32_BIT;
  case SampleCount::x64:
    return VK_SAMPLE_COUNT_64_BIT;
  }
  std::unreachable();
}

static inline SampleCount from_vk_sample_count(VkSampleCountFlagBits count) {
  switch (count) {
  case VK_SAMPLE_COUNT_1_BIT:
    return SampleCount::x1;
  case VK_SAMPLE_COUNT_2_BIT:
    return SampleCount::x2;
  case VK_SAMPLE_COUNT_4_BIT:
    return SampleCount::x4;
  case VK_SAMPLE_COUNT_8_BIT:
    return SampleCount::x8;
  case VK_SAMPLE_COUNT_16_BIT:
    return SampleCount::x16;
  case VK_SAMPLE_COUNT_32_BIT:
    return SampleCount::x32;
  case VK_SAMPLE_COUNT_64_BIT:
    return SampleCount::x64;
  default:
    std::unreachable();
  }
}

}
