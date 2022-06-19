#pragma once
#include "cassert"
#include "strb/apis/vulkan/Device.hpp"
#include "strb/apis/vulkan/Format.hpp"
#include "strb/apis/vulkan/Image.hpp"
#include "strb/apis/vulkan/vulkan.lib.hpp"

namespace strb::vulkan {

struct Device;

struct ImageView {
 private:
  const Device* device;
  VkImageView imageView;

 public:
  // default constructor
  inline ImageView() : device(nullptr), imageView(VK_NULL_HANDLE) {}
  ImageView(const Device& device, const Image image, const Format colorFormat);
  void destroy();
  inline operator VkImageView() const {
    assert(imageView != VK_NULL_HANDLE);
    return imageView;
  }
};

}  // namespace strb::vulkan
