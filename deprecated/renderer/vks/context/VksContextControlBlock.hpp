#pragma once

#include <memory>
#include <vulkan/vulkan.hpp>

#include "renderer/vks/context/VkPhysicalDeviceInfo.hpp"
#include "renderer/vks/context/VksContextExtensionLifetime.hpp"
#include "renderer/vks/context/VksQueueInfo.hpp"

namespace strobe::renderer::vks {

struct VksContextControlBlock {
  vk::detail::DispatchLoaderDynamic dldi;

  VksPhysicalDeviceInfo physicalDeviceInfo;

  std::vector<std::unique_ptr<VksContextExtensionLifetime>> extensionLifetimes;

  vk::SurfaceKHR surface;

  std::vector<VksQueueInfo> graphicsQueues;
  std::vector<VksQueueInfo> transferQueues;
  std::vector<VksQueueInfo> presentQueues;
  std::vector<VksQueueInfo> computeQueues;
};

}  // namespace strobe::renderer::vks
