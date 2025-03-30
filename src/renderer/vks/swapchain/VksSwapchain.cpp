#include "./VksSwapchain.hpp"

#include <stdexcept>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>

#include "input/Key.hpp"

namespace strobe::renderer::vks {

vk::SurfaceFormatKHR selectSurfaceFormat(
    std::span<const vk::SurfaceFormatKHR> availableFormats) {
  // Some platforms (especially older ones) return a single format with
  // UNDEFINED, which means: "you can choose anything"
  if (availableFormats.size() == 1 &&
      availableFormats[0].format == vk::Format::eUndefined) {
    return vk::SurfaceFormatKHR{vk::Format::eB8G8R8A8Unorm,
                                vk::ColorSpaceKHR::eSrgbNonlinear};
  }

  // Otherwise, search for the best available option
  for (const auto& format : availableFormats) {
    if (format.format == vk::Format::eB8G8R8A8Unorm &&
        format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
      return format;
    }
  }

  // Fallback: just take the first supported format
  return availableFormats[0];
}

static vk::PresentModeKHR selectPresentMode(
    std::span<const vk::PresentModeKHR> availablePresentModes) {
  // Prefer MAILBOX (triple-buffered, low-latency, no tearing)
  if (std::ranges::find(availablePresentModes, vk::PresentModeKHR::eMailbox) !=
      availablePresentModes.end()) {
    return vk::PresentModeKHR::eMailbox;
  }

  // Then try IMMEDIATE (lowest latency, allows tearing)
  if (std::ranges::find(availablePresentModes,
                        vk::PresentModeKHR::eImmediate) !=
      availablePresentModes.end()) {
    return vk::PresentModeKHR::eImmediate;
  }

  // Fallback: FIFO (vsync on, always supported)
  return vk::PresentModeKHR::eFifo;
}

static vk::Extent2D chooseSwapExtent(
    strobe::window::Window window,
    const VkSurfaceCapabilitiesKHR& capabilities) {
  if (capabilities.currentExtent.width !=
      std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  } else {
    const auto [width, height] = window.getFramebufferSize();

    vk::Extent2D actualExtent = {static_cast<uint32_t>(width),
                                 static_cast<uint32_t>(height)};

    actualExtent.width =
        std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                   capabilities.maxImageExtent.width);
    actualExtent.height =
        std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                   capabilities.maxImageExtent.height);

    return actualExtent;
  }
}

std::expected<VksSwapchain, vk::Result> VksSwapchain::tryCreate(
    strobe::window::Window& window, const VksContext& context) {
  const auto& physicalDeviceInfo = context.physicalDeviceInfo();

  auto controlBlock = new VksSwapchainControlBlock;

  vk::SurfaceFormatKHR surfaceFormat =
      selectSurfaceFormat(physicalDeviceInfo.surfaceFormats(context.surface()));
  vk::PresentModeKHR presentMode = selectPresentMode(
      physicalDeviceInfo.surfacePresentModes(context.surface()));
  const auto capabilities =
      physicalDeviceInfo.surfaceCapabilities(context.surface());
  vk::Extent2D extent = chooseSwapExtent(window, capabilities);

  uint32_t imageCount = capabilities.minImageCount + 1;
  if (capabilities.maxImageCount > 0 &&
      imageCount > capabilities.maxImageCount) {
    imageCount = capabilities.maxImageCount;
  }
  vk::SwapchainCreateInfoKHR createInfo;
  createInfo.minImageCount = imageCount;
  createInfo.imageFormat = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

  uint32_t temp[2] = {};
  if (context.getGraphicsQueue().queueFamilyIndex() ==
      context.getPresentQueue().queueFamilyIndex()) {
    createInfo.imageSharingMode = vk::SharingMode::eExclusive;
    createInfo.queueFamilyIndexCount = 0;      // Optional
    createInfo.pQueueFamilyIndices = nullptr;  // Optional
  } else {
    createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
    createInfo.queueFamilyIndexCount = 2;  // Optional
    temp[0] = context.getGraphicsQueue().queueFamilyIndex();
    temp[1] = context.getPresentQueue().queueFamilyIndex();
    createInfo.pQueueFamilyIndices = temp;  // Optional
  }
  createInfo.preTransform = capabilities.currentTransform;
  createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
  createInfo.presentMode = presentMode;
  createInfo.clipped = true;
  createInfo.oldSwapchain = nullptr;
  createInfo.surface = context.surface();

  {
    vk::Result result = context.device.createSwapchainKHR(
        &createInfo, nullptr, &controlBlock->swapchain);
    if (result != vk::Result::eSuccess) {
      return std::unexpected(result);
    }
  }

  controlBlock->images =
      context.device.getSwapchainImagesKHR(controlBlock->swapchain);
  controlBlock->format = surfaceFormat.format;
  controlBlock->extent = extent;

  controlBlock->imageViews.resize(controlBlock->images.size());
  for (std::size_t i = 0; i < controlBlock->imageViews.size(); ++i) {
    vk::ImageViewCreateInfo createInfo;
    createInfo.image = controlBlock->images[i];
    createInfo.viewType = vk::ImageViewType::e2D;
    createInfo.format = controlBlock->format;
    createInfo.components.r = vk::ComponentSwizzle::eIdentity;
    createInfo.components.g = vk::ComponentSwizzle::eIdentity;
    createInfo.components.b = vk::ComponentSwizzle::eIdentity;
    createInfo.components.a = vk::ComponentSwizzle::eIdentity;

    createInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;

    {
      vk::Result result = context.device.createImageView(
          &createInfo, nullptr, &controlBlock->imageViews.at(i));
      if (result != vk::Result::eSuccess) {
        throw std::unexpected(result);
      }
    }
  }

  return VksSwapchain(controlBlock);
}

void VksSwapchain::destroy(const VksContext& context) {
  for (const vk::ImageView imageView : m_controlBlock->imageViews) {
    context.device.destroyImageView(imageView);
  }
  context.device.destroySwapchainKHR(m_controlBlock->swapchain);

  delete m_controlBlock;
}
}  // namespace strobe::renderer::vks
