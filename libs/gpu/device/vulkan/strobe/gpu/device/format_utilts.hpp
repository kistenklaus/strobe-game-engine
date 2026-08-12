#pragma once

#include "strobe/gpu/device/format.hpp"
#include <cassert>
#include <utility>
#include <vulkan/vulkan_core.h>

namespace strobe::gpu {

static inline VkFormat to_vk_format(Format format) {
  switch (format) {
  case Format::undefined:
    return VK_FORMAT_UNDEFINED;

  case Format::r8_unorm:
    return VK_FORMAT_R8_UNORM;
  case Format::r8_snorm:
    return VK_FORMAT_R8_SNORM;
  case Format::r8_uint:
    return VK_FORMAT_R8_UINT;
  case Format::r8_sint:
    return VK_FORMAT_R8_SINT;

  case Format::rg8_unorm:
    return VK_FORMAT_R8G8_UNORM;
  case Format::rg8_snorm:
    return VK_FORMAT_R8G8_SNORM;
  case Format::rg8_uint:
    return VK_FORMAT_R8G8_UINT;
  case Format::rg8_sint:
    return VK_FORMAT_R8G8_SINT;

  case Format::rgba8_unorm:
    return VK_FORMAT_R8G8B8A8_UNORM;
  case Format::rgba8_srgb:
    return VK_FORMAT_R8G8B8A8_SRGB;
  case Format::rgba8_uint:
    return VK_FORMAT_R8G8B8A8_UINT;
  case Format::rgba8_sint:
    return VK_FORMAT_R8G8B8A8_SINT;

  case Format::bgra8_unorm:
    return VK_FORMAT_B8G8R8A8_UNORM;
  case Format::bgra8_srgb:
    return VK_FORMAT_B8G8R8A8_SRGB;

  case Format::r16_float:
    return VK_FORMAT_R16_SFLOAT;
  case Format::rg16_float:
    return VK_FORMAT_R16G16_SFLOAT;
  case Format::rgba16_float:
    return VK_FORMAT_R16G16B16A16_SFLOAT;

  case Format::r32_float:
    return VK_FORMAT_R32_SFLOAT;
  case Format::rg32_float:
    return VK_FORMAT_R32G32_SFLOAT;
  case Format::rgb32_float:
    return VK_FORMAT_R32G32B32_SFLOAT;
  case Format::rgba32_float:
    return VK_FORMAT_R32G32B32A32_SFLOAT;

  case Format::r32_uint:
    return VK_FORMAT_R32_UINT;
  case Format::rg32_uint:
    return VK_FORMAT_R32G32_UINT;
  case Format::rgba32_uint:
    return VK_FORMAT_R32G32B32A32_UINT;

  case Format::r32_sint:
    return VK_FORMAT_R32_SINT;
  case Format::rg32_sint:
    return VK_FORMAT_R32G32_SINT;
  case Format::rgba32_sint:
    return VK_FORMAT_R32G32B32A32_SINT;

  case Format::d16_unorm:
    return VK_FORMAT_D16_UNORM;
  case Format::d32_float:
    return VK_FORMAT_D32_SFLOAT;
  case Format::d24_unorm_s8_uint:
    return VK_FORMAT_D24_UNORM_S8_UINT;
  case Format::d32_float_s8_uint:
    return VK_FORMAT_D32_SFLOAT_S8_UINT;
  case Format::unsupported:
    return VK_FORMAT_UNDEFINED;
  }
  std::unreachable();
}

static inline Format from_vk_format(VkFormat format) {
  switch (format) {
  case VK_FORMAT_UNDEFINED:
    return Format::undefined;

  case VK_FORMAT_R8_UNORM:
    return Format::r8_unorm;
  case VK_FORMAT_R8_SNORM:
    return Format::r8_snorm;
  case VK_FORMAT_R8_UINT:
    return Format::r8_uint;
  case VK_FORMAT_R8_SINT:
    return Format::r8_sint;

  case VK_FORMAT_R8G8_UNORM:
    return Format::rg8_unorm;
  case VK_FORMAT_R8G8_SNORM:
    return Format::rg8_snorm;
  case VK_FORMAT_R8G8_UINT:
    return Format::rg8_uint;
  case VK_FORMAT_R8G8_SINT:
    return Format::rg8_sint;

  case VK_FORMAT_R8G8B8A8_UNORM:
    return Format::rgba8_unorm;
  case VK_FORMAT_R8G8B8A8_SRGB:
    return Format::rgba8_srgb;
  case VK_FORMAT_R8G8B8A8_UINT:
    return Format::rgba8_uint;
  case VK_FORMAT_R8G8B8A8_SINT:
    return Format::rgba8_sint;

  case VK_FORMAT_B8G8R8A8_UNORM:
    return Format::bgra8_unorm;
  case VK_FORMAT_B8G8R8A8_SRGB:
    return Format::bgra8_srgb;

  case VK_FORMAT_R16_SFLOAT:
    return Format::r16_float;
  case VK_FORMAT_R16G16_SFLOAT:
    return Format::rg16_float;
  case VK_FORMAT_R16G16B16A16_SFLOAT:
    return Format::rgba16_float;

  case VK_FORMAT_R32_SFLOAT:
    return Format::r32_float;
  case VK_FORMAT_R32G32_SFLOAT:
    return Format::rg32_float;
  case VK_FORMAT_R32G32B32_SFLOAT:
    return Format::rgb32_float;
  case VK_FORMAT_R32G32B32A32_SFLOAT:
    return Format::rgba32_float;

  case VK_FORMAT_R32_UINT:
    return Format::r32_uint;
  case VK_FORMAT_R32G32_UINT:
    return Format::rg32_uint;
  case VK_FORMAT_R32G32B32A32_UINT:
    return Format::rgba32_uint;

  case VK_FORMAT_R32_SINT:
    return Format::r32_sint;
  case VK_FORMAT_R32G32_SINT:
    return Format::rg32_sint;
  case VK_FORMAT_R32G32B32A32_SINT:
    return Format::rgba32_sint;

  case VK_FORMAT_D16_UNORM:
    return Format::d16_unorm;
  case VK_FORMAT_D32_SFLOAT:
    return Format::d32_float;
  case VK_FORMAT_D24_UNORM_S8_UINT:
    return Format::d24_unorm_s8_uint;
  case VK_FORMAT_D32_SFLOAT_S8_UINT:
    return Format::d32_float_s8_uint;
  default:
    return Format::unsupported;
  }
  std::unreachable();
}

} // namespace strobe::gpu
