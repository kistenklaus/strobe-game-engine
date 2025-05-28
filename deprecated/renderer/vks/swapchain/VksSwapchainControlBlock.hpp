#pragma once

#include <vulkan/vulkan.hpp>

namespace strobe::renderer::vks {


struct VksSwapchainControlBlock {
  vk::SwapchainKHR swapchain;
  std::vector<vk::Image> images;
  std::vector<vk::ImageView> imageViews;

  vk::Format format;
  vk::Extent2D extent;


};
}
