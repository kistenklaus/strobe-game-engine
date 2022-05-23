#pragma once
#include "strb/apis/vulkan/vulkan.lib.hpp"

namespace strb::vulkan {

enum class Format {
  RGBA_UINT32 = VK_FORMAT_B8G8R8A8_UINT,
  RGBA_UNORM = VK_FORMAT_B8G8R8A8_UNORM
};

namespace format {
inline VkFormat toVkFormat(Format format) {
  return static_cast<VkFormat>(format);
}
} // namespace format

} // namespace strb::vulkan
