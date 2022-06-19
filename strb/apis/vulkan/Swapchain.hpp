#pragma once
#include "strb/apis/vulkan/Device.hpp"
#include "strb/apis/vulkan/Format.hpp"
#include "strb/apis/vulkan/ImageView.hpp"
#include "strb/apis/vulkan/PhysicalDevice.hpp"
#include "strb/apis/vulkan/Semaphore.hpp"
#include "strb/apis/vulkan/Surface.hpp"
#include "strb/apis/vulkan/vulkan.lib.hpp"

namespace strb::vulkan {

struct ImageView;
struct Device;
struct Semaphore;

struct Swapchain {
 private:
  const Device* device;
  const Surface* surface;
  const PhysicalDevice* physicalDevice;
  uint32_t width;
  uint32_t height;
  uint32_t imageCount;
  Format format;
  strb::vector<ImageView> imageViews;
  VkSwapchainKHR swapchain;

 public:
  inline Swapchain()
      : device(nullptr),
        surface(nullptr),
        physicalDevice(nullptr),
        imageViews({}),
        swapchain(VK_NULL_HANDLE) {}
  Swapchain(const Device& device, const PhysicalDevice& physicalDevice,
            const Surface& surface, const uint32_t width, const uint32_t height,
            const uint32_t minImageCount, const Format format);
  void recreate(const uint32_t width, const uint32_t height);
  void destroy();
  inline operator VkSwapchainKHR() const { return this->swapchain; }
  inline const ImageView& getImageView(const uint64_t index) const {
    return imageViews[index];
  }
  inline const strb::vector<ImageView> getImageViews() {
    return this->imageViews;
  }
  inline const uint64_t getImageCount() const { return imageViews.size(); }
  uint32_t nextFrameIndex(const Semaphore& imageAvaiable);
};

}  // namespace strb::vulkan
