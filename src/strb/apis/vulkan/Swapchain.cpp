#include "strb/apis/vulkan/Swapchain.hpp"

#include <cassert>
#include <limits>

#include "strb/apis/vulkan/assert.hpp"

namespace strb::vulkan {

Swapchain::Swapchain(const Device& device, const PhysicalDevice& physicalDevice,
                     const Surface& surface, const uint32_t width,
                     const uint32_t height, const uint32_t minImageCount,
                     const Format format)
    : device(&device),
      surface(&surface),
      physicalDevice(&physicalDevice),
      width(width),
      height(height),
      format(format) {
  assert(device.getGraphicsQueueFamilyIndex().has_value());
  VkSurfaceCapabilitiesKHR surfaceCap;
  VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
      physicalDevice, surface, &surfaceCap);
  ASSERT_VKRESULT(result);
  // TODO check if format is supported!!
  VkSwapchainCreateInfoKHR swapchainCreateInfo;
  swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swapchainCreateInfo.pNext = nullptr;
  swapchainCreateInfo.flags = 0;
  swapchainCreateInfo.surface = surface;
  this->imageCount = std::max(surfaceCap.minImageCount, minImageCount);
  swapchainCreateInfo.minImageCount = this->imageCount;
  swapchainCreateInfo.imageFormat = format::toVkFormat(format);
  swapchainCreateInfo.imageColorSpace =
      VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;  // TODO check if valid.
  swapchainCreateInfo.imageExtent = VkExtent2D{width, height};
  swapchainCreateInfo.imageArrayLayers = 1;
  swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  swapchainCreateInfo.queueFamilyIndexCount = 0;
  swapchainCreateInfo.pQueueFamilyIndices = nullptr;
  swapchainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
  swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swapchainCreateInfo.presentMode =
      VK_PRESENT_MODE_FIFO_KHR;  // TODO try if MAILBOX WORKS.
  swapchainCreateInfo.clipped = VK_TRUE;
  swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

  VkBool32 surfaceSupport = false;
  result = vkGetPhysicalDeviceSurfaceSupportKHR(
      physicalDevice, device.getGraphicsQueueFamilyIndex().value(), surface,
      &surfaceSupport);
  ASSERT_VKRESULT(result);
  assert(surfaceSupport);
  result = vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr,
                                &this->swapchain);
  ASSERT_VKRESULT(result);

  uint32_t n_swapchainImages;
  result = vkGetSwapchainImagesKHR(device, this->swapchain, &n_swapchainImages,
                                   nullptr);
  ASSERT_VKRESULT(result);
  strb::vector<VkImage> swapchainImages(n_swapchainImages);
  result = vkGetSwapchainImagesKHR(device, this->swapchain, &n_swapchainImages,
                                   swapchainImages.data());

  ASSERT_VKRESULT(result);

  imageViews.resize(n_swapchainImages);
  for (uint64_t i = 0; i < swapchainImages.size(); i++) {
    imageViews[i] = ImageView(device, Image(swapchainImages[i]), format);
  }
  assert(imageViews.size() == swapchainImages.size());
}

void Swapchain::recreate(const uint32_t width, const uint32_t height) {
  this->width = width;
  this->height = height;

  VkSwapchainCreateInfoKHR swapchainCreateInfo;
  swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swapchainCreateInfo.pNext = nullptr;
  swapchainCreateInfo.flags = 0;
  swapchainCreateInfo.surface = *this->surface;
  swapchainCreateInfo.minImageCount = this->imageCount;
  swapchainCreateInfo.imageFormat = format::toVkFormat(format);
  swapchainCreateInfo.imageColorSpace =
      VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;  // TODO check if valid.
  swapchainCreateInfo.imageExtent = VkExtent2D{this->width, this->height};
  swapchainCreateInfo.imageArrayLayers = 1;
  swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  swapchainCreateInfo.queueFamilyIndexCount = 0;
  swapchainCreateInfo.pQueueFamilyIndices = nullptr;
  swapchainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
  swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swapchainCreateInfo.presentMode =
      VK_PRESENT_MODE_FIFO_KHR;  // TODO try if MAILBOX WORKS.
  swapchainCreateInfo.clipped = VK_TRUE;
  swapchainCreateInfo.oldSwapchain = this->swapchain;

  VkBool32 surfaceSupport = false;
  VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(
      *this->physicalDevice,
      this->device->getGraphicsQueueFamilyIndex().value(), *this->surface,
      &surfaceSupport);
  ASSERT_VKRESULT(result);
  assert(surfaceSupport);
  result = vkCreateSwapchainKHR(*this->device, &swapchainCreateInfo, nullptr,
                                &this->swapchain);
  ASSERT_VKRESULT(result);

  uint32_t n_swapchainImages;
  result = vkGetSwapchainImagesKHR(*this->device, this->swapchain,
                                   &n_swapchainImages, nullptr);
  ASSERT_VKRESULT(result);
  strb::vector<VkImage> swapchainImages(n_swapchainImages);
  result = vkGetSwapchainImagesKHR(*this->device, this->swapchain,
                                   &n_swapchainImages, swapchainImages.data());

  ASSERT_VKRESULT(result);

  for (ImageView& view : imageViews) {
    view.destroy();
  }
  imageViews.clear();

  imageViews.resize(n_swapchainImages);
  for (uint64_t i = 0; i < swapchainImages.size(); i++) {
    imageViews[i] = ImageView(*this->device, Image(swapchainImages[i]), format);
  }
  assert(imageViews.size() == swapchainImages.size());
  // Destroy old swapchain.
  vkDestroySwapchainKHR(*this->device, swapchainCreateInfo.oldSwapchain,
                        nullptr);
}

void Swapchain::destroy() {
  assert(this->device != nullptr);
  assert(this->swapchain != VK_NULL_HANDLE);
  vkDestroySwapchainKHR(*device, this->swapchain, nullptr);
  for (ImageView imageView : imageViews) {
    imageView.destroy();
  }
  dexec(this->device = nullptr);
  dexec(imageViews.clear());
  dexec(this->swapchain = VK_NULL_HANDLE);
  // this->imageViews.clear();
}

uint32_t Swapchain::nextFrameIndex(const Semaphore& imageAvaiable) {
  uint32_t nextIndex;
  VkResult result = vkAcquireNextImageKHR(
      *this->device, this->swapchain, std::numeric_limits<uint64_t>::max(),
      imageAvaiable, VK_NULL_HANDLE, &nextIndex);
  ASSERT_VKRESULT(result);
  return nextIndex;
}

}  // namespace strb::vulkan
