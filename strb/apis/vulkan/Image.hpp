#pragma once
#include "strb/apis/vulkan/vulkan.lib.hpp"

namespace strb::vulkan {

// a very simple wrapper class for VkImage. (with next to no overhead)
// currently this can be treated like a Vulkan Handle.
struct Image {
 private:
  VkImage image;

 public:
  inline Image() : image(VK_NULL_HANDLE) {}
  inline Image(VkImage image) : image(image) {}
  inline operator VkImage() const { return image; }
};

}  // namespace strb::vulkan
